/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
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

#include "precontroller.h"
#include "preklipy.h"
#include "prekeyboardview.h"
#include "preswipewords.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QLocale>
#include <QLoggingCategory>

#include <algorithm>
#include <limits>

Q_LOGGING_CATEGORY(lcPre, "luneos.keyboard.pre", QtInfoMsg)

namespace Pre {
namespace {

const QString SPACE = QStringLiteral("space");
const QString RETURN = QStringLiteral("return");
const QString BACKSPACE = QStringLiteral("backspace");
//! How many languages' words to keep loaded: switching back and forth between two is quick.
const int KEEP_WORDS = 2;
//! What a word the owner adds is worth to the swipe decoder: a fairly common word.
const int LEARNED_SCORE = 14;
//! Long before any space: no double space with it.
const qint64 NEVER = std::numeric_limits<qint64>::min() / 2;
//! Punctuation that goes against the word before it, never after a space.
const QString SENTENCE = QStringLiteral(".,?!:;");
//! What may close a word, for a space after it to be the word's.
const QString CLOSERS = QStringLiteral(")\"']}”’");
//! How far back to look for the address a period may be in.
const int MAX_TOKEN = 64;

//! [c] ends a word: a space after it is a word's.
bool endsWord(QChar c) { return c.isLetterOrNumber() || CLOSERS.contains(c); }

QString sha1(const QString &t)
{
    return QString::fromLatin1(QCryptographicHash::hash(t.toUtf8(), QCryptographicHash::Sha1).toHex());
}

} // namespace

Controller::Controller(QObject *parent)
    : QObject(parent)
    , m_lastSpaceAt(NEVER)
    , m_ownEditAt(NEVER)
{
    m_pool.setMaxThreadCount(1);
    // Something copied or cut while the keys are up: offered at once, not at the next keystroke.
    if (QClipboard *cb = QGuiApplication::clipboard())
        connect(cb, &QClipboard::dataChanged, this, &Controller::offerPaste);
}

void Controller::setBridge(const Bridge &bridge)
{
    m_bridge = bridge;
    // Whatever the keyboard does to the text: the cursor moving straight after is its doing.
    const auto own = [this] {
        m_ownEditAt = now();
        m_tapped = false;
    };
    if (bridge.press)
        m_bridge.press = [own, f = bridge.press](const QString &l, const QString &a) { own(); f(l, a); };
    if (bridge.pick) m_bridge.pick = [own, f = bridge.pick](const QString &w) { own(); f(w); };
    if (bridge.setWord) m_bridge.setWord = [own, f = bridge.setWord](const QString &w) { own(); f(w); };
    if (bridge.commit) m_bridge.commit = [own, f = bridge.commit](const QString &t) { own(); f(t); };
    if (bridge.replace)
        m_bridge.replace = [own, f = bridge.replace](int b, int a, const QString &t) { own(); f(b, a, t); };
}

Controller::~Controller()
{
    // Whatever is being worked out is moot; wait for it, as it reports back here.
    m_liveGeneration->fetch_add(1);
    m_pool.clear();
    m_pool.waitForDone();
}

void Controller::setWordDirs(const QString &libDir, const QString &dictDir)
{
    m_libDir = libDir;
    m_dictDir = dictDir;
    m_decoders.clear();
    m_decoderOrder.clear();
    m_decoder.reset();
    loadWords();
}

void Controller::setStateDir(const QString &dir)
{
    QDir().mkpath(dir);
    Klipy::instance().setStateDir(dir);
}

void Controller::setView(KeyboardView *view)
{
    m_view = view;
    if (m_view) {
        applySettings();
        showBar();
        offerPaste();
    }
}

void Controller::setActive(bool active)
{
    if (active == m_active) return;
    m_active = active;
    qCInfo(lcPre, "Pre Keyboard %s", active ? "on" : "off");
    Q_EMIT activeChanged();
}

void Controller::setPrefs(const QJsonObject &pre)
{
    m_prefs = pre;
    const Settings s = Settings::fromJson(pre, m_settings);
    if (s != m_settings) {
        m_settings = s;
        loadWords();
        applySettings();
    }
}

void Controller::setKeyboardState(bool suggestions, bool sound, const QStringList &enabledLanguages,
                                  const QString &activeLanguage)
{
    Settings s = m_settings;
    s.suggestions = suggestions;
    s.sound = sound;
    s.languages.clear();
    for (const QString &id : enabledLanguages) {
        const Language l = Language::byId(id);
        if (l.isValid() && !s.languages.contains(l)) s.languages << l;
    }
    const Language now = Language::orEnglish(activeLanguage);
    if (!s.languages.contains(now)) s.languages.prepend(now);
    s.language = now;
    if (s != m_settings) {
        m_settings = s;
        loadWords();
        applySettings();
        showBar();
    }
}

Settings Controller::shown() const
{
    Settings s = m_settings;
    s.swipe = s.swipe && m_decoder && m_decoder->size() > 0;
    return s;
}

void Controller::applySettings()
{
    if (!m_view) return;
    m_view->applySettings(shown());
    m_view->setWordsAllowed(m_wordsAllowed);
    m_view->setDigits(m_digits);
}

void Controller::setDigits(const QString &allowed)
{
    m_digits = allowed;
    if (m_view) m_view->setDigits(allowed);
}

void Controller::setWordsAllowed(bool allowed)
{
    if (allowed == m_wordsAllowed) return;
    m_wordsAllowed = allowed;
    if (m_view) m_view->setWordsAllowed(allowed);
    showBar();
}

void Controller::writePrefs(const QJsonObject &pre)
{
    m_prefs = pre;
    if (m_bridge.savePrefs) m_bridge.savePrefs(pre);
}

void Controller::startInput()
{
    m_deleting = false;
    m_lastSpaceAt = NEVER;
    m_autoTail.clear();
    m_spaceIsLast = false;
    m_swiped.clear();
    m_swipeGuesses.clear();
    m_live.clear();
    if (m_view) {
        applySettings();
        m_view->reset();
    }
    showBar();
    offerPaste();
}

void Controller::finishInput()
{
    if (m_view) m_view->hidden();
    if (m_deleting) {
        m_deleting = false;
        if (m_bridge.cancelRepeat) m_bridge.cancelRepeat();
    }
}

void Controller::update()
{
    if (m_view) m_view->updateCaps();
    offerPaste();
    // The cursor moved, and not by anything the keyboard did: the owner tapped the text.
    const int position = m_bridge.position ? m_bridge.position() : -1;
    if (position != m_position) {
        m_position = position;
        if (position >= 0 && now() - m_ownEditAt > OWN_EDIT_MS) {
            m_tapped = true;
            m_fixTyped.clear();
            m_fixed.clear();
        }
    }
    // The application told where its cursor is: the next words, or the tapped word's fixes, follow it.
    if (m_typed.isEmpty() && m_swiped.isEmpty()) {
        const QStringList next = idleWords();
        if (next != m_candidates) {
            m_candidates = next;
            showBar();
        }
    }
}

bool Controller::wantsCaps()
{
    return m_bridge.autoCaps && m_bridge.autoCaps();
}

// --- The bar --------------------------------------------------------------------------------

void Controller::setCandidates(const QString &typed, const QString &primary, const QStringList &words)
{
    // Typed on, picked from the bar, or put in: the swiped word is no longer the one being typed.
    if (!m_swiped.isEmpty() && typed != m_swiped) {
        m_swiped.clear();
        m_swipeGuesses.clear();
    }
    m_typed = typed;
    m_primary = primary;
    m_candidates = words.isEmpty() && typed.isEmpty() ? idleWords() : words;
    if (!typed.isEmpty()) m_barWord.clear();
    showBar();
}

QStringList Controller::idleWords()
{
    const QStringList fixes = wordFixes();
    return fixes.isEmpty() ? nextWords() : fixes;
}

namespace {

bool wordChar(const QString &text, qsizetype i)
{
    const QChar c = text.at(i);
    if (c.isLetter() || c.isMark()) return true;
    // An apostrophe inside a word - don't, it's - and not one that quotes it.
    return (c == QLatin1Char('\'') || c == QChar(0x2019)) && i > 0 && i + 1 < text.size()
           && text.at(i - 1).isLetter() && text.at(i + 1).isLetter();
}

//! [word] with its first letter a capital when [like]'s is.
QString casedLike(const QString &like, const QString &word)
{
    if (like.isEmpty() || word.isEmpty() || !like.at(0).isUpper() || !word.at(0).isLower()) return word;
    return word.left(1).toUpper() + word.mid(1);
}

} // namespace

QString Controller::wordAround(const QString &before, const QString &after, int *nBefore, int *nAfter)
{
    *nBefore = 0;
    *nAfter = 0;
    const QString text = before + after;
    const qsizetype cursor = before.size();
    qsizetype start = cursor, end = cursor;
    while (start > 0 && wordChar(text, start - 1)) --start;
    while (end < text.size() && wordChar(text, end)) ++end;
    if (start == end) return QString();
    // Running on past what was asked for: too long to be a word.
    if ((start == 0 && before.size() >= MAX_TOKEN) || (end == text.size() && after.size() >= MAX_TOKEN))
        return QString();
    // Part of something that isn't a word: 3rd, an address, a link, a #tag.
    static const QString joinsBefore = QStringLiteral("@#_/.-");
    static const QString joinsAfter = QStringLiteral("@_/");
    if (start > 0 && (text.at(start - 1).isDigit() || joinsBefore.contains(text.at(start - 1)))) return QString();
    if (end < text.size() && (text.at(end).isDigit() || joinsAfter.contains(text.at(end)))) return QString();
    if (!text.at(start).isLetter()) return QString();
    *nBefore = int(cursor - start);
    *nAfter = int(end - cursor);
    return text.mid(start, end - start);
}

QStringList Controller::wordFixes()
{
    m_barWord.clear();
    if (!m_settings.suggestions || !m_wordsAllowed || !m_bridge.alternatives || !m_bridge.replace
        || !m_bridge.before || !m_bridge.after || (m_bridge.prose && !m_bridge.prose()))
        return QStringList();
    const QString before = m_bridge.before(MAX_TOKEN), after = m_bridge.after(MAX_TOKEN);
    int nb = 0, na = 0;
    const QString word = wordAround(before, after, &nb, &na);
    // Tapped on, or the cursor inside it (not just after a word typed).
    if (word.isEmpty() || (!m_tapped && na == 0)) return QStringList();
    const QString left = before.left(before.size() - nb);
    const QString key = left + QChar(0) + word;
    if (key != m_fixesFor) {
        m_fixesFor = key;
        m_fixes.clear();
        for (const QString &w : m_bridge.alternatives(word, left)) {
            const QString c = casedLike(word, w);
            if (!w.isEmpty() && w.compare(word, Qt::CaseInsensitive) != 0 && !m_fixes.contains(c)) m_fixes << c;
            if (m_fixes.size() == 3) break;
        }
    }
    if (!m_fixes.isEmpty()) {
        m_barWord = word;
        m_barWordBefore = nb;
        m_barWordAfter = na;
    }
    return m_fixes;
}

void Controller::corrected(const QString &typed, const QString &fixed)
{
    m_fixTyped = typed;
    m_fixed = fixed;
    vibrate(CORRECTED_MS);
}

bool Controller::undoCorrection()
{
    if (m_fixed.isEmpty() || !m_bridge.before || !m_bridge.replace) return false;
    // After a period: just its space, here; the word's correction waits for the next backspace.
    if (autoSpaced()) return false;
    const QString typed = m_fixTyped, fixed = m_fixed;
    m_fixTyped.clear();
    m_fixed.clear();
    // The word, then what finished it (a space or punctuation), still the last thing typed.
    const QString b = m_bridge.before(int(fixed.size()) + 2);
    const qsizetype at = b.size() - fixed.size() - 1;
    if (at < 0 || b.mid(at, fixed.size()) != fixed || b.back().isLetterOrNumber() || (at > 0 && wordChar(b, at - 1)))
        return false;
    m_bridge.replace(int(fixed.size()) + 1, 0, typed + b.back());
    return true;
}

QStringList Controller::nextWords()
{
    if (!m_settings.suggestions || !m_wordsAllowed || !m_bridge.nextWords || !m_bridge.before
        || (m_bridge.prose && !m_bridge.prose()))
        return QStringList();
    const QString before = m_bridge.before(MAX_TOKEN);
    // Right after a word, or in one, it is that word that is being typed.
    if (!before.isEmpty() && !before.at(before.size() - 1).isSpace()) return QStringList();
    if (before != m_nextFor || m_next.isEmpty()) {
        m_nextFor = before;
        m_next = m_bridge.nextWords();
    }
    return m_next;
}

/*!
 * The Pre's bar: what space would put in, in the middle. While a word is typed
 * and that is a correction, the word as typed is on the left, to keep it; the
 * next likeliest fills the other places.
 */
void Controller::showBar()
{
    QStringList bar;
    const QStringList &guesses = !m_live.isEmpty() ? m_live : m_swipeGuesses;
    if (shown().showsBar() && m_wordsAllowed && !guesses.isEmpty()) {
        // A swipe: the word it typed (or would, lifted now) in the middle, the next guesses either side.
        const QString best = m_live.isEmpty() ? m_swiped : m_live.value(0);
        QStringList others;
        for (const QString &w : guesses) if (w != best) others << w;
        bar = { others.value(0), best, others.value(1) };
    } else if (shown().showsBar() && m_wordsAllowed) {
        // What space puts in: the correction, else the word as typed. With no
        // word typed, the likeliest next word.
        const QString best = !m_primary.isEmpty() ? m_primary
                           : !m_typed.isEmpty() ? m_typed : m_candidates.value(0);
        QStringList others;
        for (const QString &w : m_candidates)
            if (!w.isEmpty() && w != best && w != m_typed && !others.contains(w)) others << w;
        if (!m_typed.isEmpty() && !best.isEmpty() && best != m_typed)
            bar = { m_typed, best, others.value(0) };
        else
            bar = { others.value(0), best, others.value(1) };
    }
    // An emoji for the word being typed or swiped (not for the next words).
    m_barEmoji = false;
    if (m_settings.emojiSuggestions && m_wordsAllowed && !bar.value(1).isEmpty()
        && (!m_typed.isEmpty() || !guesses.isEmpty())) {
        const QString e = emojiWords().forWord(bar.value(1));
        if (!e.isEmpty()) {
            bar = { bar.value(0), bar.value(1), e };
            m_barEmoji = true;
        }
    }
    m_bar = { bar.value(0), bar.value(1), bar.value(2) };
    if (m_view) m_view->setSuggestions(m_bar, m_barEmoji);
}

const EmojiWords &Controller::emojiWords()
{
    const QString id = m_settings.language.id;
    if (id != m_emojiFor) {
        m_emojiFor = id;
        // Only the emoji the emoji page has (emoji.txt: category, version, emoji).
        QSet<QString> drawable;
        QFile f(emojiFile());
        if (f.open(QIODevice::ReadOnly)) {
            for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
                const QStringList parts = line.split(QLatin1Char('\t'));
                if (parts.size() == 3 && !line.startsWith(QLatin1Char('#'))) drawable.insert(parts.at(2).trimmed());
            }
        }
        m_emojiWords = drawable.isEmpty() ? EmojiWords()
                     : EmojiWords::load(m_dataDir + QLatin1Char('/') + id + QStringLiteral("/emoji_words.txt"), drawable);
        qCInfo(lcPre, "emoji for %d words in %s", m_emojiWords.size(), qPrintable(id));
    }
    return m_emojiWords;
}

