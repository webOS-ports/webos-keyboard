/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "precorrector.h"

#include <algorithm>
#include <cmath>

namespace Pre {
namespace {

const int MAX_LENGTH = 24;
//! An ambiguous word becomes its contraction after a context that makes it this many times likelier...
const int CERTAIN = 10;
//! ...seen this often at least.
const int MIN_EVIDENCE = 10;
//! A one-letter prefix begins thousands of words: look through this many at most.
const int COMPLETION_SCAN = 15000;
//! How many log2-frequency points one unit of edit cost is worth.
const float COST_WEIGHT = 4.0f;
const QString ALPHABET = QStringLiteral("abcdefghijklmnopqrstuvwxyz");
const QString VOWELS = QStringLiteral("aeiouy");
//! The letters right above the space bar, which a thumb reaching for it hits instead.
const QString ABOVE_SPACE = QStringLiteral("cvbn");
//! Running two words together costs about what a slip onto the next key does...
const float MISSED_SPACE = 0.6f;
//! ...and hitting a letter for the space a little more, since it adds a wrong letter...
const float SPACE_AS_LETTER = 0.8f;
//! ...and more again when the second word isn't one that commonly follows the first.
const float UNPAIRED = 1.5f;
//! Taken off a split of two little words that don't go together.
const float LITTLE_PAIR = 3.0f;
//! Putting in, taking out or changing an accent: the cheapest slip of all.
const float ACCENT = 0.15f;
//! Leaving out an elision's apostrophe ("cest"): next to nothing.
const float MISSED_APOSTROPHE = 0.3f;

bool allLettersOrApostrophe(const QString &s)
{
    for (QChar c : s)
        if (!c.isLetter() && c != QLatin1Char('\''))
            return false;
    return true;
}

bool allLetters(const QString &s)
{
    for (QChar c : s)
        if (!c.isLetter())
            return false;
    return true;
}

//! Every character an upper-case letter or an apostrophe ("NASA", "OK", "DON'T").
bool allCapitals(const QString &s)
{
    for (QChar c : s)
        if (!c.isUpper() && c != QLatin1Char('\''))
            return false;
    return true;
}

//! A list of words in the order first added, each once (Kotlin's LinkedHashSet).
class Ordered
{
public:
    void add(const QString &w) { if (!m_seen.contains(w)) { m_seen.insert(w); m_list << w; } }
    bool contains(const QString &w) const { return m_seen.contains(w); }
    int size() const { return m_list.size(); }
    QString first() const { return m_list.isEmpty() ? QString() : m_list.first(); }
    QStringList take(int n) const { return m_list.mid(0, n); }
    const QStringList &list() const { return m_list; }
private:
    QSet<QString> m_seen;
    QStringList m_list;
};

} // namespace

const QHash<QString, QString> &Corrector::contractions()
{
    static const QHash<QString, QString> c = {
        { "dont", "don't" }, { "cant", "can't" }, { "wont", "won't" }, { "isnt", "isn't" },
        { "arent", "aren't" }, { "wasnt", "wasn't" }, { "werent", "weren't" }, { "hasnt", "hasn't" },
        { "havent", "haven't" }, { "hadnt", "hadn't" }, { "doesnt", "doesn't" }, { "didnt", "didn't" },
        { "couldnt", "couldn't" }, { "shouldnt", "shouldn't" }, { "wouldnt", "wouldn't" },
        { "mustnt", "mustn't" }, { "neednt", "needn't" }, { "aint", "ain't" },
        { "im", "I'm" }, { "ive", "I've" }, { "id", "I'd" },
        { "youre", "you're" }, { "youve", "you've" }, { "youll", "you'll" }, { "youd", "you'd" },
        { "theyre", "they're" }, { "theyve", "they've" }, { "theyll", "they'll" }, { "theyd", "they'd" },
        { "weve", "we've" }, { "hes", "he's" }, { "shes", "she's" },
        { "thats", "that's" }, { "whats", "what's" }, { "wheres", "where's" }, { "whos", "who's" },
        { "hows", "how's" }, { "theres", "there's" }, { "heres", "here's" }, { "itll", "it'll" },
        { "thatll", "that'll" }, { "wouldve", "would've" }, { "couldve", "could've" },
        { "shouldve", "should've" }, { "mightve", "might've" }, { "mustve", "must've" },
        { "hed", "he'd" }, { "itd", "it'd" }, { "thatd", "that'd" }, { "whod", "who'd" },
        { "wholl", "who'll" }, { "whatll", "what'll" },
        { "yall", "y'all" }, { "oclock", "o'clock" }, { "whatre", "what're" }, { "whove", "who've" },
    };
    return c;
}

const QHash<QString, QString> &Corrector::ambiguous()
{
    static const QHash<QString, QString> a = {
        { "ill", "I'll" }, { "its", "it's" }, { "well", "we'll" }, { "were", "we're" }, { "lets", "let's" },
        { "wed", "we'd" }, { "hell", "he'll" }, { "shell", "she'll" }, { "shed", "she'd" },
    };
    return a;
}

Corrector::Corrector(const QStringList &lines, const Language &lang)
    : follows([](const QString &, const QString &) { return false; }),
      m_lang(lang)
{
    m_words.reserve(131072);
    for (const QString &line : lines) {
        if (line.isEmpty() || line[0] == QLatin1Char('#'))
            continue;
        // A regional dictionary's word that the one before it had and it hasn't: "-color".
        if (line[0] == QLatin1Char('-')) {
            const QString w = line.mid(1);
            const QString key = w.toLower();
            auto it = m_words.find(key);
            if (it != m_words.end() && it->word == w)
                m_words.erase(it);
            continue;
        }
        const int tab = line.indexOf(QLatin1Char('\t'));
        if (tab <= 0)
            continue;
        const QString w = line.left(tab);
        bool ok = false;
        const int score = line.mid(tab + 1).trimmed().toInt(&ok);
        if (!ok)
            continue;
        const QString key = w.toLower();
        auto had = m_words.find(key);
        // Prefer the lower-case spelling when a word comes both ways ("bill" and "Bill").
        if (had == m_words.end() || (w == key && had->word != key) || (had->word != key && score > had->score))
            m_words.insert(key, Entry{ w, score });
    }

    if (!m_lang.accents.isEmpty()) {
        for (auto it = m_words.cbegin(); it != m_words.cend(); ++it) {
            const QString f = Language::fold(it.key());
            if (f == it.key())
                continue;
            auto had = m_plain.find(f);
            if (had == m_plain.end() || it->score > had->score)
                m_plain.insert(f, it.value());
        }
    }

    m_sorted = m_words.keys();
    std::sort(m_sorted.begin(), m_sorted.end());

    const QStringList rows = layoutRows(m_lang.layout);
    for (int i = 0; i < rows[0].length(); ++i) m_position.insert(rows[0][i], { i, 0 });
    for (int i = 0; i < rows[1].length(); ++i) m_position.insert(rows[1][i], { i, 1 });
    for (int i = 0; i < rows[2].length(); ++i) m_position.insert(rows[2][i], { i + 1, 2 });
}

void Corrector::loadContractions(const QStringList &lines)
{
    for (const QString &line : lines) {
        if (line.isEmpty() || line[0] == QLatin1Char('#'))
            continue;
        const QStringList f = line.split(QLatin1Char('\t'));
        if (f.size() < 4)
            continue;
        bool ok1 = false, ok2 = false;
        const int a = f[2].toInt(&ok1);
        const int b = f[3].toInt(&ok2);
        if (!ok1 || !ok2)
            continue;
        m_odds.insert(f[0] + QLatin1Char('\t') + f[1], { a, b });
    }
}

bool Corrector::contractionMeant(const QString &word, const QString &context) const
{
    if (context.isEmpty())
        return false;
    auto it = m_odds.find(word + QLatin1Char('\t') + context);
    if (it == m_odds.end())
        return false;
    return it->second >= MIN_EVIDENCE && it->second + 1 >= CERTAIN * (it->first + 1);
}

QVector<QPair<QString, int>> Corrector::entries() const
{
    QVector<QPair<QString, int>> out;
    out.reserve(m_words.size());
    for (auto it = m_words.cbegin(); it != m_words.cend(); ++it)
        out.append({ it->word, it->score });
    return out;
}

void Corrector::learn(const QString &word)
{
    QMutexLocker l(&m_learnedLock);
    m_learned.insert(word.toLower());
}

void Corrector::learnAll(const QStringList &words)
{
    for (const QString &w : words)
        learn(w);
}

void Corrector::unlearn(const QString &word)
{
    QMutexLocker l(&m_learnedLock);
    m_learned.remove(word.toLower());
}

bool Corrector::learned(const QString &lower) const
{
    QMutexLocker l(&m_learnedLock);
    return m_learned.contains(lower);
}

bool Corrector::isKnown(const QString &word) const
{
    const auto e = m_lang.elided(word);
    if (!e.first.isEmpty())
        return isKnown(e.second) || learned(word.toLower());
    const QString lower = word.toLower();
    return m_words.contains(lower) || learned(lower);
}

QString Corrector::spelling(const QString &word) const
{
    const auto e = m_lang.elided(word);
    if (!e.first.isEmpty())
        return spelling(e.second);
    const QString lower = word.toLower();
    auto it = m_words.find(lower);
    if (it != m_words.end())
        return it->word;
    return learned(lower) ? lower : QString();
}

QString Corrector::correct(const QString &typed, const QString &context) const
{
    if (typed.isEmpty() || typed.length() > MAX_LENGTH)
        return QString();
    if (!allLettersOrApostrophe(typed) || !typed[0].isLetter())
        return QString();
    const QString lower = typed.toLower();
    if (learned(lower))
        return QString();
    // A word typed all in capitals, beyond one letter, is meant that way (NASA, OK).
    if (typed.length() > 1 && allCapitals(typed))
        return QString();
    // "l'hommme": the word after the apostrophe is the one to correct.
    const auto e = m_lang.elided(typed);
    if (!e.first.isEmpty()) {
        const QString fixed = correct(e.second, QString());
        return fixed.isEmpty() ? QString() : e.first + fixed;
    }

    if (m_lang.english()) {
        auto c = contractions().find(lower);
        if (c != contractions().end()) {
            const QString r = casedLike(typed, c.value());
            return r != typed ? r : QString();
        }
        auto a = ambiguous().find(lower);
        if (a != ambiguous().end() && contractionMeant(lower, context))
            return casedLike(typed, a.value());
    }

    auto known = m_words.find(lower);
    if (known != m_words.end()) {
        // Known. Only its capital can be wrong: "i", or "london" for "London".
        return (typed == lower && known->word != lower) ? known->word : QString();
    }
    if (lower.length() < 2 || lower.contains(QLatin1Char('\'')))
        return QString();

    QList<Scored> found;
    const QList<Scored> one = nearestScored(lower, 1);
    if (!one.isEmpty()) found << one.first();
    const Scored two = split(lower);
    if (!two.first.isEmpty()) found << two;
    const Scored el = elision(lower);
    if (!el.first.isEmpty()) found << el;
    const Scored acc = accented(lower);
    if (!acc.first.isEmpty()) found << acc;
    if (found.isEmpty())
        return QString();
    Scored best = found.first();
    for (const Scored &s : found)
        if (s.second > best.second)
            best = s;
    const QString r = casedLike(typed, best.first);
    return r != typed ? r : QString();
}

Corrector::Scored Corrector::accented(const QString &w) const
{
    auto it = m_plain.find(Language::fold(w));
    if (it == m_plain.end())
        return {};
    if (it->word.compare(w, Qt::CaseInsensitive) == 0)
        return {};
    return { it->word, it->score - COST_WEIGHT * ACCENT };
}

Corrector::Scored Corrector::elision(const QString &w) const
{
    Scored best;
    bool have = false;
    for (const QString &e : m_lang.elisions) {
        if (w.length() < e.length() + 2 || !w.startsWith(e))
            continue;
        auto rest = m_words.find(w.mid(e.length()));
        if (rest == m_words.end())
            continue;
        // Only before a vowel or a silent h does a word elide.
        if (!QStringLiteral("aeiouyh").contains(Language::base(rest->word[0].toLower())))
            continue;
        const float value = rest->score - COST_WEIGHT * MISSED_APOSTROPHE;
        if (!have || value > best.second) {
            best = { e + QLatin1Char('\'') + rest->word, value };
            have = true;
        }
    }
    return best;
}

QStringList Corrector::suggestions(const QString &typed, const QString &context, int max) const
{
    if (typed.isEmpty() || typed.length() > MAX_LENGTH || !typed[0].isLetter())
        return {};
    const auto e = m_lang.elided(typed);
    if (!e.first.isEmpty()) {
        QStringList out;
        for (const QString &w : suggestions(e.second, QString(), max))
            out << e.first + w;
        return out;
    }
    const QString lower = typed.toLower();
    Ordered out;
    const QString fix = correct(typed, context);
    if (!fix.isEmpty()) {
        out.add(fix);
    } else if (learned(lower)) {
        out.add(typed);
    } else {
        auto it = m_words.find(lower);
        if (it != m_words.end())
            out.add(casedLike(typed, it->word));
    }
    // A word that is also a contraction without its apostrophe ("ill", "its"):
    // the contraction next, since nothing else here would find it; or, when the
    // context made it the contraction, the word as typed next.
    if (m_lang.english()) {
        if (!fix.isEmpty() && ambiguous().contains(lower)) {
            auto it = m_words.find(lower);
            if (it != m_words.end())
                out.add(casedLike(typed, it->word));
        }
        if (out.size() > 0) {
            auto a = ambiguous().find(out.first().toLower());
            if (a != ambiguous().end())
                out.add(casedLike(typed, a.value()));
        }
    }
    for (const QString &w : completions(lower, 2))
        out.add(casedLike(typed, w));
    if (lower.length() >= 2 && !lower.contains(QLatin1Char('\'')) && allLetters(typed))
        for (const QString &w : nearest(lower, max))
            out.add(casedLike(typed, w));
    for (const QString &w : completions(lower, max + 2))
        out.add(casedLike(typed, w));
    return out.take(max);
}

QStringList Corrector::alternatives(const QString &word, int max) const
{
    if (word.isEmpty() || word.length() > MAX_LENGTH || !word[0].isLetter())
        return {};
    const auto e = m_lang.elided(word);
    if (!e.first.isEmpty()) {
        QStringList out;
        for (const QString &w : alternatives(e.second, max))
            out << e.first + w;
        return out;
    }
    const QString lower = word.toLower();
    Ordered out;
    auto add = [&](const QString &w) {
        if (w.compare(word, Qt::CaseInsensitive) != 0 && out.size() < max)
            out.add(casedLike(word, w));
    };
    if (m_lang.english()) {
        auto c = contractions().find(lower);
        if (c != contractions().end()) add(c.value());
        auto a = ambiguous().find(lower);
        if (a != ambiguous().end()) add(a.value());
    }
    QHash<QString, float> one;
    edits(lower, 0.0f, one, true);
    one.remove(lower);
    // A word tapped to be fixed was most likely the wrong key or vowel, or a
    // swipe that came out a near neighbour: the same length, and closeness
    // counts for more than how common a word is.
    QHash<QString, float> near;
    for (auto it = one.cbegin(); it != one.cend(); ++it) {
        const QString &s = it.key();
        float cost = it.value();
        if (s.length() != lower.length()) cost += 0.6f;
        else if (vowelSwap(s, lower)) cost = 0.4f;   // ball, bell, bill, bull
        near.insert(s, cost);
    }
    for (const Scored &w : pickScored(near, 1.9f, 3, max + 1, 2 * COST_WEIGHT))
        add(w.first);
    if (out.size() < max && lower.length() >= 4) {
        QHash<QString, float> two;
        for (auto it = one.cbegin(); it != one.cend(); ++it)
            edits(it.key(), it.value(), two, false);
        two.remove(lower);
        for (const Scored &w : pickScored(two, 1.9f, 9, max + 1, COST_WEIGHT))
            add(w.first);
    }
    for (const QString &w : completions(lower, max + 1))
        add(w);
    return out.list();
}

bool Corrector::vowelSwap(const QString &a, const QString &b)
{
    int at = -1;
    for (int i = 0; i < a.length(); ++i) {
        if (a[i] != b[i]) {
            if (at >= 0) return false;
            at = i;
        }
    }
    return at >= 0 && VOWELS.contains(a[at]) && VOWELS.contains(b[at]);
}

QStringList Corrector::completions(const QString &prefix, int max) const
{
    auto lo = std::lower_bound(m_sorted.cbegin(), m_sorted.cend(), prefix);
    QList<const Entry *> found;
    int looked = 0;
    for (auto i = lo; i != m_sorted.cend() && i->startsWith(prefix) && looked < COMPLETION_SCAN; ++i, ++looked) {
        if (i->length() > prefix.length()) {
            auto it = m_words.constFind(*i);
            if (it != m_words.cend())
                found << &it.value();
        }
    }
    std::stable_sort(found.begin(), found.end(), [](const Entry *a, const Entry *b) { return a->score > b->score; });
    QStringList out;
    for (int i = 0; i < found.size() && i < max; ++i)
        out << found[i]->word;
    return out;
}

QStringList Corrector::nearest(const QString &w, int max) const
{
    QStringList out;
    for (const Scored &s : nearestScored(w, max))
        out << s.first;
    return out;
}

QList<Corrector::Scored> Corrector::nearestScored(const QString &w, int max) const
{
    QHash<QString, float> one;
    edits(w, 0.0f, one, true);
    const QList<Scored> found = pickScored(one, w.length() <= 3 ? 0.6f : 1.3f, w.length() <= 3 ? 12 : 5, max, COST_WEIGHT);
    if (!found.isEmpty())
        return found;
    if (w.length() < 5)
        return {};
    // Two slips: grow every one-slip spelling once more, with a narrower set of
    // edits so a long word stays quick even on an old phone.
    QHash<QString, float> two;
    for (auto it = one.cbegin(); it != one.cend(); ++it)
        edits(it.key(), it.value(), two, false);
    return pickScored(two, 1.9f, 9, max, COST_WEIGHT);
}

int Corrector::halfScore(const QString &w)
{
    switch (w.length()) {
    case 1: return 24;
    case 2: return 18;
    case 3: return 14;
    default: return 10;
    }
}

Corrector::Scored Corrector::split(const QString &w) const
{
    if (w.length() < 3)
        return {};
    Scored best;
    bool have = false;
    auto consider = [&](const QString &a, const QString &b, float cost) {
        auto ea = m_words.find(a);
        if (ea == m_words.end()) return;
        auto eb = m_words.find(b);
        if (eb == m_words.end()) return;
        if (ea->score < halfScore(a) || eb->score < halfScore(b)) return;
        const bool paired = follows(a, b);
        // Two little words that don't go together ("to on" for "toon") are
        // likelier a slip in one word, so such a split costs more and loses to
        // a common word nearby ("too"), but beats a rare one, and stands when
        // nothing is near ("nownin" -> "now in").
        const bool little = !paired && a.length() + b.length() < 6 && std::max(a.length(), b.length()) < 4;
        const float pair = (paired ? 0.0f : UNPAIRED) + (little ? LITTLE_PAIR : 0.0f);
        const float value = std::min(ea->score, eb->score) - COST_WEIGHT * cost - pair;
        if (!have || value > best.second) {
            best = { ea->word + QLatin1Char(' ') + eb->word, value };
            have = true;
        }
    };
    for (int i = 1; i < w.length(); ++i)
        consider(w.left(i), w.mid(i), MISSED_SPACE);
    for (int i = 1; i < w.length() - 1; ++i)
        if (ABOVE_SPACE.contains(w[i]))
            consider(w.left(i), w.mid(i + 1), SPACE_AS_LETTER);
    return best;
}

QList<Corrector::Scored> Corrector::pickScored(const QHash<QString, float> &cands, float maxCost, int minScore,
                                               int max, float weight) const
{
    QList<QPair<const Entry *, float>> found;
    for (auto it = cands.cbegin(); it != cands.cend(); ++it) {
        if (it.value() > maxCost)
            continue;
        auto e = m_words.constFind(it.key());
        if (e == m_words.cend() || e->score < minScore)
            continue;
        found.append({ &e.value(), e->score - weight * it.value() });
    }
    std::stable_sort(found.begin(), found.end(),
                     [](const QPair<const Entry *, float> &a, const QPair<const Entry *, float> &b) {
                         return a.second > b.second;
                     });
    QList<Scored> out;
    for (int i = 0; i < found.size() && i < max; ++i)
        out << Scored(found[i].first->word, found[i].second);
    return out;
}

void Corrector::edits(const QString &w, float base, QHash<QString, float> &out, bool full) const
{
    auto add = [&](const QString &s, float c) {
        const float total = base + c;
        auto had = out.find(s);
        if (had == out.end()) out.insert(s, total);
        else if (total < had.value()) had.value() = total;
    };
    const int n = w.length();
    if (!m_lang.accents.isEmpty())
        accentEdits(w, [&](const QString &s) { add(s, ACCENT); });
    QString sb;
    for (int i = 0; i < n; ++i) {
        // An extra letter: cheaper when it doubles its neighbour.
        sb = w.left(i) + w.mid(i + 1);
        add(sb, (i > 0 && w[i] == w[i - 1]) ? 0.5f : adjacentToNeighbour(w, i) ? 0.7f : 1.0f);
        // Two letters swapped.
        if (i < n - 1 && w[i] != w[i + 1]) {
            sb = w.left(i) + w[i + 1] + w[i] + w.mid(i + 2);
            add(sb, 0.6f);
        }
        // The wrong key.
        for (QChar c : ALPHABET) {
            if (c == w[i]) continue;
            float cost;
            if (adjacent(w[i], c)) cost = 0.6f;
            else if (VOWELS.contains(w[i]) && VOWELS.contains(c)) cost = 0.9f;
            else if (full) cost = 1.3f;
            else continue;
            sb = w;
            sb[i] = c;
            add(sb, cost);
        }
    }
    // A missed letter.
    for (int i = 0; i <= n; ++i) {
        for (QChar c : ALPHABET) {
            const bool doubling = (i > 0 && w[i - 1] == c) || (i < n && w[i] == c);
            if (!full && !doubling && !VOWELS.contains(c) &&
                !(i > 0 && adjacent(w[i - 1], c)) && !(i < n && adjacent(w[i], c)))
                continue;
            sb = w.left(i) + c + w.mid(i);
            add(sb, doubling ? 0.5f : 1.0f);
        }
    }
}

void Corrector::accentEdits(const QString &w, const std::function<void(const QString &)> &add) const
{
    for (int i = 0; i < w.length(); ++i) {
        const QChar b = Language::base(w[i]);
        const QString forms = m_lang.accentsOf(b) + (b != w[i] ? QString(b) : QString());
        for (QChar c : forms) {
            if (c == w[i]) continue;
            QString sb = w;
            sb[i] = c;
            add(sb);
        }
    }
    static const QList<QPair<QString, QChar>> joined = {
        { QStringLiteral("ss"), QChar(0xDF) }, { QStringLiteral("oe"), QChar(0x153) }, { QStringLiteral("ae"), QChar(0xE6) },
    };
    for (const auto &j : joined) {
        if (!m_lang.accents.contains(j.second)) continue;
        int at = w.indexOf(j.first);
        while (at >= 0) {
            add(w.left(at) + j.second + w.mid(at + 2));
            at = w.indexOf(j.first, at + 1);
        }
    }
}

bool Corrector::adjacentToNeighbour(const QString &w, int i) const
{
    return (i > 0 && adjacent(w[i], w[i - 1])) || (i < w.length() - 1 && adjacent(w[i], w[i + 1]));
}

bool Corrector::adjacent(QChar a, QChar b) const
{
    auto p = m_position.find(a);
    if (p == m_position.end()) return false;
    auto q = m_position.find(b);
    if (q == m_position.end()) return false;
    return std::abs(p->first - q->first) <= 1 && std::abs(p->second - q->second) <= 1;
}

QString Corrector::casedLike(const QString &typed, const QString &word)
{
    if (!typed.isEmpty() && !word.isEmpty() && typed[0].isUpper() && word[0].isLower())
        return word[0].toUpper() + word.mid(1);
    return word;
}

} // namespace Pre
