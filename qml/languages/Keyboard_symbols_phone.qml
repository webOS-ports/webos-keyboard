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

            CharKey { label: "1"; /*shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]*/	}
            CharKey { label: "2"; /*shifted: "@"; extended: ["2", "@", "²"]*/}
            CharKey { label: "3"; /*shifted: "#"; extended: ["3", "#", "³", "¾"]*/}
            CharKey { label: "4"; /*shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]*/}
            CharKey { label: "5"; /*shifted: "%"; extended: ["5", "%", "‰"]*/}
            CharKey { label: "6"; /*shifted: "^"; extended: ["6", "^"]*/}
            CharKey { label: "7"; /*shifted: "&"; extended: ["7", "&"]*/}
            CharKey { label: "8"; /*shifted: "*"; extended: ["8", "*"]*/}
            CharKey { label: "9"; /*shifted: "("; extended: ["9", "(", "[", "{"]*/}
            CharKey { label: "0"; /*shifted: ")"; extended: ["0", ")", "]", "}"]*/}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
           // anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "!";}
            CharKey { label: "@";}
            CharKey { label: "#";}
            CharKey { label: "$";}
			CharKey { label: "%";}
            CharKey { label: "&";}
            CharKey { label: "*";}
            CharKey { label: "(";}
            CharKey { label: ")";}
        }

        Item {
            //anchors.left: parent.left
			//anchors.horizontalCenter: parent.horizontalCenter;
            //anchors.margins: 50;
            //spacing: 0
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;
			
			ShiftKey {id: shiftKey; anchors.left: parent.left;}
            CharKey {width: UI.keyWidth * 8/7; id: semicolonKey; anchors.left: shiftKey.right; label: ";"; }
            CharKey {width: UI.keyWidth * 8/7; id: colonKey; anchors.left: semicolonKey.right; label: ":"; }
            CharKey {width: UI.keyWidth * 8/7; id: equalKey; anchors.left: colonKey.right; label: "="; }
            CharKey {width: UI.keyWidth * 8/7; id: plusKey; anchors.left: equalKey.right; label: "+"; }
            CharKey {width: UI.keyWidth * 8/7; id: minusKey; anchors.left: plusKey.right; label: "-"; }
            CharKey {width: UI.keyWidth * 8/7; id: apostropheKeyM; anchors.left: minusKey.right; label: "'"; }
            CharKey {width: UI.keyWidth * 8/7; id: quoteKey; anchors.left: apostropheKeyM.right; label: "\""; }

            BackspaceKey {id: backspaceKey; anchors.right: parent.right; anchors.left: quoteKey.right 	}

        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight;

			SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC"; anchors.left: parent.left; }
            LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: smileyKey.left; noMagnifier: true }
            //CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: apostropheKey; label: "'"; extended: "\""; shifted: "\""; anchors.right: minusKey.left; }
            CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: smileyKey;      label: "..."; /*extended: "_"; shifted: "_";*/  anchors.right: enterKey.left; }
            ReturnKey      { id: enterKey;      label: "Enter"; shifted: "Enter"; extended: "Enter";  anchors.right: parent.right;}
	        }
       
    } // column
}