void Controller::onPick(int i, int caps)
{
    QString w = m_bar.value(i);
    if (w.isEmpty() || !m_bridge.pick) return;
    m_fixTyped.clear();
    m_fixed.clear();
    if (!m_barWord.isEmpty()) {
        // In place of the word tapped, if it is still there; the bar goes on offering for the new one.
        int nb = 0, na = 0;
        const QString word = m_bridge.before && m_bridge.after
                           ? wordAround(m_bridge.before(MAX_TOKEN), m_bridge.after(MAX_TOKEN), &nb, &na) : QString();
        const bool same = word == m_barWord && nb == m_barWordBefore && na == m_barWordAfter;
        m_barWord.clear();
        if (same) {
            if (caps == KeyboardView::CAPS_ALL) w = QLocale(m_settings.language.id).toUpper(w);
            m_bridge.replace(nb, na, w);
            m_tapped = true;
        }
        m_candidates = idleWords();
        showBar();
        return;
    }
    if (i == 2 && m_barEmoji) {
        // In place of the word, as it is: shift is nothing to an emoji.
        m_bridge.pick(w);
        addRecentEmoji(w);
    } else {
        if (caps == KeyboardView::CAPS_ALL) w = QLocale(m_settings.language.id).toUpper(w);
        else if (caps == KeyboardView::CAPS_FIRST) w = w.left(1).toUpper() + w.mid(1);
        m_bridge.pick(w);
    }
    // It goes in with a space after it: the first of a double space, and a
    // space a comma goes in front of.
    m_autoTail.clear();
    m_spaceIsLast = true;
    m_lastSpaceAt = now();
}

