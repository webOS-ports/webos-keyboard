/*
 * Copyright 2013 Canonical Ltd.
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
import "../../keys"

KeyPad {
    anchors.fill: parent

    content: c1
    symbols: "languages/webos/Keyboard_symbols.qml"

    Column {
        id: c1
        anchors.fill: parent
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            CharKey { label: "1"; shifted: "!"; }
            CharKey { label: "2"; shifted: "@"; }
            CharKey { label: "3"; shifted: "#"; extended: ["è", "é", "ê", "ë", "€"]; extendedShifted: ["È","É", "Ê", "Ë", "€"] }
            CharKey { label: "4"; shifted: "$"; }
            CharKey { label: "5"; shifted: "%"; extended: ["þ"]; extendedShifted: ["Þ"] }
            CharKey { label: "6"; shifted: "^"; extended: ["ý", "¥"]; extendedShifted: ["Ý", "¥"] }
            CharKey { label: "7"; shifted: "&"; extended: ["û","ù","ú","ü"]; extendedShifted: ["Û","Ù","Ú","Ü"] }
            CharKey { label: "8"; shifted: "*"; extended: ["î","ï","ì","í"]; extendedShifted: ["Î","Ï","Ì","Í"] }
            CharKey { label: "9"; shifted: "("; extended: ["ö","ô","ò","ó"]; extendedShifted: ["Ö","Ô","Ò","Ó"] }
            CharKey { label: "0"; shifted: ")"; }
        }

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
            BackspaceKey {}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            CharKey { label: "a"; shifted: "A"; extended: ["ä","à","â","ª","á","å", "æ"]; extendedShifted: ["Ä","À","Â","ª","Á","Å","Æ"] }
            CharKey { label: "s"; shifted: "S"; extended: ["ß","$"]; extendedShifted: ["$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["ð"]; extendedShifted: ["Ð"] }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; }
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; }
            ReturnKey { id: enterKey; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            ShiftKey {}
            CharKey { label: "z"; shifted: "Z"; }
            CharKey { label: "x"; shifted: "X"; }
            CharKey { label: "c"; shifted: "C"; extended: ["ç"]; extendedShifted: ["Ç"] }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["ñ"]; extendedShifted: ["Ñ"] }
            CharKey { label: "m"; shifted: "M"; }
            CharKey { label: ","; shifted: "/" }
            CharKey { label: "."; shifted: "?" }
            ShiftKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: panel.keyHeight;

            SymbolShiftKey { id: symShiftKey;                              anchors.left: parent.left; }
            LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: apostropheKey.left; noMagnifier: true }
            CharKey        { id: apostropheKey; label: "'"; shifted: "\""; anchors.right: minusKey.left; }
            CharKey        { id: minusKey;      label: "-"; shifted: "_";  anchors.right: parent.right; }
        }
    } // column
}
