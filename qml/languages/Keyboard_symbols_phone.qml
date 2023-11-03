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

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 0;

        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight

            CharKey { label: "1"; shifted: "1"; }
            CharKey { label: "2"; shifted: "2"; }
            CharKey { label: "3"; shifted: "3"; }
            CharKey { label: "4"; shifted: "4"; }
            CharKey { label: "5"; shifted: "5"; }
            CharKey { label: "6"; shifted: "6"; }
            CharKey { label: "7"; shifted: "7"; }
            CharKey { label: "8"; shifted: "8"; }
            CharKey { label: "9"; shifted: "9"; }
            CharKey { label: "0"; shifted: "0"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            // anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "!"; shifted: "!"; }
            CharKey { label: "@"; shifted: "@"; }
            CharKey { label: "#"; shifted: "#";}
            CharKey { label: "$"; shifted: "$";}
            CharKey { label: "%"; shifted: "%";}
            CharKey { label: "&"; shifted: "&";}
            CharKey { label: "*"; shifted: "*";}
            CharKey { label: "("; shifted: "(";}
            CharKey { label: ")"; shifted: ")";}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            ShiftKey {id: shiftKey; anchors.left: parent.left;}
            CharKey {width: UI.keyWidth; id: semicolonKey; anchors.left: shiftKey.right; label: ";"; shifted: ";"; }
            CharKey {width: UI.keyWidth; id: colonKey; anchors.left: semicolonKey.right; label: ":"; shifted: ":";}
            CharKey {width: UI.keyWidth; id: equalKey; anchors.left: colonKey.right; label: "="; shifted: "=";}
            CharKey {width: UI.keyWidth; id: plusKey; anchors.left: equalKey.right; label: "+"; shifted: "+";}
            CharKey {width: UI.keyWidth; id: minusKey; anchors.left: plusKey.right; label: "-"; shifted: "-";}
            CharKey {width: UI.keyWidth; id: underscoreKey; anchors.left: minusKey.right; label: "_"; shifted: "_";}
            CharKey {width: UI.keyWidth; id: apostropheKeyM; anchors.left: underscoreKey.right; label: "'"; shifted: "'";}
            CharKey {width: UI.keyWidth; id: quoteKey; anchors.left: apostropheKeyM.right; label: "\""; shifted: "\"";}
            BackspaceKey {id: backspaceKey; anchors.right: parent.right; anchors.left: quoteKey.right     }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

            SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC"; anchors.left: parent.left; }
            LanguageKey    { id: languageMenuButton; anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey; anchors.left: languageMenuButton.right; anchors.right: smileyKey.left; }
            CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: smileyKey; label: "..."; shifted: "..."; anchors.right: dismissKey.left; }
            DismissKey     { id: dismissKey; anchors.right: enterKey.left}
            ReturnKey      { id: enterKey; label: "Enter"; shifted: "Enter"; extended: "Enter"; anchors.right: parent.right;}
        }
    } // column
}
