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

#ifndef PRE_KEYMAP_H
#define PRE_KEYMAP_H

#include "prelanguage.h"

#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace Pre {

/*
 * The Palm Pre's keyboard, key for key: four rows of ten in matching columns,
 * the orange key and Sym, and each key's alternate character where the Pre
 * prints it - the layout of webos-keyboard's "Pre (Orange)" and "Pre (White)",
 * which were measured off a Pre.
 *
 * The real Pre's alternates:
 *   Q/ W+ E1 R2 T3 (printed top left)   Y( U) I% O" P= (top right)
 *   A& S- D4 F5 G6 (top left)           H$ J! K: L' (top right)
 *   Z* X7 C8 V9 (top left)              B# N? M; ,_ (top right)
 *   @0 (here the 0 is printed top left, as 1, 4 and 7 are above it).
 */

enum class Kind { Char, Shift, Backspace, Return, Orange, Sym, Space, Globe, Emoji };

//! A small icon printed in a key's top right corner, for what holding the key does.
//! The period's microphone shows only where dictation could start.
enum class Corner { None, Gear, Mic };

//! Which top corner a key's printed alternate sits in; the letter takes the bottom corner opposite.
enum class AltPos { Left, Right };

struct Key
{
    Kind kind = Kind::Char;
    //! What the key types, and what it shows (letters show as capitals, as printed on the Pre).
    QString text;
    //! What shift types, where that is not [text]'s capital (Arabic, Hebrew, Czech); empty otherwise.
    QString shifted;
    //! The printed alternate, typed while the orange key is active; empty when none.
    QString alt;
    AltPos altPos = AltPos::Right;
    //! The press-and-hold list; its first entry is always the key's own character.
    QStringList extended;
    //! The press-and-hold list with shift, where it is not [extended]'s capitals; empty otherwise.
    QStringList extendedShifted;
    //! Printed in the accent colour: the Sym page's digits.
    bool accent = false;
    //! A label for a function key ("Sym", "ABC").
    QString label;
    //! Printed in the top right: Return's gear on the Sym page, the period's microphone.
    Corner corner = Corner::None;
};

typedef QSharedPointer<const Key> KeyPtr;

/*!
 * One place in a row. A slot with [drawn] false is a spacer: it draws nothing,
 * but a touch on it goes to [key], as webos-keyboard's SpacerKey { forwardTo }
 * did. That is how the Pre's bottom row is inset by about a key at each end and
 * still easy to hit.
 */
struct Slot
{
    float weight = 1.0f;
    KeyPtr key;
    bool drawn = true;
};

typedef QList<QList<Slot>> Rows;

namespace Keymap {
//! The letters page for [lang]: its layout, and its accents first in the holds.
const Rows &letters(const Language &lang);
//! The Sym page.
const Rows &symbols();
}

} // namespace Pre

#endif // PRE_KEYMAP_H