bool Controller::canAdd(const QString &word)
{
    return !word.isEmpty() && m_bridge.canAdd && m_bridge.canAdd(word);
}

void Controller::onAddWord(const QString &word)
{
    // A word deleted before is wanted after all.
    const QString id = m_settings.language.id;
    QStringList removed = Settings::removed(m_prefs, id);
    if (removed.removeAll(word) > 0) writePrefs(Settings::withRemoved(m_prefs, id, removed));
    if (m_bridge.addWord) m_bridge.addWord(word);
    // Swiped from now on too.
    if (m_decoder) m_decoder->add(word, LEARNED_SCORE);
}

bool Controller::canRemove(const QString &word)
{
    return !word.isEmpty() && m_bridge.canAdd && !m_bridge.canAdd(word);
}

void Controller::onRemoveWord(const QString &word)
{
    if (word.isEmpty()) return;
    const QString id = m_settings.language.id;
    QStringList removed = Settings::removed(m_prefs, id);
    if (!removed.contains(word)) removed << word;
    // Saved first: the input method blocks the "removed" words in the word
    // engine as the preference comes back, and the bridge's removal asks it again.
    writePrefs(Settings::withRemoved(m_prefs, id, removed));
    if (m_bridge.removeWord) m_bridge.removeWord(word);
    // Not swiped either, and gone from the bar now.
    if (m_decoder) m_decoder->forget(word);
    m_candidates.removeAll(word);
    m_swipeGuesses.removeAll(word);
    m_live.removeAll(word);
    if (m_primary == word) m_primary.clear();
    showBar();
}

