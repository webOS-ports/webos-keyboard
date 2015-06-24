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
    symbols: "languages/Keyboard_symbols_tablet.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight * UI.topRowKeyHeightRatio

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
            TrackBall { width: keypadRoot.width - (UI.keyWidth*UI.numKeyWidthRatio*10); anchors.verticalCenter: parent.verticalCenter }
        }

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
			BackspaceKey { padding: 0 }
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
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; width: UI.keyWidth; fontSize: UI.isLandscape ? UI.xsFontSize : "12pt"}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey { width: UI.keyWidth * 1.5; }
            CharKey { label: "ئ"; }
            CharKey { label: "ء"; }
            CharKey { label: "ؤ"; }
            CharKey { label: "ر"; }
            CharKey { label: "ى"; shifted: "آ" }
            CharKey { label: "ة"; }
            CharKey { label: "و"; }
            CharKey { label: "ز"; }
            CharKey { label: "ظ"; }
			ShiftKey { width: UI.keyWidth * 1.5; }
            
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { id: commaKey;    label: "ذ"; shifted: "/";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; noMagnifier: true }
                CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: specialChar.left; }
                CharKey        { id: specialChar; label: "ط";                anchors.right: dismissKey.left }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; noMagnifier: true }
                UrlKey         { id: urlKey; label: ".eg"; extended: [".iq", ".lb", ".sa", ".sy", ".jo", ".ye"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: "ذ";  anchors.right: specialChar.left; }
                CharKey        { id: specialChar; label: "ط";                anchors.right: dismissKey.left }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                // note FIXME: full list of tld:
                // [".ma", ".dz", ".ly", ".tn", ".iq", ".lb", ".ps", ".jo", ".ye", ".bh", ".dj", ".kw", ".km", ".mr‎", ".om", ".qa", ".sa", ".sy", ".so‎", ".sd", ".ae"]
                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                            anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                CharKey        { id: slashKey; label: "/"; shifted: "/";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; noMagnifier: true }
                UrlKey         { id: urlKey; label: ".eg"; extended: [".ma", ".dz", ".ly", ".tn", ".iq", ".lb", ".ps", ".jo", ".ye", ".bh", ".dj", ".kw", ".km", ".mr‎", ".om", ".qa", ".sa", ".sy", ".so‎", ".sd", ".ae"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: "ذ";  anchors.right: specialChar.left; }
                CharKey        { id: specialChar; label: "ط";                anchors.right: dismissKey.left }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
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
