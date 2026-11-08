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

#include "precomposer.h"
#include "precorrector.h"
#include "prewords.h"

#include <limits>

namespace Pre {
namespace {

const int MAX_WORD = 32;
const QString OPENERS = QStringLiteral("(\"'[{“‘¿¡");
const QString CLOSERS = QStringLiteral(")\"']}”’");
const QString SEPARATORS = QStringLiteral(" .,!?;:)]}\"…");
//! Punctuation that goes against the word before it, never after a space.
const QString SENTENCE = QStringLiteral(".,?!:;");
const qint64 NEVER = std::numeric_limits<qint64>::min() / 2;

bool wordChar(QChar c) { return c.isLetter() || c == QLatin1Char('\''); }

QString trimmedEnd(const QString &s)
{
    int n = s.length();
    while (n > 0 && s[n - 1].isSpace()) --n;
    return s.left(n);
}

QString trimApostrophes(const QString &s)
{
    int a = 0, b = s.length();
    while (a < b && s[a] == QLatin1Char('\'')) ++a;
    while (b > a && s[b - 1] == QLatin1Char('\'')) --b;
    return s.mid(a, b - a);
}

} // namespace

Composer::Composer(Field *field, std::function<qint64()> clock)
    : onLearned([](const QString &) {}),
      onCorrected([](const QString &, const QString &, int) {}),
      m_field(field), m_clock(clock), m_lastSpaceAt(NEVER)
{
}

bool Composer::isSeparator(const QString &text)
{
    return text.length() == 1 && SEPARATORS.contains(text[0]);
}

void Composer::markSpace()
{
    const QString b = m_field->before(2);
    m_spaceTail = (b.length() == 2 && b[1] == QLatin1Char(' ') && (b[0].isLetterOrNumber() || CLOSERS.contains(b[0])))
            ? b : QString();
}

bool Composer::spaceTyped()
{
    if (m_spaceTail.isEmpty()) return false;
    return m_field->before(m_spaceTail.length()) == m_spaceTail;
}

bool Composer::autoSpaced()
{
    if (m_autoTail.isEmpty()) return false;
    return m_field->before(m_autoTail.length()) == m_autoTail;
}

void Composer::reset()
{
    m_last = Correction();
    m_swiped.clear();
    m_autoTail.clear();
    m_spaceTail.clear();
    m_lastSpaceAt = NEVER;
}

void Composer::type(const QString &text, const Rules &rules)
{
    const bool spaceAgain = text == QLatin1String(" ") && m_clock() - m_lastSpaceAt < DOUBLE_SPACE_MS;
    const bool spaced = autoSpaced();
    const QString tail = spaceTyped() ? m_spaceTail : QString();
    const bool typedSpace = !tail.isEmpty();
    m_autoTail.clear();
    m_spaceTail.clear();
    m_swiped.clear();
    // A space on top of the keyboard's own: already there. It still counts toward a double space.
    if (text == QLatin1String(" ") && spaced) {
        m_last = Correction();
        m_lastSpaceAt = m_clock();
        markSpace();
        return;
    }
    m_last = Correction();
    if (text == QLatin1String(" ") && rules.doubleSpacePeriod && spaceAgain && endsWithWordAndSpace()) {
        m_field->remove(1);
        m_field->commit(QStringLiteral(". "));
        m_autoTail = QStringLiteral(". ");
        m_lastSpaceAt = NEVER;
        return;
    }
    // A period after a number and a space starts something (3 .5, 3.14 .net); after a word, it ends it.
    const bool afterNumber = typedSpace && text == QLatin1String(".") && tail[0].isDigit();
    const bool snap = (spaced || (typedSpace && !afterNumber)) && text.length() == 1 && SENTENCE.contains(text[0]);
    Correction fixed;
    int after = text.length();
    // Sentence punctuation goes against the word, not after the space.
    if (snap) m_field->remove(1);
    if (isSeparator(text) && rules.autocorrect) {
        const Correction fix = correctWord(text);
        if (fix.valid) {
            m_field->remove(fix.typed.length());
            m_field->commit(fix.fixed + text);
            m_last = fix;
            fixed = fix;
        } else {
            m_field->commit(text);
        }
    } else {
        m_field->commit(text);
    }
    if (text == QLatin1String(".") && rules.prose && periodWantsSpace()) {
        m_field->commit(QStringLiteral(" "));
        m_autoTail = QStringLiteral(". ");
        after++;
    } else if (snap && typedSpace) {
        // The owner's space moves to after the punctuation, where it belongs.
        m_field->commit(QStringLiteral(" "));
        m_autoTail = text + QLatin1Char(' ');
        after++;
    }
    if (fixed.valid) onCorrected(fixed.typed, fixed.fixed, after);
    if (text == QLatin1String(" ")) markSpace();
    m_lastSpaceAt = text == QLatin1String(" ") ? m_clock() : NEVER;
}

bool Composer::periodWantsSpace()
{
    const QString b = m_field->before(MAX_WORD + 2);
    if (b.length() < 2) return false;
    const QChar prev = b[b.length() - 2];
    if (prev.isSpace() || prev.isDigit()) return false;
    QString token = b.left(b.length() - 1);
    token = token.mid(token.lastIndexOf(QLatin1Char(' ')) + 1);
    token = token.mid(token.lastIndexOf(QLatin1Char('\n')) + 1);
    if (token.contains(QLatin1Char('@')) || token.contains(QLatin1Char('/')) ||
        token.startsWith(QLatin1String("www"), Qt::CaseInsensitive))
        return false;
    const QString next = m_field->after(1);
    return next.isEmpty() || !next[0].isSpace();
}

void Composer::beforeEnter(const Rules &rules)
{
    m_last = Correction();
    m_swiped.clear();
    m_lastSpaceAt = NEVER;
    m_spaceTail.clear();
    // The keyboard's own space is not left trailing on the line.
    if (autoSpaced()) m_field->remove(1);
    m_autoTail.clear();
    if (!rules.autocorrect) return;
    const Correction fix = correctWord(QString());
    if (!fix.valid) return;
    m_field->remove(fix.typed.length());
    m_field->commit(fix.fixed);
    onCorrected(fix.typed, fix.fixed, 0);
}

bool Composer::backspace()
{
    const bool spaced = autoSpaced();
    m_autoTail.clear();
    m_spaceTail.clear();
    if (!m_swiped.isEmpty()) {
        const QString w = m_swiped;
        m_swiped.clear();
        const QString tail = w + (spaced ? QStringLiteral(" ") : QString());
        if (m_field->before(tail.length()) != tail) return false;
        m_field->remove(tail.length() + (m_swipedSpace ? 1 : 0));
        m_autoTail = m_swipedOn;
        return true;
    }
    // After a period: just its space, here, before any undo of the word's correction.
    if (spaced) return false;
    if (!m_last.valid) return false;
    const Correction fix = m_last;
    m_last = Correction();
    m_lastSpaceAt = NEVER;
    const QString done = fix.fixed + fix.separator;
    if (m_field->before(done.length()) != done) return false;
    m_field->remove(done.length());
    m_field->commit(fix.typed + fix.separator);
    if (fix.separator == QLatin1String(" ")) markSpace();
    return true;
}

void Composer::forget()
{
    m_last = Correction();
    m_swiped.clear();
    m_autoTail.clear();
    m_spaceTail.clear();
}

bool Composer::swipedIsLast()
{
    if (m_swiped.isEmpty()) return false;
    const QString tail = m_swiped + (autoSpaced() ? QStringLiteral(" ") : QString());
    if (m_field->before(tail.length()) != tail) return false;
    const QString after = m_field->after(1);
    return after.isEmpty() || !after[0].isLetterOrNumber();
}

void Composer::swipe(const QString &word, const Rules &rules)
{
    m_last = Correction();
    m_lastSpaceAt = NEVER;
    m_swipedOn = autoSpaced() ? m_autoTail : QString();
    m_autoTail.clear();
    m_spaceTail.clear();
    const QString b = m_field->before(1);
    m_swipedSpace = !b.isEmpty() && !b[0].isSpace() && !OPENERS.contains(b[0]);
    Correction fixed;
    if (m_swipedSpace && rules.autocorrect) {
        const Correction fix = correctWord(QStringLiteral(" "));
        if (fix.valid) {
            m_field->remove(fix.typed.length());
            m_field->commit(fix.fixed);
            fixed = fix;
        }
    }
    m_field->commit((m_swipedSpace ? QStringLiteral(" ") : QString()) + word + QLatin1Char(' '));
    if (fixed.valid) onCorrected(fixed.typed, fixed.fixed, word.length() + 2);
    m_swiped = word;
    m_autoTail = word + QLatin1Char(' ');
}

void Composer::pick(const QString &word, bool learn)
{
    m_last = Correction();
    const QString sw = m_swiped;
    if (!sw.isEmpty() && swipedIsLast()) {
        const bool spaced = autoSpaced();
        m_field->remove(sw.length() + (spaced ? 1 : 0));
        m_field->commit(word + QLatin1Char(' '));
        m_swiped = word;
        m_autoTail = word + QLatin1Char(' ');
        return;
    }
    m_swiped.clear();
    m_autoTail.clear();
    const QString typed = currentWord();
    m_field->remove(typed.length());
    m_field->commit(word + QLatin1Char(' '));
    markSpace();
    // A space after a picked word counts as the first of a double-tapped space.
    m_lastSpaceAt = m_clock();
    if (learn && corrector) {
        corrector->learn(word);
        onLearned(word);
    }
}

QString Composer::currentWord()
{
    const QString after = m_field->after(1);
    if (!after.isEmpty() && (after[0].isLetterOrNumber() || after[0] == QLatin1Char('\''))) return QString();
    const QString before = m_field->before(MAX_WORD + 1);
    int i = before.length();
    while (i > 0 && wordChar(before[i - 1])) i--;
    if (i > 0 && !before[i - 1].isSpace() && !OPENERS.contains(before[i - 1])) return QString();
    if (before.length() - i > MAX_WORD) return QString();
    return before.mid(i);
}

QString Composer::nextContext()
{
    const QString after = m_field->after(1);
    if (!after.isEmpty() && !after[0].isSpace()) return QString();
    const QString before = m_field->before(MAX_WORD + 2);
    if (!before.isEmpty() && !before[before.length() - 1].isSpace()) return QString();
    return contextOf(before);
}

QPair<QString, QString> Composer::finishedPair()
{
    const QString before = m_field->before(2 * MAX_WORD + 4);
    int n = before.length();
    while (n > 0 && (before[n - 1].isSpace() || SENTENCE.contains(before[n - 1]))) n--;
    const QString end = before.left(n);
    if (end.length() == before.length()) return {};
    int i = end.length();
    while (i > 0 && wordChar(end[i - 1])) i--;
    const QString word = trimApostrophes(end.mid(i));
    if (word.isEmpty() || (i > 0 && !end[i - 1].isSpace() && !OPENERS.contains(end[i - 1]))) return {};
    // Only a word that ended the whole text before it is known to have run to its start.
    if (i == 0 && before.length() >= 2 * MAX_WORD + 4) return {};
    const QString ctx = contextOf(end.left(i));
    if (ctx.isEmpty()) return {};
    return { ctx, word };
}

QString Composer::contextOf(const QString &before)
{
    const QString t = trimmedEnd(before);
    if (t.isEmpty()) return before.length() < MAX_WORD ? NextWords::START : QString();
    if (before.length() > t.length() && before.mid(t.length()).contains(QLatin1Char('\n'))) return NextWords::START;
    const QChar last = t[t.length() - 1];
    if (QStringLiteral(".!?").contains(last)) return NextWords::START;
    if (last == QLatin1Char(',')) return NextWords::COMMA;
    if (!last.isLetter() && last != QLatin1Char('\'')) return QString();
    int i = t.length();
    while (i > 0 && wordChar(t[i - 1])) i--;
    if (i > 0 && !t[i - 1].isSpace() && !OPENERS.contains(t[i - 1])) return QString();
    if (i == 0 && t.length() > MAX_WORD) return QString();
    const QString word = trimApostrophes(t.mid(i));
    // "l'homme" follows as "homme" does: the elision is no word of its own.
    return (corrector ? corrector->lang().bare(word) : word).toLower();
}

Composer::Correction Composer::correctWord(const QString &separator)
{
    if (!corrector) return Correction();
    const QString before = m_field->before(2 * MAX_WORD + 4);
    int i = before.length();
    while (i > 0 && wordChar(before[i - 1])) i--;
    // A word glued to something else (a URL, an address, a number) is not ours to touch.
    if (i > 0 && !before[i - 1].isSpace() && !OPENERS.contains(before[i - 1])) return Correction();
    QString word = before.mid(i);
    while (word.endsWith(QLatin1Char('\''))) word.chop(1);
    if (word.isEmpty() || word.length() != before.length() - i || word.length() > MAX_WORD) return Correction();
    const QString fixed = corrector->correct(word, contextOf(before.left(i)));
    if (fixed.isEmpty()) return Correction();
    Correction c;
    c.typed = word;
    c.fixed = fixed;
    c.separator = separator;
    c.valid = true;
    return c;
}

QString Composer::wordContext()
{
    const QString before = m_field->before(2 * MAX_WORD + 4);
    int i = before.length();
    while (i > 0 && wordChar(before[i - 1])) i--;
    if (i == 0 && before.length() >= 2 * MAX_WORD + 4) return QString();
    return contextOf(before.left(i));
}

bool Composer::endsWithWordAndSpace()
{
    const QString b = m_field->before(2);
    return b.length() == 2 && b[1] == QLatin1Char(' ') && (b[0].isLetterOrNumber() || CLOSERS.contains(b[0]));
}

} // namespace Pre