// --- Keys -----------------------------------------------------------------------------------

void Controller::onText(const QString &text)
{
    if (text.isEmpty()) return;
    // Anything typed moves on from the last correction (this may make another).
    m_fixTyped.clear();
    m_fixed.clear();
    // A character goes in as a LuneOS key's would, to be predicted and corrected;
    // anything longer (an emoji, an emoticon) as it is.
    if (text == QLatin1String(" ") && !m_swiped.isEmpty() && m_typed == m_swiped && m_bridge.pick) {
        // A swiped word goes in as it was swiped: autocorrect is for letters tapped wrong.
        m_bridge.pick(m_swiped);
        m_autoTail.clear();
        m_spaceIsLast = true;
        m_lastSpaceAt = now();
    } else if (text.length() == 1) {
        if (!punctuate(text)) key(text);
    } else {
        m_autoTail.clear();
        m_spaceIsLast = false;
        m_lastSpaceAt = NEVER;
        if (m_bridge.commit) m_bridge.commit(text);
    }
}

void Controller::key(const QString &text)
{
    const QString action = text == QLatin1String(" ") ? SPACE : QString();
    if (m_bridge.press) m_bridge.press(text, action);
    if (m_bridge.release) m_bridge.release(text, action);
}

void Controller::backspaceKey()
{
    if (m_bridge.press) m_bridge.press(QString(), BACKSPACE);
    if (m_bridge.release) m_bridge.release(QString(), BACKSPACE);
}

