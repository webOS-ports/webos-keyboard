/*
 * Copyright 2013 Canonical Ltd.
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
 * Copyright (C) 2015 Herman van Hazendonk <github.com@herrie.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Weights follow PhoneKeymap.cpp: letters at one unit so they line up with the
 * row above, shift and backspace taking the slack with a quarter-unit pad beside
 * them, and the bottom row Sym 1.5 / comma 1.5 / space / period 1.5 / Enter 1.5.
 * No language key and no hide key - the shift key's symbol-layer identity is
 * cKey_ToggleLanguage, so language switching lives on the 123 page.
 */

import QtQuick 2.0
import keys 1.0

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_phone.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        // Letters at one unit  [sum 12]
        KeyRow {
            height: keyHeight

            CharKey { label: "й"; shifted: "Й" }
            CharKey { label: "ц"; shifted: "Ц" }
            CharKey { label: "у"; shifted: "У" }
            CharKey { label: "к"; shifted: "К" }
            CharKey { label: "е"; shifted: "Е" }
            CharKey { label: "н"; shifted: "Н" }
            CharKey { label: "г"; shifted: "Г"; extended: ["г", "ґ"]; extendedShifted: ["Г", "Ґ"] }
            CharKey { label: "ш"; shifted: "Ш" }
            CharKey { label: "щ"; shifted: "Щ" }
            CharKey { label: "з"; shifted: "З" }
            CharKey { label: "х"; shifted: "Х" }
            CharKey { label: "ї"; shifted: "Ї"; extended: ["ї", "і"]; extendedShifted: ["Ї", "І"] }
        }

        // Letters at one unit, inset 0.5 each end  [sum 12]
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: r2First }
            CharKey { id: r2First; label: "ф"; shifted: "Ф" }
            CharKey { label: "і"; shifted: "І"; extended: ["і", "ї", "и"]; extendedShifted: ["І", "Ї", "И"] }
            CharKey { label: "в"; shifted: "В" }
            CharKey { label: "а"; shifted: "А" }
            CharKey { label: "п"; shifted: "П" }
            CharKey { label: "р"; shifted: "Р" }
            CharKey { label: "о"; shifted: "О" }
            CharKey { label: "л"; shifted: "Л" }
            CharKey { label: "д"; shifted: "Д" }
            CharKey { label: "ж"; shifted: "Ж" }
            CharKey { id: r2Last; label: "є"; shifted: "Є"; extended: ["є", "е"]; extendedShifted: ["Є", "Е"] }
            SpacerKey { weight: 0.5; forwardTo: r2Last }
        }

        // Shift 1.25, quarter-unit pads, letters at one unit, Backspace 1.25  [sum 12]
        KeyRow {
            height: keyHeight

            ShiftKey { weight: 1.25 }
            SpacerKey { weight: 0.25; forwardTo: lowFirstKey }
            CharKey { id: lowFirstKey; label: "я"; shifted: "Я" }
            CharKey { label: "ч"; shifted: "Ч" }
            CharKey { label: "с"; shifted: "С" }
            CharKey { label: "м"; shifted: "М" }
            CharKey { label: "и"; shifted: "И" }
            CharKey { label: "т"; shifted: "Т" }
            CharKey { label: "ь"; shifted: "Ь"; extended: ["ь", "’"]; extendedShifted: ["Ь", "’"] }
            CharKey { label: "б"; shifted: "Б" }
            CharKey { label: "ю"; shifted: "Ю" }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.25 }
        }

        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey   { label: ","; shifted: "/"; weight: 1.5; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                SpaceKey       { weight: 6 }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey   { label: ","; shifted: "/"; weight: 1.5; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 4 }
                UrlKey         { label: ".ua"; shifted: ".ua" }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                UrlKey         { label: "/"; shifted: "/" }
                UrlKey         { label: ":"; shifted: ":"; extended: ["://", "http://", "https://"] }
                SpaceKey       { weight: 4 }
                UrlKey         { label: ".ua"; shifted: ".ua" }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        Loader {
            width: parent.width

            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
