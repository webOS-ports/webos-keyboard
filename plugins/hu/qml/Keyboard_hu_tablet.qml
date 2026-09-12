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
 * No keymap for Hungarian in the reference, so the row weights follow its idiom:
 * digits at one unit with the 2-unit trackball, letters at one unit with the
 * function keys taking the ends, and the bottom row as every tabletkeymaps/ file
 * has it, summing to 11.
 * 
 * The alphabet is the Hungarian one rather than English with z and y swapped:
 * o-double-acute and u-acute close the top row, e-acute, a-acute and
 * u-double-acute close the home row, and i-acute opens the bottom one.
 */

import QtQuick 2.0
import keys 1.0

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_tablet.qml"
    alternativeLayouts: [ "Thumb" ]

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        // Digits + trackball  [sum 12]
        KeyRow {
            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"] }
            NumKey { label: "2"; shifted: "@"; extended: ["2", "@", "²"] }
            NumKey { label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"] }
            NumKey { label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"] }
            NumKey { label: "5"; shifted: "%"; extended: ["5", "%", "‰"] }
            NumKey { label: "6"; shifted: "^"; extended: ["6", "^"] }
            NumKey { label: "7"; shifted: "&"; extended: ["7", "&"] }
            NumKey { label: "8"; shifted: "*"; extended: ["8", "*"] }
            NumKey { label: "9"; shifted: "("; extended: ["9", "(", "[", "{"] }
            NumKey { label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"] }
            TrackBall { }
        }

        // Top row + Backspace  [sum 13.5]
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
            BackspaceKey { weight: 1.5 }
        }

        // Half-key inset, home row, Return  [sum 14]
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: homeFirstKey }
            CharKey { id: homeFirstKey; label: "a"; shifted: "A"; extended: ["á","ä","â","ă","ą"]; extendedShifted: ["Á","Ä","Â","Ă","Ą"] }
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
            ReturnKey { weight: 1.5; label: "Enter"; shifted: "Enter"; alignTextRight: true }
        }

        // Shift, bottom row, Shift  [sum 12.5]
        KeyRow {
            height: keyHeight

            ShiftKey { }
            CharKey { label: "í"; shifted: "Í"; extended: ["í", "ì", "î", "ï"]; extendedShifted: ["Í", "Ì", "Î", "Ï"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["¥"]; extendedShifted: ["¥"] }
            CharKey { label: "x"; shifted: "X" }
            CharKey { label: "c"; shifted: "C"; extended: ["ć","č","ç"]; extendedShifted: ["Ć","Č","Ç"] }
            CharKey { label: "v"; shifted: "V" }
            CharKey { label: "b"; shifted: "B" }
            CharKey { label: "n"; shifted: "N"; extended: ["ń","ň","ñ"]; extendedShifted: ["Ń","Ň","Ñ"] }
            CharKey { label: "m"; shifted: "M" }
            AnnotatedKey { label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
            AnnotatedKey { label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
            ShiftKey { weight: 1.5 }
        }

        // Bottom row, default field  [sum 11]
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButton.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButton }
                SpaceKey       { weight: 5 }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        // Bottom row, email field  [sum 11]
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonEmail.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonEmail }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 4 }
                UrlKey { label: ".com"; shifted: ".com"; extended: [".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"] }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        // Bottom row, URL field  [sum 11]
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonUrl.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonUrl }
                UrlKey         { label: "/"; shifted: "/"; extended: ["http://", "https://", "www."] }
                SpaceKey       { weight: 4 }
                UrlKey { label: ".com"; shifted: ".com"; extended: [".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"] }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        Loader {
            width: parent.width

            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
