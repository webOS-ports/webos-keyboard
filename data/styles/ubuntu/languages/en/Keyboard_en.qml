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
    symbols: "languages/Keyboard_symbols.qml"

    Column {
        id: c1
        anchors.fill: parent
        spacing: 0

        Row {
            height: panel.keyHeight * .75;
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            NumKey { label: "1"; shifted: "!"; extended: ["!", "¹", "¼", "½", "¡"];}
            NumKey { label: "2"; shifted: "@"; extended: ["@", "²"];}
            NumKey { label: "3"; shifted: "#"; extended: ["#", "³", "¾"]; }
            NumKey { label: "4"; shifted: "$"; extended: ["$", "€", "£", "¥", "¢" ,"¤" ];}
            NumKey { label: "5"; shifted: "%"; extended: ["%", "‰"]; }
            NumKey { label: "6"; shifted: "^"; extended: ["^"]; }
            NumKey { label: "7"; shifted: "&"; extended: ["&"] }
            NumKey { label: "8"; shifted: "*"; extended: ["*"] }
            NumKey { label: "9"; shifted: "("; extended: ["(", "[", "{"] }
            NumKey { label: "0"; shifted: ")"; extended: [")", "]", "}"]  }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; extended: ["è", "é", "ê", "ë", "ę", "ē"]; extendedShifted: ["È","É", "Ê", "Ë", "Ę", "Ē"] }
            CharKey { label: "r"; shifted: "R"; extended: ["®"]}
            CharKey { label: "t"; shifted: "T"; extended: ["™", "þ"]; extendedShifted: ["Þ"] }
            CharKey { label: "y"; shifted: "Y"; extended: ["ý", "ÿ"]; extendedShifted: ["Ý", "Ÿ"] }
            CharKey { label: "u"; shifted: "U"; extended: ["ù","ú","û","ü","ű"]; extendedShifted: ["Ù","Ú","Û","Ü","Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["ì","í","î","ï","İ","ı"]; extendedShifted: ["Ì", "Í", "Î", "Ï", "İ", "I"] }
            CharKey { label: "o"; shifted: "O"; extended: ["ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"] }
            CharKey { label: "p"; shifted: "P"; extended: ["§", "π"]; extendedShifted: ["§", "Π"] }
            BackspaceKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: panel.keyHeight;

            CharKey { id: aKey; label: "a"; shifted: "A"; extended: ["à","á","â","ã","ä","å","æ","ª"]; extendedShifted: ["À","Á","Â","Ã","Ä","Å","Æ","ª"]; anchors.right: sKey.left; }
            CharKey { id: sKey; label: "s"; shifted: "S"; extended: ["š", "ş", "ß", "σ"]; extendedShifted: ["Š", "Ş", "ß", "Σ"]; anchors.right: dKey.left; }
            CharKey { id: dKey; label: "d"; shifted: "D"; extended: ["ð", "†", "‡"]; extendedShifted: ["Ð", "†", "‡"]; anchors.right: fKey.left; }
            CharKey { id: fKey; label: "f"; shifted: "F"; anchors.right: gKey.left;}
            CharKey { id: gKey; label: "g"; shifted: "G"; extended: ["ğ"]; extendedShifted: ["Ğ"]; anchors.right: hKey.left;}
            CharKey { id: hKey; label: "h"; shifted: "H"; anchors.right: jKey.left;}
            CharKey { id: jKey; label: "j"; shifted: "J"; anchors.right: kKey.left;}
            CharKey { id: kKey; label: "k"; shifted: "K"; anchors.right: lKey.left;}
            CharKey { id: lKey; label: "l"; shifted: "L"; extended: ["ł"]; extendedShifted: ["Ł"]; anchors.right: enterKey.left;}
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; anchors.right: parent.right;}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            ShiftKey {}
            CharKey { label: "z"; shifted: "Z"; extended: ["ž", "ź", "ż"]; extendedShifted: ["Ž", "Ź", "Ż"]; }
            CharKey { label: "x"; shifted: "X"; }
            CharKey { label: "c"; shifted: "C"; extended: ["ç", "ć", "©", "¢"]; extendedShifted: ["Ç", "Ć", "©", "¢"] }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["ñ", "ń"]; extendedShifted: ["Ñ", "Ń"] }
            CharKey { label: "m"; shifted: "M"; extended: ["µ"]; extendedShifted: ["Μ"] }
            GreyKey { label: ","; shifted: "/"; extended: "/"; }
            GreyKey { label: "."; shifted: "?"; extended: "?", "•", "…", "¿"; }
            ShiftKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: panel.keyHeight;

            SymbolShiftKey { id: symbolKey; anchors.left: parent.left;}
            SpaceKey       { id: spaceKey; noMagnifier: true; anchors.left: symbolKey.right; anchors.right: quoteKey.left;}
            GreyKey { id: quoteKey; label: "'"; shifted: '"';  extended: "'", '"', "`", "‘", "’", "“", "”", "«", "»"; anchors.right: dashKey.left }
            GreyKey { id: dashKey; label: "-"; shifted: "_"; extended: "_", "±", "¬"; anchors.right: dismissKey.left; }
            DismissKey { id: dismissKey; anchors.right: parent.right; width: panel.keyWidth;}
        }

    } // column
}
