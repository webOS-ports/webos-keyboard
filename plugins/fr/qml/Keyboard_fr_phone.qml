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
 * Weights follow PhoneKeymap.cpp, sAzerty + cCustom_AZERTY_*. The letter rows sum
 * to 10.5 and the bottom row to 11, which is deliberate: azerty gives the symbol
 * and Enter keys 2 units each rather than 1.5, so its bottom row keys come out
 * slightly narrower than the letters above them.
 *
 * There is no language key and no hide key: the shift key's symbol-layer identity
 * is cKey_ToggleLanguage, so language switching lives on the 123 page.
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

        // AZERTY_TOP_10(1) + KEY_2(-0.5, P, BracketRight)  [sum 10.5]
        KeyRow {
            height: keyHeight

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "â", "æ", "á", "ä", "ã", "å", "ª"]; extendedShifted: ["A", "À", "Â", "Æ", "Á", "Ä", "Ã", "Å", "ª"] }
            CharKey { label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"] }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "é", "è", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "É", "È", "Ê", "Ë", "Ę", "Ē", "€"] }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R", "®", "Ř", "Ŕ"] }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ÿ", "ý", "¥"]; extendedShifted: ["Y", "Ÿ", "Ý", "¥"] }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "û", "ü", "ú", "ű"]; extendedShifted: ["U", "Ù", "Û", "Ü", "Ú", "Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "î", "ï", "ì", "í", "İ", "ı"]; extendedShifted: ["I", "Î", "Ï", "Ì", "Í", "İ", "ı"] }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ô", "ö", "ò", "ó", "õ", "ø", "ő", "œ", "º"]; extendedShifted: ["O", "Ô", "Ö", "Ò", "Ó", "Õ", "Ø", "Ő", "Œ", "º"] }
            CharKey { id: pKey; label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"] }
            SpacerKey { weight: 0.5; forwardTo: pKey }
        }

        // KEY_4(-0.5, Q, Less) + AZERTY_MID_10(1)  [sum 10.5]
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: qKey }
            CharKey { id: qKey; label: "q"; shifted: "Q" }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "Ś"] }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"] }
            CharKey { label: "f"; shifted: "F" }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"] }
            CharKey { label: "h"; shifted: "H" }
            CharKey { label: "j"; shifted: "J" }
            CharKey { label: "k"; shifted: "K" }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"] }
            CharKey { label: "m"; shifted: "M"; extended: ["m", "µ"]; extendedShifted: ["M", "Μ"] }
        }

        // KEY_4(1.25, Shift) + KEY_4(-0.25, Shift) + AZERTY_LOW_6(1)
        // + KEY_4(1.5, Apostrophe, At) + KEY_1(-0.25, Backspace) + KEY_1(1.25, Backspace)
        // [sum 10.5]. Azerty is the one layout whose quarter-key pad beside shift is
        // another shift rather than the first letter.
        KeyRow {
            height: keyHeight

            ShiftKey { id: shiftKey; weight: 1.25 }
            SpacerKey { weight: 0.25; forwardTo: shiftKey }
            CharKey { label: "w"; shifted: "W" }
            CharKey { label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"] }
            CharKey { label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"] }
            CharKey { label: "v"; shifted: "V" }
            CharKey { label: "b"; shifted: "B" }
            CharKey { label: "n"; shifted: "N"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"] }
            AnnotatedKey { label: "'"; shifted: "@"; weight: 1.5 }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.25 }
        }

        // AZERTY_BOTTOM with cCustom_AZERTY_plain  [sum 11]
        Component {
            id: contentTypeNormal

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 2 }
                AnnotatedKey   { label: ","; shifted: "?"; weight: 1.5; extended: [",", "?", "¿"]; extendedShifted: [",", "?", "¿"] }
                SpaceKey       { weight: 4 }
                AnnotatedKey   { label: "."; shifted: "!"; weight: 1.5; extended: [".", "!", "•", "…", "¡"]; extendedShifted: [".", "!", "•", "…", "¡"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 2 }
            }
        }

        // cCustom_AZERTY_email  [sum 11]
        Component {
            id: contentTypeEmail

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 2 }
                AnnotatedKey   { label: ","; shifted: "?"; weight: 1.5; extended: [",", "?", "¿"]; extendedShifted: [",", "?", "¿"] }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 2 }
                UrlKey         { label: ".com"; shifted: ".fr"; extended: [".com", ".fr", ".net", ".org", ".edu"] }
                AnnotatedKey   { label: "."; shifted: "!"; weight: 1.5; extended: [".", "!", "•", "…", "¡"]; extendedShifted: [".", "!", "•", "…", "¡"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 2 }
            }
        }

        // cCustom_AZERTY_url  [sum 10.5]
        Component {
            id: contentTypeUrl

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 2 }
                UrlKey         { label: "/"; shifted: "/" }
                UrlKey         { label: ":"; shifted: ":"; extended: ["://", "http://", "https://"] }
                SpaceKey       { weight: 2 }
                UrlKey         { label: ".com"; shifted: ".fr"; extended: [".com", ".fr", ".net", ".org", ".edu"] }
                AnnotatedKey   { label: "."; shifted: "!"; weight: 1.5; extended: [".", "!", "•", "…", "¡"]; extendedShifted: [".", "!", "•", "…", "¡"] }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 2 }
            }
        }

        Loader {
            anchors.left: parent.left
            anchors.right: parent.right

            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