qint64 Controller::now() const
{
    if (m_clock) return m_clock();
    static QElapsedTimer started;
    if (!started.isValid()) started.start();
    return started.elapsed();
}

bool Controller::autoSpaced() const
{
    return !m_autoTail.isEmpty() && m_bridge.before && m_bridge.before(int(m_autoTail.size())) == m_autoTail;
}

bool Controller::spaceAfterWord(bool *digit) const
{
    if (!m_spaceIsLast || !m_bridge.before) return false;
    const QString b = m_bridge.before(2);
    const bool is = b.size() == 2 && b.at(1) == QLatin1Char(' ') && endsWord(b.at(0));
    if (digit) *digit = is && b.at(0).isDigit();
    return is;
}

bool Controller::periodWantsSpace(const QString &before, const QString &after)
{
    // [before] ends in the period itself.
    if (before.size() < 2) return false;
    const QChar prev = before.at(before.size() - 2);
    if (prev.isSpace() || prev.isDigit()) return false;
    QString token = before.left(before.size() - 1);
    token = token.mid(token.lastIndexOf(QLatin1Char(' ')) + 1);
    token = token.mid(token.lastIndexOf(QLatin1Char('\n')) + 1);
    if (token.contains(QLatin1Char('@')) || token.contains(QLatin1Char('/'))
        || token.startsWith(QLatin1String("www"), Qt::CaseInsensitive))
        return false;
    return after.isEmpty() || !after.at(0).isSpace();
}

