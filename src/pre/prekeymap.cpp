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

#include "prekeymap.h"

#include <QHash>
#include <QMutex>

#include <algorithm>
#include <cmath>

#include <map>

namespace Pre {
namespace {

//! The bottom row's two-column keys at each end: drawn this wide, after an inset of EDGE that still takes a touch.
const float WIDE = 1.65f;
const float EDGE = 0.35f;

KeyPtr makeKey(Kind kind, const QString &text = QString(), const QString &alt = QString(),
               AltPos pos = AltPos::Right, const QStringList &extended = {}, bool accent = false,
               const QString &label = QString(), Corner corner = Corner::None)
{
    QSharedPointer<Key> k(new Key);
    k->kind = kind;
    k->text = text;
    k->alt = alt;
    k->altPos = pos;
    k->extended = extended;
    k->accent = accent;
    k->label = label;
    k->corner = corner;
    return k;
}

Slot slot(float weight, const KeyPtr &key, bool drawn = true)
{
    Slot s;
    s.weight = weight;
    s.key = key;
    s.drawn = drawn;
    return s;
}

QStringList words(const QString &s)
{
    return s.split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

Slot c(const QString &text, const QString &alt, AltPos pos, const QString &ext)
{
    return slot(1.0f, makeKey(Kind::Char, text, alt, pos, words(ext)));
}

Slot f(Kind kind, float w = 1.0f, const QString &label = QString())
{
    return slot(w, makeKey(kind, QString(), QString(), AltPos::Right, {}, false, label));
}

/*!
 * What holding each letter offers after the letter itself (LuneOS's lists). In
 * a language with accents of its own, those come first (Language::accents).
 */
const QHash<QChar, QString> &holds()
{
    static const QHash<QChar, QString> h = {
        { QLatin1Char('e'), QStringLiteral("è é ê ë ę ē € ě") },
        { QLatin1Char('r'), QStringLiteral("® ř ŕ") },
        { QLatin1Char('t'), QStringLiteral("™ þ ť ţ") },
        { QLatin1Char('y'), QStringLiteral("ý ÿ ¥") },
        { QLatin1Char('u'), QStringLiteral("ù ú û ü ű") },
        { QLatin1Char('i'), QStringLiteral("ì í î ï İ ı") },
        { QLatin1Char('o'), QStringLiteral("ò ó ô õ ö ø ő œ º ω") },
        { QLatin1Char('p'), QStringLiteral("¶ § π") },
        { QLatin1Char('a'), QStringLiteral("à á â ã ä å æ ª") },
        { QLatin1Char('s'), QStringLiteral("š ş ß σ $ ś — –") },
        { QLatin1Char('d'), QStringLiteral("ð † ‡ ď đ") },
        { QLatin1Char('g'), QStringLiteral("ğ") },
        { QLatin1Char('l'), QStringLiteral("ł ĺ") },
        { QLatin1Char('z'), QStringLiteral("ž ź ż") },
        { QLatin1Char('c'), QStringLiteral("ç ć © ¢ č") },
        { QLatin1Char('n'), QStringLiteral("ñ ń ň") },
        { QLatin1Char('m'), QStringLiteral("µ") },
    };
    return h;
}

/*!
 * The printed alternates by place, not by letter: they are the Pre's number and
 * symbol grid, and stay where they are when a language moves the letters (Y and
 * Z in German). The left half of the top three rows prints them top left.
 */
const QList<QStringList> &alts()
{
    static const QList<QStringList> a = {
        { QStringLiteral("/"), QStringLiteral("+"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"),
          QStringLiteral("("), QStringLiteral(")"), QStringLiteral("%"), QStringLiteral("\""), QStringLiteral("=") },
        { QStringLiteral("&"), QStringLiteral("-"), QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"),
          QStringLiteral("$"), QStringLiteral("!"), QStringLiteral(":"), QStringLiteral("'") },
        { QStringLiteral("*"), QStringLiteral("7"), QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("#"),
          QStringLiteral("?"), QStringLiteral(";") },
    };
    return a;
}

//! Whether the Pre prints alternate [col] of [row] top left: the left half of the top three rows.
bool printedLeft(int row, int col) { return row == 2 ? col < 4 : col < 5; }

//! [letter]'s key in row [row], place [col] (on the bottom letter row, counted after the orange key).
Slot letter(QChar letter, int row, int col, const Language &lang)
{
    const QString alt = alts()[row][col];
    const bool leftHalf = printedLeft(row, col);
    QStringList own;
    for (QChar a : lang.accentsOf(letter))
        own << QString(a);
    QStringList rest;
    for (const QString &h : words(holds().value(letter)))
        if (!own.contains(h))
            rest << h;
    QStringList ext;
    if (!own.isEmpty() || !rest.isEmpty())
        ext = QStringList(QString(letter)) + own + rest;
    return slot(1.0f, makeKey(Kind::Char, QString(letter), alt, leftHalf ? AltPos::Left : AltPos::Right, ext));
}

/*!
 * The room a language's own letter row [row] of [n] keys has on the Pre's
 * grid: ten keys across the top, nine beside backspace, eight (the comma's
 * included) between the orange key and Return. A row with more letters than
 * the Pre's shares that room among them, so backspace, Return, the orange key
 * and the bottom row keep their size and place in every language, and stay
 * where the Sym page has its own backspace and Return.
 */
int rowRoom(int row) { return row == 0 ? 10 : row == 1 ? 9 : 8; }
//! The keys sharing [row]'s room: its [n] letters, and on the bottom row the comma.
int rowKeys(int row, int n) { return row == 2 ? n + 1 : n; }
//! A letter's width in [row] of [n]: one key, or less where there are more of them than the Pre's.
float letterWeight(int row, int n) { return float(rowRoom(row)) / float(std::max(rowRoom(row), rowKeys(row, n))); }

/*!
 * For a language's own letter row [row] of [n] keys: which of the Pre's
 * alternates for the row each key prints (an index into alts()[row]; -1 for
 * none). Each goes on the key over the middle of its column on the Pre's grid,
 * as the keys are laid out (see rowRoom), so the digits stand where they do in
 * every other language. A row with more keys than the Pre's has a few
 * without; none has fewer.
 */
QVector<int> altColumns(int row, int n)
{
    const int pre = int(alts()[row].size());
    const float w = letterWeight(row, n);
    QVector<int> out(n, -1);
    for (int col = 0; col < pre; ++col) {
        // The column's middle, from where the row's letters start, in this row's keys.
        const int at = int(std::floor((col + 0.5f) / w));
        if (at >= 0 && at < n && out[at] < 0) out[at] = col;
    }
    return out;
}

//! A letter of a language with its own rows, place [index] of [rowLength]: its phone layout's key, with
//! the Pre's alternate [col] of [row] (-1: none).
Slot ownLetter(const LetterKey &lk, int row, int col, int index, int rowLength)
{
    const QStringList &a = alts()[row];
    const QString alt = col >= 0 && col < a.size() ? a[col] : QString();
    QSharedPointer<Key> k(new Key);
    k->kind = Kind::Char;
    k->text = lk.text;
    k->shifted = lk.shifted.isEmpty() || lk.shifted == lk.text.toUpper() ? QString() : lk.shifted;
    k->alt = alt;
    // On the side the Pre prints it; a key without one by its half of the row.
    k->altPos = (col >= 0 ? printedLeft(row, col) : index < (rowLength + 1) / 2) ? AltPos::Left : AltPos::Right;
    k->extended = lk.extended.size() > 1 ? lk.extended : QStringList();
    if (!k->extended.isEmpty()) {
        QStringList upper;
        for (const QString &e : k->extended) upper << e.toUpper();
        if (lk.extendedShifted.size() == k->extended.size() && lk.extendedShifted != upper)
            k->extendedShifted = lk.extendedShifted;
    }
    return slot(letterWeight(row, rowLength), k);
}

Rows lettersOf(const Language &lang)
{
    static const KeyPtr shift = makeKey(Kind::Shift);
    static const KeyPtr sym = makeKey(Kind::Sym, QString(), QString(), AltPos::Right, {}, false, QStringLiteral("Sym"));
    QList<Slot> top, middle, bottom, last;
    // The comma, as wide as the bottom row's letters.
    float commaWeight = 1.0f;
    if (lang.layout == Layout::Own && lang.letters.size() == 3) {
        const LetterRows &r = lang.letters;
        QList<Slot> *const out[3] = { &top, &middle, &bottom };
        for (int row = 0; row < 3; ++row) {
            const int n = int(r[row].size());
            const QVector<int> cols = altColumns(row, n);
            if (row == 2) bottom << f(Kind::Orange);
            for (int i = 0; i < n; ++i) *out[row] << ownLetter(r[row][i], row, cols[i], i, n);
            if (row == 1) middle << f(Kind::Backspace);
        }
        commaWeight = letterWeight(2, int(r[2].size()));
    } else {
        const QStringList rows = layoutRows(lang.layout == Layout::Own ? Layout::Qwerty : lang.layout);
        for (int i = 0; i < rows[0].length(); ++i) top << letter(rows[0][i], 0, i, lang);
        for (int i = 0; i < rows[1].length(); ++i) middle << letter(rows[1][i], 1, i, lang);
        middle << f(Kind::Backspace);
        bottom << f(Kind::Orange);
        for (int i = 0; i < rows[2].length(); ++i) bottom << letter(rows[2][i], 2, i, lang);
    }
    // The comma holds what the Pre's period did.
    Slot comma = c(QStringLiteral(","), QStringLiteral("_"), AltPos::Right, QStringLiteral(", … • ¿"));
    comma.weight = commaWeight;
    bottom << comma;
    bottom << f(Kind::Return);
    // The Pre's bottom row, inset at each end. On the column grid above: shift
    // and Sym take two columns each (the same size, inset alike), @ is one key
    // in the column of E, D and X with its 0 where theirs are, the space bar 3
    // to 7, and the period the column of M below I and K. The Sym page's bottom
    // row is the same, key for key, so nothing moves when the page changes.
    last << slot(EDGE, shift, false)
         << slot(WIDE, shift)
         << c(QStringLiteral("@"), QStringLiteral("0"), AltPos::Left, QString())
         << slot(4.0f, makeKey(Kind::Space, QStringLiteral(" ")))
         << slot(1.0f, makeKey(Kind::Char, QStringLiteral("."), QString(), AltPos::Right, {}, false,
                               QString(), Corner::Mic))
         << slot(WIDE, sym)
         << slot(EDGE, sym, false);
    return { top, middle, bottom, last };
}

//! A digit in the accent colour; held, its superscript and fractions.
Slot digit(QChar d, const QString &ext)
{
    return slot(1.0f, makeKey(Kind::Char, QString(d), QString(), AltPos::Right,
                              words(QString(d) + QLatin1Char(' ') + ext), true));
}

//! The digits, in the accent colour: the Sym page's top row, and the number row above the letters.
QList<Slot> digitRow()
{
    return { digit(QLatin1Char('1'), QStringLiteral("¹ ½ ⅓ ¼ ⅛")), digit(QLatin1Char('2'), QStringLiteral("² ⅔")),
             digit(QLatin1Char('3'), QStringLiteral("³ ¾ ⅜")), digit(QLatin1Char('4'), QStringLiteral("⁴")),
             digit(QLatin1Char('5'), QStringLiteral("⁵ ⅝")), digit(QLatin1Char('6'), QStringLiteral("⁶")),
             digit(QLatin1Char('7'), QStringLiteral("⁷ ⅞")), digit(QLatin1Char('8'), QStringLiteral("⁸")),
             digit(QLatin1Char('9'), QStringLiteral("⁹")), digit(QLatin1Char('0'), QStringLiteral("⁰ ⁿ ∅")) };
}

/*!
 * [page] with the digits printed on its letters (E R T, D F G, X C V and the
 * @'s 0) left off, for the owner who has them in the number row and wants
 * the keys plainer. The other alternates stay.
 */
Rows withoutDigits(const Rows &page)
{
    Rows out = page;
    for (QList<Slot> &row : out)
        for (Slot &s : row) {
            const QString &a = s.key->alt;
            if (a.size() != 1 || !a.at(0).isDigit()) continue;
            QSharedPointer<Key> k(new Key(*s.key));
            k->hideAlt = true;
            s.key = k;
        }
    return out;
}

//! A symbol; [ext] is its press-and-hold list after itself.
Slot s(const QString &t, const QString &ext = QString())
{
    return slot(1.0f, makeKey(Kind::Char, t, QString(), AltPos::Right,
                              ext.isEmpty() ? QStringList() : words(t + QLatin1Char(' ') + ext)));
}

/*
 * The Sym page: digits in the accent colour, then the symbols the letters page
 * does not type. Every key that is on both pages is in the same place on both,
 * so switching never moves one from under the thumb: backspace at the end of
 * the second row, Return at the end of the third (held, Text Assist), the
 * space bar and the period where they are, and the key that changes page (Sym,
 * ABC) at the bottom right, so the same spot goes there and back.
 */
Rows symbolsOf()
{
    const KeyPtr abc = makeKey(Kind::Sym, QString(), QString(), AltPos::Right, {}, false, QStringLiteral("ABC"));
    // The globe, where shift is on the letters: the languages, held the style.
    const KeyPtr globe = makeKey(Kind::Globe);
    return {
        digitRow(),
        { s(QStringLiteral("[")), s(QStringLiteral("]")), s(QStringLiteral("{")), s(QStringLiteral("}")),
          s(QStringLiteral("<"), QStringLiteral("‹ « ≤")), s(QStringLiteral(">"), QStringLiteral("› » ≥")),
          s(QStringLiteral("^")), s(QStringLiteral("~"), QStringLiteral("≈")),
          s(QStringLiteral("|"), QStringLiteral("¦ ‖ ¬")),
          // Backspace where it is on the letters page, at the end of A to L's row.
          f(Kind::Backspace) },
        { s(QStringLiteral("\\")),
          s(QStringLiteral("`"), QStringLiteral("´ ‘ ’ “ ” ‚ „")),
          s(QStringLiteral("°"), QStringLiteral("′ ″ ‰")),
          s(QStringLiteral("£"), QStringLiteral("₹ ₩ ₽ ₺ ₪ ₱ ₿ ¤ ƒ")),
          s(QStringLiteral("×"), QStringLiteral("· ∗")),
          s(QStringLiteral("÷"), QStringLiteral("√ ∞ ∆ ∑")),
          s(QStringLiteral("±"), QStringLiteral("∓")),
          s(QStringLiteral("≠"), QStringLiteral("≡")),
          s(QStringLiteral("¡"), QStringLiteral("‽")),
          // Return where it is on the letters page, under backspace. Hold it for
          // Text Assist, where the Pre Keyboard is set up.
          slot(1.0f, makeKey(Kind::Return, QString(), QString(), AltPos::Right, {}, false, QString(), Corner::Gear)) },
        // Key for key the letters page's bottom row (the globe where shift is,
        // emoji where @ is, the period where the period is, ABC where Sym is),
        // so nothing on it moves when the page changes.
        { slot(EDGE, globe, false),
          slot(WIDE, globe),
          // Tap for the emoji page; hold for LuneOS's text emoticons.
          slot(1.0f, makeKey(Kind::Emoji, QString(), QString(), AltPos::Right,
                             words(QStringLiteral(":-) ;-) :-( :'( :-P :-O <3")))),
          slot(4.0f, makeKey(Kind::Space, QStringLiteral(" "))),
          slot(1.0f, makeKey(Kind::Char, QStringLiteral("."), QString(), AltPos::Right, {}, false,
                             QString(), Corner::Mic)),
          slot(WIDE, abc),
          slot(EDGE, abc, false) },
    };
}

/*!
 * The row that takes the digits' place on the Sym page when the letters have
 * a number row: arrows, ticks and a few signs, none of them anywhere else on
 * either page.
 */
QList<Slot> moreSymbols()
{
    return { s(QStringLiteral("←"), QStringLiteral("⇐ ↖ ↙")), s(QStringLiteral("→"), QStringLiteral("⇒ ↗ ↘")),
             s(QStringLiteral("↑"), QStringLiteral("⇑")), s(QStringLiteral("↓"), QStringLiteral("⇓")),
             s(QStringLiteral("↔"), QStringLiteral("⇔ ↕")), s(QStringLiteral("✓"), QStringLiteral("✔ ☑")),
             s(QStringLiteral("✗"), QStringLiteral("✘ ☐")), s(QStringLiteral("★"), QStringLiteral("☆")),
             s(QStringLiteral("♥"), QStringLiteral("♡ ♦ ♣ ♠")), s(QStringLiteral("♪"), QStringLiteral("♫")) };
}

} // namespace

const Rows &Keymap::letters(const Language &lang, bool numberRow, bool hideDigits)
{
    // std::map: a reference handed out stays good however many languages come after.
    static QMutex lock;
    static std::map<QString, Rows> built;
    QMutexLocker locker(&lock);
    auto it = built.find(lang.id);
    if (it == built.end())
        it = built.emplace(lang.id, lettersOf(lang.isValid() ? lang : Language::orEnglish(lang.id))).first;
    if (!numberRow) return it->second;
    // The same keys under the number row; with the digits hidden, new ones only where a digit was.
    const QString page = lang.id + (hideDigits ? QStringLiteral("/numbers/plain") : QStringLiteral("/numbers"));
    auto numbered = built.find(page);
    if (numbered == built.end()) {
        Rows rows = hideDigits ? withoutDigits(it->second) : it->second;
        rows.prepend(digitRow());
        numbered = built.emplace(page, rows).first;
    }
    return numbered->second;
}

const Rows &Keymap::symbols(bool numberRow)
{
    static const Rows plain = symbolsOf();
    static const Rows numbered = [] {
        Rows rows = symbolsOf();
        rows.insert(1, moreSymbols());
        return rows;
    }();
    return numberRow ? numbered : plain;
}

} // namespace Pre
