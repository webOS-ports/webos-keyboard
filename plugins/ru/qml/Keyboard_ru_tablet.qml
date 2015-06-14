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
    id: keypadRoot
    
    content: c1
    symbols: "languages/Keyboard_symbols_tablet.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Row {
            //anchors.horizontalCenter: parent.horizontalCenter;
            anchors.left: parent.left;
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
            TrackBall { width: keypadRoot.width - (UI.keyWidth*10); anchors.verticalCenter: parent.verticalCenter }
        }
        
        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "й"; shifted: "Й"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ц"; shifted: "Ц"; width: UI.keyWidth * 0.95;}
            CharKey { label: "у"; shifted: "Y"; extended: ["ў"]; extendedShifted: ["Ў"];width: UI.keyWidth * 0.95; }
            CharKey { label: "к"; shifted: "К"; width: UI.keyWidth * 0.95;}
            CharKey { label: "e"; shifted: "E"; extended: ["ë", "€"]; extendedShifted: ["Ë", "€"];width: UI.keyWidth * 0.95; }
            CharKey { label: "н"; shifted: "Н"; width: UI.keyWidth * 0.95;}
            CharKey { label: "г"; shifted: "Г"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ш"; shifted: "Ш"; width: UI.keyWidth * 0.95;}
            CharKey { label: "щ"; shifted: "Щ"; width: UI.keyWidth * 0.95;}
            CharKey { label: "з"; shifted: "З"; width: UI.keyWidth * 0.95;}
            CharKey { label: "х"; shifted: "Х"; width: UI.keyWidth * 0.95;}
            BackspaceKey {width: UI.keyWidth * 1.5;}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "ф"; shifted: "Ф"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ы"; shifted: "Ы"; width: UI.keyWidth * 0.95;}
            CharKey { label: "в"; shifted: "В"; width: UI.keyWidth * 0.95;}
            CharKey { label: "a"; shifted: "A"; width: UI.keyWidth * 0.95;}
            CharKey { label: "п"; shifted: "П"; width: UI.keyWidth * 0.95;}
            CharKey { label: "р"; shifted: "Р"; width: UI.keyWidth * 0.95;}
            CharKey { label: "о"; shifted: "О"; width: UI.keyWidth * 0.95;}
            CharKey { label: "л"; shifted: "Л"; width: UI.keyWidth * 0.95;}
            CharKey { label: "д"; shifted: "Д"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ж"; shifted: "Ж"; width: UI.keyWidth * 0.95;}
            CharKey { label: "э"; shifted: "Э"; width: UI.keyWidth * 0.95;}
            ReturnKey      { id: enterKey; label: "Enter";  width: UI.keyWidth * 1.5;}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey { padding: 0 }
            CharKey { label: "я"; shifted: "Я"; }
            CharKey { label: "ч"; shifted: "Ч"; }
            CharKey { label: "с"; shifted: "С"; }
            CharKey { label: "м"; shifted: "М"; }
            CharKey { label: "і"; shifted: "І"; }
            CharKey { label: "и"; shifted: "И"; }
            CharKey { label: "т"; shifted: "Т"; }
            CharKey { label: "ь"; shifted: "Ь"; }
            CharKey { label: "б"; shifted: "Б"; }
            CharKey { label: "ю"; shifted: "Ю"; }
           // BackspaceKey { padding: 0 }
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                CharKey        { id: commaKey;    label: ","; shifted: "/";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; noMagnifier: true }
                CharKey        { id: dotKey;       label: "."; shifted: "."; anchors.right: extraCharKey.left; }
                CharKey        { id: extraCharKey; label: "ъ"; shifted: "Ъ"; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;                               anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                CharKey        { id: atKey;    label: "@"; shifted: "@";     anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; noMagnifier: true }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: extraCharKey.left; }
                CharKey        { id: extraCharKey; label: "ъ"; shifted: "Ъ"; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;                               anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                CharKey        { id: slashKey; label: "/"; shifted: "/";     anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; noMagnifier: true }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"]; anchors.right: dotKey.left; }
                CharKey        { id: dotKey;      label: "."; shifted: ".";  anchors.right: extraCharKey.left; }
                CharKey        { id: extraCharKey; label: "ъ"; shifted: "Ъ"; anchors.right: enterKey.left; }
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