bool Controller::punctuate(const QString &text)
{
    const qint64 t = now();
    const bool space = text == QLatin1String(" ");
    const bool spaceAgain = space && t - m_lastSpaceAt < DOUBLE_SPACE_MS;
    const bool spaced = autoSpaced();
    bool afterDigit = false;
    const bool typedSpace = spaceAfterWord(&afterDigit);
    m_autoTail.clear();
    m_spaceIsLast = false;
    m_lastSpaceAt = NEVER;
    const bool prose = m_bridge.prose ? m_bridge.prose() : m_wordsAllowed;
    if (!prose || !m_bridge.before) {
        if (space) m_lastSpaceAt = t;
        return false;
    }

    // A space on top of the keyboard's own: already there. It still counts toward a double space.
    if (space && spaced) {
        m_lastSpaceAt = t;
        return true;
    }
    if (space && m_settings.doubleSpacePeriod && spaceAgain && typedSpace) {
        backspaceKey();
        key(QStringLiteral("."));
        key(QStringLiteral(" "));
        m_autoTail = QStringLiteral(". ");
        return true;
    }
    if (space) {
        key(text);
        m_spaceIsLast = true;
        m_lastSpaceAt = t;
        return true;
    }

    // A period after a number and a space starts something (3 .5); after a word, it ends it.
    const bool snap = SENTENCE.contains(text.at(0))
                      && (spaced || (typedSpace && !(afterDigit && text == QLatin1String("."))));
    // What the text will be once [text] is in, to tell whether the period wants its space.
    QString before = m_bridge.before(MAX_TOKEN);
    if (snap) {
        backspaceKey();
        before.chop(1);
    }
    key(text);
    before += text;
    if (text == QLatin1String(".") && periodWantsSpace(before, m_bridge.after ? m_bridge.after(1) : QString())) {
        key(QStringLiteral(" "));
        m_autoTail = QStringLiteral(". ");
    } else if (snap && typedSpace) {
        // The owner's space moves to after the punctuation, where it belongs.
        key(QStringLiteral(" "));
        m_autoTail = text + QLatin1Char(' ');
    }
    return true;
}

void Controller::onBackspace(bool)
{
    const bool undone = undoCorrection();
    // Straight after a period, it takes just the period's space.
    m_autoTail.clear();
    m_spaceIsLast = false;
    m_lastSpaceAt = NEVER;
    if (!undone) backspaceKey();
}

void Controller::onBackspaceHeld(bool held, bool lifted)
{
    // The editor repeats a held backspace, and goes over to whole words, as it
    // does for LuneOS's own keys.
    if (held) {
        m_autoTail.clear();
        m_spaceIsLast = false;
        m_lastSpaceAt = NEVER;
        m_deleting = true;
        if (m_bridge.press) m_bridge.press(QString(), BACKSPACE);
        return;
    }
    if (!m_deleting) return;
    m_deleting = false;
    if (lifted) {
        if (m_bridge.release) m_bridge.release(QString(), BACKSPACE);
    } else if (m_bridge.cancelRepeat) {
        m_bridge.cancelRepeat();
    }
}

void Controller::onEnter()
{
    m_fixTyped.clear();
    m_fixed.clear();
    // The keyboard's own space is not left at the end of the line.
    if (autoSpaced()) backspaceKey();
    m_autoTail.clear();
    m_spaceIsLast = false;
    m_lastSpaceAt = NEVER;
    if (m_bridge.press) m_bridge.press(QString(), RETURN);
    if (m_bridge.release) m_bridge.release(QString(), RETURN);
}

void Controller::onCursor(int dx, int dy, bool select)
{
    if (m_bridge.cursor) m_bridge.cursor(dx, dy, select);
}

// --- Swipe to type ---------------------------------------------------------------------------

/*
 * The words to swipe in the language typed in now: from the word engine's own
 * data for it (see SwipeWords), loaded off the keyboard's thread the first
 * time; swiping is off till they are in. A few languages' are kept.
 */
