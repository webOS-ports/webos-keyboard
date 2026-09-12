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
 * Weights follow PhoneKeymap.cpp, sQwerty + cCustom_QWERT_*. Every row sums to 10.
 *
 * There is no language key and no hide key on this keyboard: the shift key's
 * symbol-layer identity is cKey_ToggleLanguage, so language switching lives on the
 * 123 page (see languages/Keyboard_symbols_phone.qml).
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

        // QWERTY_TOP_10(1)
        KeyRow {
            height: keyHeight

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€", "ě"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€", "Ě"]; }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R","®", "Ř", "Ŕ"]; }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"]; }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Ù","Ú","Û", "Ü", "Ű"]; }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "§", "Π"]; }
        }

        // KEY_4(-0.5, A) + QWERTY_MID_9(1) + KEY_4(-0.5, L): the home row is inset by
        // half a key at each end, and those halves stay touchable.
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: aKey }
            CharKey { id: aKey; label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"]; }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$", "Ś"]; }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"]; }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]; }
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { id: lKey; label: "l"; shifted: "L"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"]; }
            SpacerKey { weight: 0.5; forwardTo: lKey }
        }

        // KEY_4(1.25, Shift) + KEY_4(-0.25, Z) + QWERTY_LOW_7(1) + KEY_1(-0.25, Backspace)
        // + KEY_1(1.25, Backspace). The letters stay one unit wide so they line up
        // with the row above; the quarter-key pads beside shift and backspace make
        // those two comfortable to hit without being drawn.
        KeyRow {
            height: keyHeight

            ShiftKey { id: shiftKey; weight: 1.25 }
            SpacerKey { weight: 0.25; forwardTo: zKey }
            CharKey { id: zKey; label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; }
            CharKey { label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]; }
            CharKey { label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"]; }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"]; }
            CharKey { label: "m"; shifted: "M"; extended: ["m", "µ"]; extendedShifted: ["M", "Μ"]; }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.25 }
        }

        // QWERT_BOTTOM_ROW with cCustom_QWERT_plain
        Component {
            id: contentTypeNormal

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey   { label: ","; shifted: "/"; weight: 1.5; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; }
                SpaceKey       { weight: 4 }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        // cCustom_QWERT_email: @ and .com flank the space key, which drops to
        // SPACE_KEY_WEIGHT - 1 - 1.
        Component {
            id: contentTypeEmail

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey   { label: ","; shifted: "/"; weight: 1.5; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; }
                UrlKey         { label: "@"; shifted: "@"; }
                SpaceKey       { weight: 2 }
                UrlKey         { label: ".com"; shifted: ".com"; extended: [".com", ".net", ".edu", ".org", ".co.uk"]; }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        // cCustom_QWERT_url: a plain slash, then a colon whose popup carries the
        // scheme prefixes (sURL_extended).
        Component {
            id: contentTypeUrl

            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                UrlKey         { label: "/"; shifted: "/"; }
                UrlKey         { label: ":"; shifted: ":"; extended: ["://", "http://", "https://"]; }
                SpaceKey       { weight: 2 }
                UrlKey         { label: ".com"; shifted: ".com"; extended: [".com", ".net", ".edu", ".org", ".co.uk"]; }
                AnnotatedKey   { label: "."; shifted: "?"; weight: 1.5; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; }
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
