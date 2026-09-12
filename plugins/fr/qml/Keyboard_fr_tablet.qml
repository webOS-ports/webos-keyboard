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
 * Weights follow tabletkeymaps/fr.h, sFrAzerty. Row sums: 12 / 11.5 / 11.5 / 11.5 / 11.5.
 * This keymap sets needNumLock, so number and phone fields promote the digits
 * rather than switching to a separate pad.
 */

import QtQuick 2.0
import keys 1.0

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_tablet.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        // FR_AZERTY_NUMBERS_10(1) + KEY_1(2, cKey_Trackball)  [sum 12]
        KeyRow {
            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { label: "&"; shifted: "1"; extended: ["&", "1", "¹", "¼", "½", "¡"] }
            NumKey { label: "é"; shifted: "2"; extended: ["é", "2", "²"] }
            NumKey { label: "\""; shifted: "3"; extended: ["\"", "3", "³", "¾", "“", "”", "«", "»"] }
            NumKey { label: "'"; shifted: "4"; extended: ["'", "4", "‘", "’"] }
            NumKey { label: "("; shifted: "5"; extended: ["(", "5", "[","{"] }
            NumKey { label: "-"; shifted: "6"; extended: ["-", "6", "±", "¬"] }
            NumKey { label: "è"; shifted: "7"; extended: ["è", "7", "`"] }
            NumKey { label: ")"; shifted: "8"; extended: [")", "8", "]","}"] }
            NumKey { label: "ç"; shifted: "9"; extended: ["ç", "9", "¢", "$", "€", "£", "¥", "¤"] }
            NumKey { label: "à"; shifted: "0"; extended: ["à", "0", "%", "‰"] }
            TrackBall { }
        }

        // FR_AZERTY_TOP_10(1) + KEY_1(1.5, Backspace)  [sum 11.5]
        KeyRow {
            height: keyHeight

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"] }
            CharKey { label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"] }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€", "ě"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€", "Ě"] }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R","®", "Ř", "Ŕ"] }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"] }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "û", "ü", "ű"]; extendedShifted: ["U", "Ù", "Ú", "Û", "Ü", "Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"] }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"] }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"] }
            BackspaceKey { weight: 1.5 }
        }

        // KEY_2(-0.5, Q, Less) + FR_AZERTY_MID_10(1) + KEY_1(1, Return)  [sum 11.5]
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: qKey }
            CharKey { id: qKey; label: "q"; shifted: "Q" }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$", "Ś"] }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"] }
            CharKey { label: "f"; shifted: "F" }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"] }
            CharKey { label: "h"; shifted: "H" }
            CharKey { label: "j"; shifted: "J" }
            CharKey { label: "k"; shifted: "K" }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"] }
            CharKey { label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"] }
            ReturnKey { weight: 1; label: "Enter"; shifted: "Enter" }
        }

        // KEY_1(1, Shift) + FR_AZERTY_LOW_9(1) + KEY_1(1.5, Shift)  [sum 11.5]
        KeyRow {
            height: keyHeight

            ShiftKey { }
            CharKey { label: "w"; shifted: "W" }
            CharKey { label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"] }
            CharKey { label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"] }
            CharKey { label: "v"; shifted: "V" }
            CharKey { label: "b"; shifted: "B" }
            CharKey { label: "n"; shifted: "N"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"] }
            AnnotatedKey { label: ","; shifted: "?"; extended: [",", "?", "¿"]; extendedShifted: [",", "?", "¿"] }
            AnnotatedKey { label: "."; shifted: ";"; extended: [".", ";", "•", "…"]; extendedShifted: [".", ";", "•", "…"] }
            AnnotatedKey { label: ":"; shifted: "/"; extended: [":", "/", "\\"]; extendedShifted: [":", "/", "\\"] }
            ShiftKey { weight: 1.5 }
        }

        // Bottom row, default field. cKey_Symbol is 2 units and gives up its right half
        // only when updateLanguageKey() has a language to show.
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButton.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButton }
                SpaceKey       { weight: 5 }
                AnnotatedKey { label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"] }
                AnnotatedKey { label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"] }
                DismissKey     { weight: 1.5 }
            }
        }

        // Bottom row, email field: space drops to SPACE_SIZE - 2.
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonEmail.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonEmail }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".com"; extended: [".com", ".fr", ".net", ".org", ".ca", ".cd", ".ci", ".ch", ".be", ".ht", ".edu", ".gov"] }
                AnnotatedKey { label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"] }
                AnnotatedKey { label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"] }
                DismissKey     { weight: 1.5 }
            }
        }

        // Bottom row, URL field.
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonUrl.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonUrl }
                CharKey { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; label: "/"; shifted: "/"; extended: ["http://", "https://", "www."] }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".com"; extended: [".com", ".fr", ".net", ".org", ".ca", ".cd", ".ci", ".ch", ".be", ".ht", ".edu", ".gov"] }
                AnnotatedKey { label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"] }
                AnnotatedKey { label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"] }
                DismissKey     { weight: 1.5 }
            }
        }

        Loader {
            width: parent.width
            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