void Controller::loadWords()
{
    const QString id = m_settings.language.id;
    if (!m_settings.swipe || id.isEmpty() || (m_libDir.isEmpty() && m_dictDir.isEmpty())) return;
    const QSharedPointer<SwipeDecoder> have = m_decoders.value(id);
    if (have) {
        if (m_decoder != have) { m_decoder = have; applySettings(); }
        return;
    }
    if (m_decoder) { m_decoder.reset(); applySettings(); }
    if (m_loading == id) return;
    m_loading = id;
    const QString lib = m_libDir, dict = m_dictDir;
    m_pool.start([this, id, lib, dict] {
        QElapsedTimer t;
        t.start();
        QSharedPointer<SwipeDecoder> words(new SwipeDecoder(SwipeWords::load(id, lib, dict)));
        qCInfo(lcPre, "swipe: %d words for %s in %lld ms", words->size(), qPrintable(id), t.elapsed());
        // The controller waits for this before it goes (see ~Controller), so it is here to take them.
        QMetaObject::invokeMethod(this, [this, id, words] { wordsLoaded(id, words); }, Qt::QueuedConnection);
    });
}

void Controller::wordsLoaded(const QString &id, const QSharedPointer<SwipeDecoder> &words)
{
    if (m_loading == id) m_loading.clear();
    m_decoders.insert(id, words);
    m_decoderOrder.removeAll(id);
    m_decoderOrder << id;
    while (m_decoderOrder.size() > KEEP_WORDS) m_decoders.remove(m_decoderOrder.takeFirst());
    if (id == m_settings.language.id) {
        m_decoder = words;
        applySettings();
        showBar();
    } else {
        loadWords();
    }
    Q_EMIT swipeWordsLoaded();
}

QString Controller::cased(const QString &word, int caps) const
{
    if (caps == KeyboardView::CAPS_ALL) return QLocale(m_settings.language.id).toUpper(word);
    if (caps == KeyboardView::CAPS_FIRST && !word.isEmpty()) return word.left(1).toUpper() + word.mid(1);
    return word;
}

/*
 * The finger lifted: its words are worked out off the keyboard's thread (a
 * language with a third of a million words takes a while on a phone), then
 * swiped() puts them in.
 */
void Controller::onSwipe(const QVector<float> &xs, const QVector<float> &ys, int caps)
{
    // Guesses still being worked out for the moving finger are moot now.
    const quint64 generation = m_liveGeneration->fetch_add(1) + 1;
    m_live.clear();
    SwipeDecoder::Layout layout;
    if (!m_decoder || !m_view || !m_view->swipeLayout(layout) || !m_bridge.setWord) { showBar(); return; }
    const QSharedPointer<SwipeDecoder> words = m_decoder;
    m_pool.start([this, words, generation, xs, ys, layout, caps] {
        QElapsedTimer t;
        t.start();
        const QStringList found = words->decode(xs, ys, layout, 4);
        qCDebug(lcPre, "swipe %llu: %d points, %lld ms, %s", generation, int(xs.size()), t.elapsed(),
                qPrintable(found.join(QLatin1Char(' '))));
        // The controller waits for this before it goes (see ~Controller).
        QMetaObject::invokeMethod(this, [this, found, caps] { swiped(found, caps); }, Qt::QueuedConnection);
    });
}

/*
 * A swipe's words, best first: the best goes in as the word being typed, the
 * next guesses either side of it in the bar, to swap it for. A word typed
 * before it goes in first, as space would put it (a swiped one as it was), and
 * a space goes between it and a word the cursor is right after.
 */
void Controller::swiped(QStringList found, int caps)
{
    if (found.isEmpty() || !m_bridge.setWord) { showBar(); return; }
    for (QString &w : found) w = cased(w, caps);
    if (!m_typed.isEmpty()) {
        if (m_typed == m_swiped && m_bridge.pick) m_bridge.pick(m_swiped);
        else onText(QStringLiteral(" "));
    } else if (m_bridge.atWordEnd && m_bridge.atWordEnd()) {
        onText(QStringLiteral(" "));
    }
    m_swiped = found.first();
    m_swipeGuesses = found;
    m_bridge.setWord(m_swiped);
    showBar();
}

/*
 * Guesses while the finger is still moving: the word it would be if lifted
 * now, in the middle, between the next two. Worked out off the keyboard's
 * thread so the trail never stutters; a newer piece of the swipe, or its end,
 * makes an older one moot.
 */
