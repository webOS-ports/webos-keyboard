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
import "../keys"

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

            CharKey { label: "`"; }
            CharKey { label: "~"; }
            CharKey { label: "€"; }
            CharKey { label: "£"; }
            CharKey { label: "\\"; }
            CharKey { label: "\|"; }
            CharKey { label: "\{"; }
            CharKey { label: "\}"; }
            CharKey { label: "\["; }
            CharKey { label: "\]"; }
            BackspaceKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: panel.keyHeight;

            CharKey { id: lessKey; label: "\<"; anchors.right: greaterKey.left}
            CharKey { id: greaterKey; label: "\>"; anchors.right: equalsKey.left}
            CharKey { id: equalsKey; label: "\="; anchors.right: plusKey.left}
            CharKey { id: plusKey; label: "\+"; anchors.right: multiplyKey.left}
            CharKey { id: multiplyKey; label: "\×"; anchors.right: dividedKey.left}
            CharKey { id: dividedKey; label: "\÷"; anchors.right: degreeKey.left}
            CharKey { id: degreeKey; label: "\°"; anchors.right: semicolonKey.left}
            CharKey { id: semicolonKey; label: "\;"; anchors.right: colonKey.left}
            CharKey { id: colonKey; label: "\:"; anchors.right: enterKey.left}
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; anchors.right: parent.right;}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            ShiftKey {}
            CharKey { label: ":)"; }
            CharKey { label: ";)"; }
            CharKey { label: ":("; }
            CharKey { label: ":'("; }
            CharKey { label: ":P"; }
            CharKey { label: ":O"; }
            CharKey { label: "<3"; }
            GreyKey { label: ","; shifted: "/"; extended: "/"; }
            GreyKey { label: "."; shifted: "?"; extended: "?"; }
            ShiftKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: panel.keyHeight;

            SymbolShiftKey { id: symbolKey; anchors.left: parent.left;}
            SpaceKey       { id: spaceKey; noMagnifier: true; anchors.left: symbolKey.right; anchors.right: quoteKey.left;}
            GreyKey { id: quoteKey; label: "'"; shifted: '"';  extended: '"'; anchors.right: dashKey.left }
            GreyKey { id: dashKey; label: "-"; shifted: "_"; extended: "_"; anchors.right: dismissKey.left; }
            DismissKey { id: dismissKey; anchors.right: parent.right; width: panel.keyWidth;}
        }

    } // column
}
