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
            CharKey { label: "e"; shifted: "E"; extended: ["è", "é", "ê", "ë", "€"]; extendedShifted: ["È","É", "Ê", "Ë", "€"] }
            CharKey { label: "r"; shifted: "R"; }
            CharKey { label: "t"; shifted: "T"; extended: ["þ"]; extendedShifted: ["Þ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["ý", "¥"]; extendedShifted: ["Ý", "¥"] }
            CharKey { label: "u"; shifted: "U"; extended: ["û","ù","ú","ü"]; extendedShifted: ["Û","Ù","Ú","Ü"] }
            CharKey { label: "i"; shifted: "I"; extended: ["î","ï","ì","í"]; extendedShifted: ["Î","Ï","Ì","Í"] }
            CharKey { label: "o"; shifted: "O"; extended: ["ö","ô","ò","ó"]; extendedShifted: ["Ö","Ô","Ò","Ó"] }
            CharKey { label: "p"; shifted: "P"; }
         }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0
			//SpacerLuneOS {width: panel.keyWidth * 0.25 }
            CharKey { label: "a"; shifted: "A"; extended: ["ä","à","â","ª","á","å", "æ"]; extendedShifted: ["Ä","À","Â","ª","Á","Å","Æ"]; width: panel.keyWidth * 8/7 }
            CharKey { label: "s"; shifted: "S"; extended: ["ß","$"]; extendedShifted: ["$"]; width: panel.keyWidth * 8/7 }
            CharKey { label: "d"; shifted: "D"; extended: ["ð"]; extendedShifted: ["Ð"]; width: panel.keyWidth * 8/7 }
            CharKey { label: "f"; shifted: "F"; width: panel.keyWidth * 8/7}
            CharKey { label: "g"; shifted: "G"; width: panel.keyWidth * 8/7}
            CharKey { label: "h"; shifted: "H"; width: panel.keyWidth * 8/7}
            CharKey { label: "j"; shifted: "J"; width: panel.keyWidth * 8/7}
            CharKey { label: "k"; shifted: "K"; width: panel.keyWidth * 8/7}
            CharKey { label: "l"; shifted: "L"; width: panel.keyWidth * 8/7}
			//SpacerLuneOS { width: panel.keyWidth * 0.25 }
         }

     //   Row {
 Item {
            //anchors.left: parent.left
			//anchors.horizontalCenter: parent.horizontalCenter;
            //anchors.margins: 50;
            //spacing: 0
            anchors.left: parent.left
            anchors.right: parent.right

            height: panel.keyHeight;
		    
	// anchors.horizontalCenter: parent.horizontalCenter;
        //    spacing: 0

            ShiftKey {id: shiftKey; anchors.left: parent.left; /*anchors.right: zKey.left*/}
			//SpacerLuneOS { }
            CharKey { id: zKey; anchors.left: shiftKey.right; /*anchors.right: xKey.left;*/ label: "z"; shifted: "Z"; width: panel.keyWidth * 8/7; }
            CharKey { id: xKey; anchors.left: zKey.right; /*anchors.right: cKey.left;*/ label: "x"; shifted: "X"; width: panel.keyWidth * 8/7; }
            CharKey { id: cKey; anchors.left: xKey.right; /*anchors.right: vKey.left;*/ label: "c"; shifted: "C"; extended: ["ç"]; extendedShifted: ["Ç"]; width: panel.keyWidth * 8/7;  }
            CharKey { id: vKey; anchors.left: cKey.right; /*anchors.right: bKey.left;*/ label: "v"; shifted: "V"; width: panel.keyWidth * 8/7; }
            CharKey { id: bKey; anchors.left: vKey.right; /*anchors.right: nKey.left;*/ label: "b"; shifted: "B"; width: panel.keyWidth * 8/7; }
            CharKey { id: nKey; anchors.left: bKey.right; /*anchors.right: mKey.left;*/ label: "n"; shifted: "N"; extended: ["ñ"]; extendedShifted: ["Ñ"]; width: panel.keyWidth * 8/7;  }
            CharKey { id: mKey; anchors.left: nKey.right; /*anchors.right: backspaveKey.left;*/ label: "m"; shifted: "M"; width: panel.keyWidth * 8/7; }
			//SpacerLuneOS { }
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