void Controller::onSwipeProgress(const QVector<float> &xs, const QVector<float> &ys, int caps)
{
    SwipeDecoder::Layout layout;
    if (!m_decoder || !m_view || !m_view->swipeLayout(layout)) return;
    const quint64 generation = m_liveGeneration->fetch_add(1) + 1;
    const QSharedPointer<SwipeDecoder> words = m_decoder;
    const std::shared_ptr<std::atomic<quint64>> latest = m_liveGeneration;
    // One at a time: a piece the pool hasn't got to yet finds it is moot and stops at once.
    m_pool.start([this, words, latest, generation, xs, ys, layout, caps] {
        if (latest->load() != generation) return;
        const QStringList found = words->decode(xs, ys, layout, 3, {}, [&] { return latest->load() != generation; });
        if (latest->load() != generation) return;
        QMetaObject::invokeMethod(this, [this, generation, found, caps] {
            QStringList cased;
            for (const QString &w : found) cased << this->cased(w, caps);
            liveGuesses(generation, cased);
        }, Qt::QueuedConnection);
    });
}

void Controller::liveGuesses(quint64 generation, const QStringList &found)
{
    if (m_liveGeneration->load() != generation || found.isEmpty()) return;
    m_live = found;
    showBar();
}

// --- Settings and languages -----------------------------------------------------------------

void Controller::onToggleStyle()
{
    m_settings.style = m_settings.style == Style::Orange ? Style::White : Style::Orange;
    writePrefs(m_settings.toJson(m_prefs));
    applySettings();
}

void Controller::onToggleHold()
{
    m_settings.holdForMore = !m_settings.holdForMore;
    writePrefs(m_settings.toJson(m_prefs));
    applySettings();
}

void Controller::onLanguageFlick(int step)
{
    const QList<Language> &langs = m_settings.languages;
    if (langs.size() < 2 || !m_bridge.useLanguage) return;
    const int at = std::max(0, int(langs.indexOf(m_settings.language)));
    const int n = int(langs.size());
    m_bridge.useLanguage(langs[((at + step) % n + n) % n].id);
}

void Controller::onUseLanguage(const QString &id)
{
    if (m_bridge.useLanguage && Language::byId(id).isValid()) m_bridge.useLanguage(id);
}

void Controller::onOpenSettings()
{
    if (m_bridge.hide) m_bridge.hide();
    if (m_bridge.luna)
        m_bridge.luna(QStringLiteral("palm://com.webos.service.applicationManager/launch"),
                      QStringLiteral("{\"id\":\"org.webosports.app.settings.textassist\"}"));
}

void Controller::vibrate(int ms)
{
    if (!m_settings.vibrate || !m_bridge.luna) return;
    m_bridge.luna(QStringLiteral("palm://com.palm.vibrate/vibrate"),
                  QStringLiteral("{\"period\":0,\"duration\":%1}").arg(ms));
}

void Controller::hapticKey() { vibrate(m_settings.keyBuzzMs()); }

//! A long press: a little more than a key, at the same strength.
void Controller::hapticLongPress() { vibrate(m_settings.keyBuzzMs() + 8); }

// --- Emoji, paste and GIFs ------------------------------------------------------------------

QString Controller::emojiFile() const { return m_dataDir + QStringLiteral("/emoji.txt"); }

QStringList Controller::recentEmoji() const { return Settings::recentEmoji(m_prefs); }

void Controller::addRecentEmoji(const QString &e)
{
    writePrefs(Settings::withRecentEmoji(m_prefs, e));
}

void Controller::offerPaste()
{
    if (!m_view) return;
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    m_view->setPasteOffered(!t.isEmpty() && sha1(t) != m_clipDone && m_settings.showsBar() && m_wordsAllowed);
}

void Controller::onPaste()
{
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    if (m_view) m_view->setPasteOffered(false);
    if (t.isEmpty()) return;
    m_clipDone = sha1(t);
    if (m_bridge.commit) m_bridge.commit(t);
}

/*!
 * A GIF or sticker tapped on the emoji page. LuneOS's applications take no
 * pictures from a keyboard, so its link is typed in, and KLIPY is told it was shared.
 */
void Controller::onMedia(int kind, const QString &link, const QString &slug, const QString &query)
{
    if (link.isEmpty()) return;
    if (m_bridge.commit) m_bridge.commit(link);
    Klipy::Item item;
    item.slug = slug;
    Klipy::instance().shared(kind == int(Klipy::Kind::Gif) ? Klipy::Kind::Gif : Klipy::Kind::Sticker, item, query);
}

} // namespace Pre
