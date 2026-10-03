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
 * The Palm Pre's own keyboard, key for key: four rows of ten keys in matching
 * columns, the orange key and Sym, and each key's alternate character printed
 * where the Pre prints it (see CharKey.alt). The orange key (AltKey) types
 * those alternates; long-press still offers the accented letters.
 *
 * The bottom row is the Pre's: shift, @/0, space, period and Sym, inset by
 * about a key at each end. Weights are measured off the Pre and sum to 10.
 * There is no language key and no hide key: language switching lives on the
 * Sym page (see languages/Keyboard_symbols_phone.qml).
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

        KeyRow {
            height: keyHeight

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; alt: "1"; altPosition: "left"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€", "ě"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€", "Ě"]; }
            CharKey { label: "r"; shifted: "R"; alt: "2"; altPosition: "left"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R","®", "Ř", "Ŕ"]; }
            CharKey { label: "t"; shifted: "T"; alt: "3"; altPosition: "left"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"]; }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Ù","Ú","Û", "Ü", "Ű"]; }
            CharKey { label: "i"; shifted: "I"; alt: "%"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "§", "Π"]; }
        }

        KeyRow {
            height: keyHeight

            CharKey { label: "a"; shifted: "A"; alt: "&"; altPosition: "left"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"]; }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$", "Ś"]; }
            CharKey { label: "d"; shifted: "D"; alt: "4"; altPosition: "left"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"]; }
            CharKey { label: "f"; shifted: "F"; alt: "5"; altPosition: "left"; }
            CharKey { label: "g"; shifted: "G"; alt: "6"; altPosition: "left"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]; }
            CharKey { label: "h"; shifted: "H"; alt: "$"; }
            CharKey { label: "j"; shifted: "J"; alt: "!"; }
            CharKey { label: "k"; shifted: "K"; alt: ":"; }
            CharKey { label: "l"; shifted: "L"; alt: "'"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"]; }
            BackspaceKey { }
        }

        KeyRow {
            height: keyHeight

            AltKey { }
            CharKey { label: "z"; shifted: "Z"; alt: "*"; altPosition: "left"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; }
            CharKey { label: "x"; shifted: "X"; alt: "7"; altPosition: "left"; }
            CharKey { label: "c"; shifted: "C"; alt: "8"; altPosition: "left"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"]; }
            CharKey { label: "v"; shifted: "V"; alt: "9"; altPosition: "left"; }
            CharKey { label: "b"; shifted: "B"; alt: "#"; }
            CharKey { label: "n"; shifted: "N"; alt: "?"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"]; }
            CharKey { label: "m"; shifted: "M"; alt: ";"; extended: ["m", "µ"]; extendedShifted: ["M", "Μ"]; }
            CharKey { label: ","; shifted: ","; alt: "-"; }
            ReturnKey {
                weight: 1
                label: maliit_input_method.actionKeyLabel.length > 0 ? maliit_input_method.actionKeyLabel : ""
                iconNormal: "return"; iconShifted: "return"; iconCapsLock: "return"
            }
        }

        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.97; forwardTo: shiftKey }
            ShiftKey { id: shiftKey; weight: 1.43 }
            CharKey { label: "@"; shifted: "@"; alt: "0"; altPosition: "inline"; weight: 1.18 }
            SpaceKey { weight: 2.75 }
            CharKey { label: "."; shifted: "."; weight: 1.17; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; }
            SymbolShiftKey { id: symKey; label: "Sym"; shifted: "Sym"; weight: 1.48 }
            SpacerKey { weight: 1.02; forwardTo: symKey }
        }
    } // column
}
