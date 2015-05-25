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

            CharKey { label: "q"; }
            CharKey { label: "w"; }
            CharKey { label: "e"; }
            CharKey { label: "r"; }
            CharKey { label: "t"; }
            CharKey { label: "y"; }
            CharKey { label: "u"; }
            CharKey { label: "i"; }
            CharKey { label: "o"; }
            CharKey { label: "p"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "a"; }
            CharKey { label: "s"; }
            CharKey { label: "d"; }
            CharKey { label: "f"; }
            CharKey { label: "g"; }
            CharKey { label: "h"; }
            CharKey { label: "j"; }
            CharKey { label: "k"; }
            CharKey { label: "l"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey {}
            CharKey { label: "z"; }
            CharKey { label: "x"; }
            CharKey { label: "c"; }
            CharKey { label: "v"; }
            CharKey { label: "b"; }
            CharKey { label: "n"; }
            CharKey { label: "m"; }
            BackspaceKey {}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            SymbolShiftKey { id: symShiftKey; label: "符号";              anchors.left: parent.left; }
            LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
            CharKey        { id: commaKey;    label: ","; shifted: "，";  anchors.left: languageMenuButton.right; }
            SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; noMagnifier: true }
            CharKey        { id: dotKey;      label: "。"; shifted: "。"; anchors.right: enterKey.left; }
            ReturnKey      { id: enterKey;                               anchors.right: parent.right }
        }
    } // column
}
