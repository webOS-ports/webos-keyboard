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
            anchors.right: parent.right;
            spacing: 0

            height: keyHeight;

            CharKey { label: "/"; shifted: "Q"; }
            CharKey { label: "'"; shifted: "W"; }
            CharKey { label: "ק"; shifted: "E"; }
            CharKey { label: "ר"; shifted: "R"; }
            CharKey { label: "א"; shifted: "T"; }
            CharKey { label: "ט"; shifted: "Y"; }
            CharKey { label: "ו"; shifted: "U"; }
            CharKey { label: "ן"; shifted: "I"; }
            CharKey { label: "ם"; shifted: "O"; }
            CharKey { label: "פ"; shifted: "P"; }
            CharKey { label: "]"; shifted: "}"; }
            CharKey { label: "["; shifted: "{"; }
            BackspaceKey { }
        }

        Row {
            anchors.right: parent.right;
            spacing: 0

            height: keyHeight;

            CharKey { label: "ש"; shifted: "A"; }
            CharKey { label: "ד"; shifted: "S"; }
            CharKey { label: "ג"; shifted: "D"; }
            CharKey { label: "כ"; shifted: "F"; }
            CharKey { label: "ע"; shifted: "G"; }
            CharKey { label: "י"; shifted: "H"; }
            CharKey { label: "ח"; shifted: "J"; }
            CharKey { label: "ל"; shifted: "K"; }
            CharKey { label: "ך"; shifted: "L"; }
            CharKey { label: "ף"; shifted: ":"; }
            CharKey { label: ","; shifted: "\""; }
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; width: UI.keyWidth * 1.5; fontSize: UI.isLandscape ? UI.xsFontSize : "12pt"}
        }

        Row {
            anchors.right: parent.right;
            spacing: 0

            height: keyHeight;

            ShiftKey { width: UI.keyWidth * 1.0; }
            CharKey { label: "ז"; shifted: "Z"; }
            CharKey { label: "ס"; shifted: "X"; }
            CharKey { label: "ב"; shifted: "C"; }
            CharKey { label: "ה"; shifted: "V"; }
            CharKey { label: "נ"; shifted: "B"; }
            CharKey { label: "מ"; shifted: "N"; }
            CharKey { label: "צ"; shifted: "M"; }
            CharKey { label: "ת"; shifted: "<"; }
            CharKey { label: "ץ"; shifted: ">"; }
            CharKey { label: "."; shifted: "?"; }
            ShiftKey { width: UI.keyWidth * 2.0; }
            
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey;                 label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: languageMenuButton.right; anchors.right: dismissKey.left; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey;                 label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                UrlKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".il";                   anchors.right: dismissKey.left; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                TabKey         { id: tabKey;                 label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".il";                   anchors.right: dismissKey.left; }
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
