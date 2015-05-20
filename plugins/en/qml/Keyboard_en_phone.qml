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
    anchors.fill: parent

    content: c1
    symbols: "languages/Keyboard_symbols_phone.qml"

    Column {
        id: c1
        anchors.fill: parent
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€"] }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®"]; extendedShifted: ["R","®"] }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ"]; extendedShifted: ["T", "™", "Þ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"] }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "û", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Û","Ù","Ú","Û", "Ü", "Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"] }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"] }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "§", "Π"]}
         }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

			CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"] }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡"]; extendedShifted: ["D", "Ð", "†", "‡"] }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]}
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł"]; extendedShifted: ["L", "Ł"]}
         }

     //   Row {
 Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: panel.keyHeight;
		    
            ShiftKey {id: shiftKey; anchors.left: parent.left; }
            CharKey { id: zKey; anchors.left: shiftKey.right; width: panel.keyWidth * 8/7; label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; }
            CharKey { id: xKey; anchors.left: zKey.right; width: panel.keyWidth * 8/7; label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]; }
            CharKey { id: cKey; anchors.left: xKey.right; width: panel.keyWidth * 8/7; label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢"]; }
            CharKey { id: vKey; anchors.left: cKey.right; width: panel.keyWidth * 8/7; label: "v"; shifted: "V"; }
            CharKey { id: bKey; anchors.left: vKey.right; width: panel.keyWidth * 8/7; label: "b"; shifted: "B"; }
            CharKey { id: nKey; anchors.left: bKey.right; width: panel.keyWidth * 8/7; label: "n"; shifted: "N"; extended: ["n", "ñ", "ń"]; extendedShifted: ["N", "Ñ", "Ń"]; }
            CharKey { id: mKey; anchors.left: nKey.right; width: panel.keyWidth * 8/7; label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]; }
            BackspaceKey {id: backspaceKey; anchors.left: mKey.right; anchors.right: parent.right}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: panel.keyHeight;

			SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
			LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
            AnnotatedKey   { id: commaKey; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; anchors.left: languageMenuButton.right}
            SpaceKey       { id: spaceKey;                                 anchors.right: dotKey.left; anchors.left: commaKey.right; noMagnifier: true }
            AnnotatedKey   { id: dotKey; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; anchors.right: enterKey.left}
            ReturnKey      { id: enterKey;      label: "Enter"; shifted: "Enter"; extended: "Enter";  anchors.right: parent.right;}
        }
    } // column
}
