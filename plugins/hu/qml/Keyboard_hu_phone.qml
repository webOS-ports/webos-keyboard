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

            CharKey { label: "q"; shifted: "Q" }
            CharKey { label: "w"; shifted: "W" }
            CharKey { label: "e"; shifted: "E"; extended: ["é","ę","ě","€"]; extendedShifted: ["É","Ę","Ě","€"] }
            CharKey { label: "r"; shifted: "R"; extended: ["ŕ","ř"]; extendedShifted: ["Ŕ","Ř"] }
            CharKey { label: "t"; shifted: "T"; extended: ["ţ","ť"]; extendedShifted: ["Ţ","Ť"] }
            CharKey { label: "z"; shifted: "Z"; extended: ["ź","ż", "ž"]; extendedShifted: ["Ź","Ż","Ž"] }
            CharKey { label: "u"; shifted: "U"; extended: ["ú","ü","ű","ů"]; extendedShifted: ["Ú","Ü","Ű","Ů"] }
            CharKey { label: "i"; shifted: "I"; extended: ["í","î"]; extendedShifted: ["Í","Î"] }
            CharKey { label: "o"; shifted: "O"; extended: ["ó","ö","ő","ô"]; extendedShifted: ["Ó","Ö","Ő","Ô"] }
            CharKey { label: "p"; shifted: "P" }
            CharKey { label: "ő"; shifted: "Ő"; extended: ["ő", "ö", "ó", "ô"]; extendedShifted: ["Ő", "Ö", "Ó", "Ô"] }
            CharKey { label: "ú"; shifted: "Ú"; extended: ["ú", "ü", "ű", "ů"]; extendedShifted: ["Ú", "Ü", "Ű", "Ů"] }
        }

        // Letters at one unit  [sum 12]
        KeyRow {
            height: keyHeight

            CharKey { label: "a"; shifted: "A"; extended: ["á","ä","â","ă","ą"]; extendedShifted: ["Á","Ä","Â","Ă","Ą"] }
            CharKey { label: "s"; shifted: "S"; extended: ["ß","ś","ş","š","$"]; extendedShifted: ["ß","Ś","Ş","Š","$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["đ","ď"]; extendedShifted: ["Đ","Ď"] }
            CharKey { label: "f"; shifted: "F" }
            CharKey { label: "g"; shifted: "G" }
            CharKey { label: "h"; shifted: "H" }
            CharKey { label: "j"; shifted: "J" }
            CharKey { label: "k"; shifted: "K" }
            CharKey { label: "l"; shifted: "L" }
            CharKey { label: "é"; shifted: "É"; extended: ["é", "è", "ê", "ë"]; extendedShifted: ["É", "È", "Ê", "Ë"] }
            CharKey { label: "á"; shifted: "Á"; extended: ["á", "à", "â", "ä"]; extendedShifted: ["Á", "À", "Â", "Ä"] }
            CharKey { label: "ű"; shifted: "Ű"; extended: ["ű", "ü", "ú", "ù"]; extendedShifted: ["Ű", "Ü", "Ú", "Ù"] }
        }

        // Shift 1.75, quarter-unit pads, letters at one unit, Backspace 1.75  [sum 12]
        KeyRow {
            height: keyHeight

            ShiftKey { weight: 1.75 }
            SpacerKey { weight: 0.25; forwardTo: lowFirstKey }
            CharKey { id: lowFirstKey; label: "í"; shifted: "Í"; extended: ["í", "ì", "î", "ï"]; extendedShifted: ["Í", "Ì", "Î", "Ï"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["¥"]; extendedShifted: ["¥"] }
            CharKey { label: "x"; shifted: "X" }
            CharKey { label: "c"; shifted: "C"; extended: ["ć","č","ç"]; extendedShifted: ["Ć","Č","Ç"] }
            CharKey { label: "v"; shifted: "V" }
            CharKey { label: "b"; shifted: "B" }
            CharKey { label: "n"; shifted: "N"; extended: ["ń","ň","ñ"]; extendedShifted: ["Ń","Ň","Ñ"] }
            CharKey { label: "m"; shifted: "M" }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.75 }
        }

        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey   { label: ","; shifted: "/"; weight: 1.5; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                SpaceKey       { weight: 6 }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
                ReturnKey      { weight: 1.5 }
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
                UrlKey         { label: ".com"; shifted: ".com" }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
                ReturnKey      { weight: 1.5 }
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
                UrlKey         { label: ".com"; shifted: ".com" }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "\u2022", "\u2026", "\u00bf"]; extendedShifted: [".", "?", "\u2022", "\u2026", "\u00bf"] }
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
