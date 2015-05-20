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
import "../keys"
import "../keys/key_constants.js" as UI

KeyPad {
    anchors.fill: parent

    content: c1

    Column {
        id: c1
        anchors.fill: parent
        anchors.margins: 0;

        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            NumKey { label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]}
            NumKey { label: "2"; shifted: "@"; extended: ["2", "@", "²"]}
            NumKey { label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"]}
            NumKey { label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]}
            NumKey { label: "5"; shifted: "%"; extended: ["5", "%", "‰"]}
            NumKey { label: "6"; shifted: "^"; extended: ["6", "^"]}
            NumKey { label: "7"; shifted: "&"; extended: ["7", "&"]}
            NumKey { label: "8"; shifted: "*"; extended: ["8", "*"]}
            NumKey { label: "9"; shifted: "("; extended: ["9", "(", "[", "{"]}
            NumKey { label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"]}
            TrackBall { width: c1.width - (panel.keyWidth*0.9*10); height: 45 }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
           // anchors.margins: 50;
            spacing: 0

            CharKey { label: "`"; shifted: "`"}
            CharKey { label: "~"; shifted: "~"; }
            CharKey { label: "€"; shifted: "€"; }
            CharKey { label: "£"; shifted: "£"; }
			CharKey { label: "\\"; shifted: "\\"; }
            CharKey { label: "|"; shifted: "|"; }
            CharKey { label: "{"; shifted: "{"; }
            CharKey { label: "}"; shifted: "}"; }
            CharKey { label: "["; shifted: "["; }
            CharKey { label: "]"; shifted: "]"; }
			BackspaceKey {}
        }

        Row {
            anchors.right: parent.right
			//anchors.horizontalCenter: parent.horizontalCenter;
            //anchors.margins: 50;
            spacing: 0

            CharKey { label: "<"; shifted: "<";}
            CharKey { label: ">"; shifted: ">";}
            CharKey { label: "="; shifted: "=";}
            CharKey { label: "+"; shifted: "+";}
            CharKey { label: "×"; shifted: "×"; }
            CharKey { label: "÷"; shifted: "÷"; }
            CharKey { label: "°"; shifted: "°"; }
            CharKey { label: ";"; shifted: ";"; }
            CharKey { label: ":"; shifted: ":"; }
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; extended: "Enter"; noMagnifier: true; }

        }

		Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            //anchors.margins: 50;
            spacing: 0

			ShiftKey {}
            CharKey { label: ":-)"; shifted: ":-)";}
            CharKey { label: ";-)"; shifted: ";-)";}
            CharKey { label: ":-("; shifted: ":-(";}
            CharKey { label: ":'("; shifted: ":'(";}
            CharKey { label: ":-P"; shifted: ":-P"; }
            CharKey { label: ":-O"; shifted: ":-O"; }
            CharKey { label: "<3"; shifted: "<3"; }
            CharKey { imgNormal: UI.imageGreyKey[formFactor]; imgPressed: UI.imageGreyKeyPressed[formFactor]; label: ","; shifted: "/"; extended:"/";}
            CharKey { imgNormal: UI.imageGreyKey[formFactor]; imgPressed: UI.imageGreyKeyPressed[formFactor]; label: "."; shifted: "?"; extended:"?";}
            ShiftKey {}
        }
        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: panel.keyHeight;

            TabKey         { id: tabKey; extended: "Tab";  shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
			SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC"; anchors.left: tabKey.right; }
            LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: apostropheKey.left; noMagnifier: true }
            CharKey        { imgNormal: UI.imageGreyKey[formFactor]; imgPressed: UI.imageGreyKeyPressed[formFactor]; id: apostropheKey; label: "'"; extended: "\""; shifted: "\""; anchors.right: minusKey.left; }
            CharKey        { imgNormal: UI.imageGreyKey[formFactor]; imgPressed: UI.imageGreyKeyPressed[formFactor]; id: minusKey;      label: "-"; extended: "_"; shifted: "_";  anchors.right: dismissKey.left; }
            DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
        }
       
    } // column
}
