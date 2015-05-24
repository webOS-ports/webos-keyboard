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
import keys 1.0

KeyPad {
    content: c1
    symbols: "languages/Keyboard_symbols_tablet.qml"

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
            CharKey { label: "e"; shifted: "E"; extended: ["é","ê","è","ë","€"]; extendedShifted: ["É","Ê","È","Ë","€"] }
            CharKey { label: "r"; shifted: "R"; }
            CharKey { label: "t"; shifted: "T"; extended: ["þ"]; extendedShifted: ["Þ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["ý", "¥", "ÿ"]; extendedShifted: ["Ý", "¥", "Ÿ"] }
            CharKey { label: "u"; shifted: "U"; extended: ["ú","û","ù","ü"]; extendedShifted: ["Ú","Û","Ù","Ü"] }
            CharKey { label: "i"; shifted: "I"; extended: ["í","î","ì","ï"]; extendedShifted: ["Í","Î","Ì","Ï"] }
            CharKey { label: "o"; shifted: "O"; extended: ["ó","õ","ô","º","ò","ö"]; extendedShifted: ["Ó","Õ","Ô","º","Ò","Ö"] }
            CharKey { label: "p"; shifted: "P"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "a"; shifted: "A"; extended: ["ã","á","à","â","ª","ä","å","æ"]; extendedShifted: ["Ã","Á","À","Â","ª","Ä","Å","Æ"] }
            CharKey { label: "s"; shifted: "S"; extended: ["ß","$"]; extendedShifted: ["$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["ð"]; extendedShifted: ["Ð"] }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; }
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; }
            CharKey { label: "ç"; shifted: "Ç"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey {}
            CharKey { label: "z"; shifted: "Z"; }
            CharKey { label: "x"; shifted: "X"; }
            CharKey { label: "c"; shifted: "C"; extended: ["ç"]; extendedShifted: ["Ç"] }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["ñ"]; extendedShifted: ["Ñ"] }
            CharKey { label: "m"; shifted: "M"; }
            BackspaceKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
            CharKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; noMagnifier: true }
            UrlKey         { id: urlKey; label: ".br"; extended: [".pt", ".mz", ".ao", ".gw", ".tl"]; anchors.right: dotKey.left; }
            CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: enterKey.left; }
            ReturnKey      { id: enterKey;                               anchors.right: parent.right }
        }
    } // column
}
