/*
 * This file is part of the LuneOS keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
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

#include "correction.h"

#include <QHash>

namespace MaliitKeyboard {
namespace Logic {
namespace {

//! The Android Pre Keyboard's (Corrector.CONTRACTIONS).
const QHash<QString, QString> &contractions()
{
    static const QHash<QString, QString> table = {
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
    return table;
}

//! Words elided before a vowel or a silent h (the Android Pre Keyboard's), longest first.
QStringList elisions(const QString &family)
{
    if (family == QLatin1String("fr"))
        return { "jusqu", "lorsqu", "puisqu", "quoiqu", "quelqu", "presqu", "qu",
                 "l", "d", "j", "m", "n", "s", "t", "c" };
    if (family == QLatin1String("it"))
        return { "nient", "quest", "quell", "dell", "dall", "nell", "sull", "coll", "bell", "sant", "tutt",
                 "senz", "anch", "all", "cos", "dov", "com", "un", "l", "d", "c", "s", "m", "t", "v", "n" };
    return {};
}

//! The small words typed onto another without the space ("ofthe", "alot"): one of a split's two must be one.
QStringList small(const QString &family)
{
    if (family == QLatin1String("en"))
        return { "a", "an", "the", "of", "to", "in", "on", "at", "by", "for", "and", "or", "but", "is", "am",
                 "are", "was", "be", "it", "i", "my", "me", "we", "you", "he", "she", "they", "this", "that",
                 "with", "from", "as", "not", "so", "do", "no", "up", "if", "all", "lot" };
    if (family == QLatin1String("de"))
        return { "der", "die", "das", "den", "dem", "ein", "eine", "und", "in", "im", "zu", "an", "auf", "ist",
                 "es", "ich", "du", "er", "sie", "wir", "mit", "von", "nicht" };
    if (family == QLatin1String("fr"))
        return { "le", "la", "les", "de", "du", "des", "un", "une", "et", "à", "en", "je", "il", "elle", "on",
                 "ne", "pas", "que", "est" };
    if (family == QLatin1String("es"))
        return { "el", "la", "los", "las", "de", "del", "en", "y", "a", "un", "una", "que", "es", "no", "se" };
    if (family == QLatin1String("it"))
        return { "il", "lo", "la", "i", "gli", "le", "di", "e", "a", "in", "un", "una", "che", "non", "è" };
    if (family == QLatin1String("nl"))
        return { "de", "het", "een", "en", "van", "in", "op", "te", "is", "ik", "je", "niet", "dat" };
    return {};
}

//! The letters with accents in a language, each standing in for its plain letter.
QString accents(const QString &family)
{
    if (family == QLatin1String("fr")) return QStringLiteral("éèàçêùâôîëïûüÿ");
    if (family == QLatin1String("de")) return QStringLiteral("äöüß");
    if (family == QLatin1String("es")) return QStringLiteral("ñéáóíúü");
    if (family == QLatin1String("it")) return QStringLiteral("èàùòìéóí");
    if (family == QLatin1String("pt")) return QStringLiteral("ãõçáéíóúâêôà");
    if (family == QLatin1String("nl")) return QStringLiteral("ëïéèö");
    if (family == QLatin1String("sv") || family == QLatin1String("fi")) return QStringLiteral("åäö");
    if (family == QLatin1String("da") || family == QLatin1String("no")) return QStringLiteral("æøå");
    if (family == QLatin1String("cs")) return QStringLiteral("áčďéěíňóřšťúůýž");
    if (family == QLatin1String("pl")) return QStringLiteral("ąćęłńóśźż");
    if (family == QLatin1String("hu")) return QStringLiteral("áéíóöőúüű");
    return QString();
}

QChar plain(QChar c)
{
    if (c == QChar(0x00DF)) return QLatin1Char('s');
    if (c == QChar(0x0142)) return QLatin1Char('l');
    if (c == QChar(0x00F8)) return QLatin1Char('o');
    if (c == QChar(0x00E6)) return QLatin1Char('a');
    const QString d = c.decomposition();
    return d.isEmpty() ? c : d.at(0);
}

bool isApostrophe(QChar c) { return c == QLatin1Char('\'') || c == QChar(0x2019); }

bool allCaps(const QString &w) { return w.size() > 1 && w == w.toUpper() && w != w.toLower(); }

//! A suggestion fit to be put in for [typed]: not a part word ("auto-") nor an acronym typed small.
bool fit(const QString &typed, const QString &s)
{
    if (s.isEmpty() || s.startsWith(QLatin1Char('-')) || s.endsWith(QLatin1Char('-'))) return false;
    if (s.contains(QLatin1Char('-')) && !typed.contains(QLatin1Char('-'))) return false;
    return !allCaps(s) || allCaps(typed);
}

//! [w] with one letter given one of [marks] (its accented forms), each that the dictionary knows.
QString accented(const QString &w, const QString &marks, const Correction::Dictionary &d)
{
    const QString lower = w.toLower();
    for (int i = 0; i < lower.size(); ++i) {
        for (QChar m : marks) {
            if (plain(m) != lower.at(i)) continue;
            QString v = lower;
            v[i] = m;
            if (m == QChar(0x00DF)) {
                // ß stands for "ss".
                if (i + 1 >= lower.size() || lower.at(i + 1) != QLatin1Char('s')) continue;
                v = lower.left(i) + m + lower.mid(i + 2);
            }
            if (d.spell(v)) return v;
            // A German noun: with its capital.
            const QString capital = v.left(1).toUpper() + v.mid(1);
            if (d.spell(capital)) return capital;
        }
    }
    return QString();
}

//! [w], or the same word with its accents, if the dictionary knows one ("ecole" -> "école").
QString known(const QString &w, const QString &family, const Correction::Dictionary &d)
{
    if (d.spell(w)) return w;
    const QString f = Correction::fold(w);
    for (const QString &s : d.suggest(w))
        if (fit(w, s) && !s.contains(QLatin1Char(' ')) && Correction::fold(s) == f) return s;
    return accented(w, accents(family), d);
}

} // namespace

QString Correction::fold(const QString &word)
{
    QString out;
    out.reserve(word.size());
    for (QChar c : word.toLower()) {
        if (isApostrophe(c) || c == QLatin1Char('-') || c == QLatin1Char(' ')) continue;
        if (c == QChar(0x00DF)) { out += QLatin1String("ss"); continue; }
        out += plain(c);
    }
    return out;
}

QString Correction::casedLike(const QString &like, const QString &word)
{
    if (like.isEmpty() || word.isEmpty() || !like.at(0).isUpper() || !word.at(0).isLower()) return word;
    return word.left(1).toUpper() + word.mid(1);
}

QString Correction::choose(const QString &typed, const QString &language, const Dictionary &dictionary,
                           const QStringList &predictions)
{
    const Dictionary &d = dictionary;
    if (typed.isEmpty() || !d.spell || !d.suggest) return typed;
    const QString family = language.left(2).toLower();
    const QString lower = typed.toLower();

    if (family == QLatin1String("en")) {
        const QString c = contractions().value(lower);
        if (!c.isEmpty()) return casedLike(typed, c);
        if (lower == QLatin1String("i")) return QStringLiteral("I");
        if (lower.size() > 2 && lower.startsWith(QLatin1Char('i')) && isApostrophe(lower.at(1)))
            return QLatin1Char('I') + typed.mid(1);
    }
    if (d.spell(typed)) return typed;

    const QStringList suggested = d.suggest(typed);
    const QString folded = fold(typed);
    // Capitals ("Haus"), accents, apostrophes, a hyphen.
    for (const QString &s : suggested)
        if (fit(typed, s) && !s.contains(QLatin1Char(' ')) && fold(s) == folded) return casedLike(typed, s);
    // One letter's accent the checker missed.
    const QString a = accented(typed, accents(family), d);
    if (!a.isEmpty()) return casedLike(typed, a);
    // What presage expects, spelt as the checker would ("becaus" -> "because", not "beaut if").
    for (const QString &p : predictions)
        for (const QString &s : suggested)
            if (fit(typed, s) && s.compare(p, Qt::CaseInsensitive) == 0) return casedLike(typed, s);
    // Two letters the wrong way round ("taht" -> "that"), or two words run together ("alot" ->
    // "a lot"): whichever the checker puts first.
    const auto swapped = [&](const QString &s) {
        if (s.size() != typed.size()) return false;
        const QString t = s.toLower();
        qsizetype i = 0;
        while (i < t.size() && t.at(i) == lower.at(i)) ++i;
        return i + 1 < t.size() && t.at(i) == lower.at(i + 1) && t.at(i + 1) == lower.at(i)
               && t.mid(i + 2) == lower.mid(i + 2);
    };
    const auto runTogether = [&](const QString &s) {
        if (s.count(QLatin1Char(' ')) != 1 || fold(s) != folded) return false;
        // A word typed onto a small one: "of the", "a lot"; not "diff rent" for "diffrent".
        const QStringList parts = s.toLower().split(QLatin1Char(' '));
        const QStringList smallWords = small(family);
        const bool first = smallWords.contains(parts.at(0)), second = smallWords.contains(parts.at(1));
        // The other a word of its own, not a scrap: "he lo" is no "hello".
        return (first && second) || (first && parts.at(1).size() >= 3) || (second && parts.at(0).size() >= 3);
    };
    for (const QString &s : suggested)
        if (fit(typed, s) && (swapped(s) || runTogether(s))) return casedLike(typed, s);
    // An elision ("lecole" -> "l'école"), before a vowel or a silent h.
    if (!typed.contains(QLatin1Char('\'')) && !typed.contains(QChar(0x2019))) {
        for (const QString &e : elisions(family)) {
            if (lower.size() < e.size() + 2 || !lower.startsWith(e)) continue;
            const QString rest = known(lower.mid(e.size()), family, d);
            if (rest.isEmpty() || !QStringLiteral("aeiouyh").contains(plain(rest.at(0).toLower()))) continue;
            for (QChar apostrophe : { QChar(0x2019), QChar(QLatin1Char('\'')) }) {
                const QString w = typed.left(e.size()) + apostrophe + rest;
                if (d.spell(w)) return w;
            }
        }
    }
    // Two words only as above: "diff rent" is no "diffrent".
    for (const QString &s : suggested)
        if (fit(typed, s) && (!s.contains(QLatin1Char(' ')) || runTogether(s))) return casedLike(typed, s);
    return predictions.isEmpty() ? typed : casedLike(typed, predictions.first());
}

} // namespace Logic
} // namespace MaliitKeyboard
