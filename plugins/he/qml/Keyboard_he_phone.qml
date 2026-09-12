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
 * Weights follow PhoneKeymap.cpp: letters stay one unit wide so they line up
 * with the row above, shift and backspace take 1.25 with a quarter-unit pad
 * beside them, and the bottom row is Sym 1.5 / comma 1.5 / space / period 1.5 /
 * Enter 1.5. There is no language key and no hide key on this keyboard - the
 * shift key's symbol-layer identity is cKey_ToggleLanguage, so language
 * switching lives on the 123 page.
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

        // Letters at one unit  [sum 10]
        KeyRow {
            height: keyHeight

            CharKey { label: "/"; shifted: "Q" }
            CharKey { label: "'"; shifted: "W" }
            CharKey { label: "ק"; shifted: "E" }
            CharKey { label: "ר"; shifted: "R" }
            CharKey { label: "א"; shifted: "T" }
            CharKey { label: "ט"; shifted: "Y" }
            CharKey { label: "ו"; shifted: "U" }
            CharKey { label: "ן"; shifted: "I" }
            CharKey { label: "ם"; shifted: "O" }
            CharKey { label: "פ"; shifted: "P" }
        }

        // Letters at one unit  [sum 10]
        KeyRow {
            height: keyHeight

            CharKey { label: "ש"; shifted: "A" }
            CharKey { label: "ד"; shifted: "S" }
            CharKey { label: "ג"; shifted: "D" }
            CharKey { label: "כ"; shifted: "F" }
            CharKey { label: "ע"; shifted: "G" }
            CharKey { label: "י"; shifted: "H" }
            CharKey { label: "ח"; shifted: "J" }
            CharKey { label: "ל"; shifted: "K" }
            CharKey { label: "ך"; shifted: "L" }
            CharKey { label: "ף"; shifted: ":" }
        }

        // Shift 0.75 + a quarter-unit pad, the letters at 1, then the same pad and Backspace 0.75  [sum 10]
        KeyRow {
            height: keyHeight

            ShiftKey { weight: 0.75 }
            SpacerKey { weight: 0.25; forwardTo: lowFirstKey }
            CharKey { id: lowFirstKey; label: "ז"; shifted: "Z" }
            CharKey { label: "ס"; shifted: "X" }
            CharKey { label: "ב"; shifted: "C" }
            CharKey { label: "ה"; shifted: "V" }
            CharKey { label: "נ"; shifted: "B" }
            CharKey { label: "מ"; shifted: "N" }
            CharKey { label: "צ"; shifted: "M" }
            CharKey { label: "ת"; shifted: ";" }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 0.75 }
        }

        // cCustom_*_plain  [sum 10]
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                SpaceKey       { weight: 4 }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
                ReturnKey      { weight: 1.5 }
            }
        }

        // cCustom_*_email: @ and .com flank the space key  [sum 10]
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 2 }
                UrlKey { label: ".com"; shifted: ".com" }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
                ReturnKey      { weight: 1.5 }
            }
        }

        // cCustom_*_url: a plain slash, then a colon carrying the scheme prefixes  [sum 9.5]
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                UrlKey         { label: "/"; shifted: "/" }
                UrlKey         { label: ":"; shifted: ":"; extended: ["://", "http://", "https://"] }
                SpaceKey       { weight: 2 }
                UrlKey { label: ".com"; shifted: ".com" }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
                ReturnKey      { weight: 1.5 }
            }
        }

        Loader {
            width: parent.width
            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
