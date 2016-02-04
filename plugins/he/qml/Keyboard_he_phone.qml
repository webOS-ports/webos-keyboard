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
    content: c1
    symbols: "languages/Keyboard_symbols_phone.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        //Row {
        Item {
            //anchors.horizontalCenter: parent.horizontalCenter
            anchors.right: parent.right;
            anchors.left: parent.left;
            //spacing: 0

            height: keyHeight;

            CharKey { id: leftEmptyKey; width: UI.keyWidth * 0.5; anchors.left: parent.left; }
            CharKey { id: qKey; label: "/"; shifted: "Q"; anchors.left: leftEmptyKey.right /*parent.left*/; width: UI.keyWidth * 11/10; }
            CharKey { id: wKey; label: "'"; shifted: "W"; anchors.left: qKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: eKey; label: "ק"; shifted: "E"; anchors.left: wKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: rKey; label: "ר"; shifted: "R"; anchors.left: eKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: tKey; label: "א"; shifted: "T"; anchors.left: rKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: yKey; label: "ט"; shifted: "Y"; anchors.left: tKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: uKey; label: "ו"; shifted: "U"; anchors.left: yKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: iKey; label: "ן"; shifted: "I"; anchors.left: uKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: oKey; label: "ם"; shifted: "O"; anchors.left: iKey.right; width: UI.keyWidth * 11/10; }
            CharKey { id: pKey; label: "פ"; shifted: "P"; anchors.left: oKey.right; width: UI.keyWidth * 11/10; /*anchors.right: parent.right;*/ }
            CharKey { id: rightEmptyKey; width: UI.keyWidth * 0.5; anchors.left: pKey.right; anchors.right: parent.right; }

        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
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
        }

        Item {
            anchors.right: parent.right;
            anchors.left: parent.left;

            height: keyHeight;

            ShiftKey {id: shiftKey; /*width: UI.keyWidth * 1;*/ anchors.left: parent.left; anchors.right: zKey.left; }
            CharKey { id: zKey; label: "ז"; shifted: "Z"; anchors.left: shiftKey.right; width: UI.keyWidth * 10/9; }
            CharKey { id: xKey; label: "ס"; shifted: "X"; anchors.left: zKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: cKey; label: "ב"; shifted: "C"; anchors.left: xKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: vKey; label: "ה"; shifted: "V"; anchors.left: cKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: bKey; label: "נ"; shifted: "B"; anchors.left: vKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: nKey; label: "מ"; shifted: "N"; anchors.left: bKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: mKey; label: "צ"; shifted: "M"; anchors.left: nKey.right; width: UI.keyWidth * 10/9;}
            CharKey { id: questionKey; label: "ת"; shifted: ";"; anchors.left: mKey.right; width: UI.keyWidth * 10/9;}
            BackspaceKey { id: backspaceKey; /*width: UI.keyWidth * 1;*/ anchors.right: parent.right; anchors.left: questionKey.right; }
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                AnnotatedKey   { id: commaKey; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; anchors.left: languageMenuButton.right}
                SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; }
                AnnotatedKey   { id: dotKey; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; anchors.right: dismissKey.left}
                DismissKey     { id: dismissKey;                                anchors.right: enterKey.left}
                ReturnKey      { id: enterKey;    label: "Enter"; shifted: "Enter"; anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: atKey;    label: "@"; shifted: "@";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".il"; extended: [".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey   { id: dotKey;      label: "."; shifted: "?"; showAnnotation2: false;  anchors.right: dismissKey.left; }
                DismissKey     { id: dismissKey;                                anchors.right: enterKey.left}
                ReturnKey      { id: enterKey; label: "Enter"; shifted: "Enter"; anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey; label: "123"; shifted: "123"; anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey        { id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."]; anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".il"; extended: [".com", ".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey   { id: dotKey;      label: "."; shifted: "?"; showAnnotation2: false;  anchors.right: dismissKey.left; }
                DismissKey     { id: dismissKey;                                anchors.right: enterKey.left}
                ReturnKey      { id: enterKey;    label: "Enter"; shifted: "Enter"; anchors.right: parent.right }
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
