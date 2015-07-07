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

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey; label: "符号";              anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                CharKey        { id: commaKey;    label: ","; shifted: "，";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "。"; shifted: "。"; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;                               anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                CharKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".cn"; extended: [".中國", ".中国", ".hk", ".香港", ".mo", ".tw", ".台灣", ".台湾"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;                               anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                CharKey        { id: slashKey;    label: "/"; shifted: "/";  anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".cn"; extended: [".中國", ".中国", ".hk", ".香港", ".mo", ".tw", ".台灣", ".台湾"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;                               anchors.right: parent.right }
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
