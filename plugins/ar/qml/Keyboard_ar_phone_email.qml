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

            CharKey { label: "ض"; }
            CharKey { label: "ص"; }
            CharKey { label: "ث"; }
            CharKey { label: "ق"; }
            CharKey { label: "ف"; }
            CharKey { label: "غ"; shifted: "إ"; }
            CharKey { label: "ع"; }
            CharKey { label: "ه"; }
            CharKey { label: "خ"; }
            CharKey { label: "ح"; }
            CharKey { label: "ج"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "ش"; }
            CharKey { label: "س"; }
            CharKey { label: "ي"; }
            CharKey { label: "ب"; }
            CharKey { label: "ل"; }
            CharKey { label: "ا"; shifted: "أ" }
            CharKey { label: "ت"; }
            CharKey { label: "ن"; }
            CharKey { label: "م"; }
            CharKey { label: "ك"; }
            CharKey { label: "د"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey { padding: 0 }
            CharKey { label: "ئ"; }
            CharKey { label: "ء"; }
            CharKey { label: "ؤ"; }
            CharKey { label: "ر"; }
            CharKey { label: "ى"; shifted: "آ" }
            CharKey { label: "ة"; }
            CharKey { label: "و"; }
            CharKey { label: "ز"; }
            CharKey { label: "ظ"; }
            BackspaceKey { padding: 0 }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
            CharKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; noMagnifier: true }
            UrlKey         { id: urlKey; label: ".eg"; extended: [".iq", ".lb", ".sa", ".sy", ".jo", ".ye"]; anchors.right: dotKey.left; }
            CharKey        { id: dotKey;      label: "."; shifted: "ذ";  anchors.right: specialChar.left; }
            CharKey        { id: specialChar; label: "ط";                anchors.right: enterKey.left }
            ReturnKey      { id: enterKey;                               anchors.right: parent.right }
        }
    } // column
}
