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
    alternativeLayouts: [ "Thumb" ] // list of alternative layouts, like Dvorak, Bepo, Splitted...
	
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

            NumKey { label: "1"; shifted: "ё"; extended: ["1", "ё", "!", "¹", "¼", "½", "¡"]; width: UI.keyWidth;}
            NumKey { label: "2"; shifted: "@"; extended: ["2", "@", "²"]; width: UI.keyWidth;}
            NumKey { label: "3"; shifted: "№"; extended: ["3", "№", "#", "³", "¾"]; width: UI.keyWidth;}
            NumKey { label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; width: UI.keyWidth;}
            NumKey { label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; width: UI.keyWidth;}
            NumKey { label: "6"; shifted: "^"; extended: ["6", "^"]; width: UI.keyWidth;}
            NumKey { label: "7"; shifted: "&"; extended: ["7", "&"]; width: UI.keyWidth;}
            NumKey { label: "8"; shifted: "х"; extended: ["8", "х", "*"]; extendedShifted: ["8", "Х", "*"]; width: UI.keyWidth;}
            NumKey { label: "9"; shifted: "ъ"; extended: ["9", "ъ", "(", "[", "{"]; extendedShifted: ["9", "Ъ", "(", "[", "{"]; width: UI.keyWidth;}
            NumKey { label: "0"; shifted: "э"; extended: ["0", "э", ")", "]", "}"]; extendedShifted: ["0", "Э", ")", "]", "}"]; width: UI.keyWidth;}
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

            ShiftKey { width: UI.keyWidth * 0.95; }
            CharKey { label: "я"; shifted: "Я"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ч"; shifted: "Ч"; width: UI.keyWidth * 0.95;}
            CharKey { label: "с"; shifted: "С"; width: UI.keyWidth * 0.95;}
            CharKey { label: "м"; shifted: "М"; width: UI.keyWidth * 0.95;}
            CharKey { label: "и"; shifted: "И"; width: UI.keyWidth * 0.95;}
            CharKey { label: "т"; shifted: "Т"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ь"; shifted: "Ь"; width: UI.keyWidth * 0.95;}
            CharKey { label: "б"; shifted: "Б"; width: UI.keyWidth * 0.95;}
            CharKey { label: "ю"; shifted: "Ю"; width: UI.keyWidth * 0.95;}
			AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ",", "/", "\\"]; extendedShifted: [".", ",", "/", "\\"]; width: UI.keyWidth * 0.95;}
            ShiftKey { width: UI.keyWidth * 1.5; }
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab"; label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: emailKey;    label: "@"; shifted: "@";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: emailKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"]; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab"; label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"]; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
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
