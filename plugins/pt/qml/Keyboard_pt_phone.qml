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

            CharKey { label: "q"; shifted: "Q" }
            CharKey { label: "w"; shifted: "W" }
            CharKey { label: "e"; shifted: "E"; extended: ["é","ê","è","ë","€"]; extendedShifted: ["É","Ê","È","Ë","€"] }
            CharKey { label: "r"; shifted: "R" }
            CharKey { label: "t"; shifted: "T"; extended: ["þ"]; extendedShifted: ["Þ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["ý", "¥", "ÿ"]; extendedShifted: ["Ý", "¥", "Ÿ"] }
            CharKey { label: "u"; shifted: "U"; extended: ["ú","û","ù","ü"]; extendedShifted: ["Ú","Û","Ù","Ü"] }
            CharKey { label: "i"; shifted: "I"; extended: ["í","î","ì","ï"]; extendedShifted: ["Í","Î","Ì","Ï"] }
            CharKey { label: "o"; shifted: "O"; extended: ["ó","õ","ô","º","ò","ö"]; extendedShifted: ["Ó","Õ","Ô","º","Ò","Ö"] }
            CharKey { label: "p"; shifted: "P" }
        }

        // Letters at one unit  [sum 10]
        KeyRow {
            height: keyHeight

            CharKey { label: "a"; shifted: "A"; extended: ["ã","á","à","â","ª","ä","å","æ"]; extendedShifted: ["Ã","Á","À","Â","ª","Ä","Å","Æ"] }
            CharKey { label: "s"; shifted: "S"; extended: ["ß","$"]; extendedShifted: ["$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["ð"]; extendedShifted: ["Ð"] }
            CharKey { label: "f"; shifted: "F" }
            CharKey { label: "g"; shifted: "G" }
            CharKey { label: "h"; shifted: "H" }
            CharKey { label: "j"; shifted: "J" }
            CharKey { label: "k"; shifted: "K" }
            CharKey { label: "l"; shifted: "L" }
            CharKey { label: "ç"; shifted: "Ç" }
        }

        // Shift 1.25 + a quarter-unit pad, the letters at 1, then the same pad and Backspace 1.25  [sum 10]
        KeyRow {
            height: keyHeight

            ShiftKey { weight: 1.25 }
            SpacerKey { weight: 0.25; forwardTo: lowFirstKey }
            CharKey { id: lowFirstKey; label: "z"; shifted: "Z" }
            CharKey { label: "x"; shifted: "X" }
            CharKey { label: "c"; shifted: "C"; extended: ["ç"]; extendedShifted: ["Ç"] }
            CharKey { label: "v"; shifted: "V" }
            CharKey { label: "b"; shifted: "B" }
            CharKey { label: "n"; shifted: "N"; extended: ["ñ"]; extendedShifted: ["Ñ"] }
            CharKey { label: "m"; shifted: "M" }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.25 }
        }

        // cCustom_*_plain  [sum 10]
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/" }
                SpaceKey       { weight: 4 }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
                ReturnKey      { weight: 1.5 }
            }
        }

        // cCustom_*_email: @ and .com flank the space key  [sum 10]
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/" }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 2 }
                UrlKey { label: ".com"; shifted: ".com" }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
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
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
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
