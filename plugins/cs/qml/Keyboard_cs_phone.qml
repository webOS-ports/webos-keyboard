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

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€", "ě"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€", "Ě"]; }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R","®", "Ř", "Ŕ"]; }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"]; }
            CharKey { label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "û", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Û","Ù","Ú","Û", "Ü", "Ű"]; }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "§", "Π"]; }
            CharKey { label: "ú"; shifted: "/"; }
         }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"]; }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$", "Ś"]; }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"]; }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]; }
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"]; }
			CharKey { label: "ů"; shifted: "\""; }
         }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            ShiftKey {id: shiftKey; anchors.left: parent.left; }
            CharKey { id: yKey; anchors.left: shiftKey.right; label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; }
            CharKey { id: xKey; anchors.left: yKey.right; width: UI.keyWidth * 8/7; label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]; }
            CharKey { id: cKey; anchors.left: xKey.right; width: UI.keyWidth * 8/7; label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"]; }
            CharKey { id: vKey; anchors.left: cKey.right; width: UI.keyWidth * 8/7; label: "v"; shifted: "V"; }
            CharKey { id: bKey; anchors.left: vKey.right; width: UI.keyWidth * 8/7; label: "b"; shifted: "B"; }
            CharKey { id: nKey; anchors.left: bKey.right; width: UI.keyWidth * 8/7; label: "n"; shifted: "N"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"]; }
            CharKey { id: mKey; anchors.left: nKey.right; width: UI.keyWidth * 8/7; label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]; }
            BackspaceKey {id: backspaceKey; anchors.left: mKey.right; anchors.right: parent.right; }
        }

        Component {
            id: contentTypeNormal

            Item {
                height: keyHeight;

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                AnnotatedKey   { id: commaKey; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                                 anchors.right: dotKey.left; anchors.left: commaKey.right; }
                AnnotatedKey   { id: dotKey; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;      label: "Enter"; shifted: "Enter"; anchors.right: parent.right; }
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight;

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: atKey;    label: "@"; shifted: "@";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".cz"; shifted: ".sk"; extended: [".cz", ".sk", ".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey   { id: dotKey;      label: "."; shifted: "?"; showAnnotation2: false; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey; label: "Enter"; shifted: "Enter"; anchors.right: parent.right; }
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."]; anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".cz"; shifted: ".sk"; extended: [".cz", ".sk", ".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey   { id: dotKey;      label: "."; shifted: "?"; showAnnotation2: false;  anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;    label: "Enter"; shifted: "Enter"; anchors.right: parent.right; }
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
