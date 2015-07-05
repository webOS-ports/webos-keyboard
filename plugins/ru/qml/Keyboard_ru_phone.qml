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

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "й"; shifted: "Й"; }
            CharKey { label: "ц"; shifted: "Ц"; }
            CharKey { label: "у"; shifted: "Y"; } //extended: ["у", "ў"]; extendedShifted: ["Y", "Ў"]; }
            CharKey { label: "к"; shifted: "К"; }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "ë", "€"]; extendedShifted: ["E", "Ë", "€"]; }
            CharKey { label: "н"; shifted: "Н"; }
            CharKey { label: "г"; shifted: "Г"; }
            CharKey { label: "ш"; shifted: "Ш"; }
            CharKey { label: "щ"; shifted: "Щ"; }
            CharKey { label: "з"; shifted: "З"; }
            CharKey { label: "х"; shifted: "Х"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "ф"; shifted: "Ф"; }
            CharKey { label: "ы"; shifted: "Ы"; }
            CharKey { label: "в"; shifted: "В"; }
            CharKey { label: "a"; shifted: "A"; }
            CharKey { label: "п"; shifted: "П"; }
            CharKey { label: "р"; shifted: "Р"; }
            CharKey { label: "о"; shifted: "О"; }
            CharKey { label: "л"; shifted: "Л"; }
            CharKey { label: "д"; shifted: "Д"; }
            CharKey { label: "ж"; shifted: "Ж"; }
            CharKey { label: "э"; shifted: "Э"; }
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
            CharKey { label: "и"; shifted: "И"; extended: ["и", "і"]; extendedShifted: ["И", "І"]; }
            CharKey { label: "т"; shifted: "Т"; }
            CharKey { label: "ь"; shifted: "Ь"; extended: ["ь", "ъ"]; extendedShifted: ["Ь", "Ъ"]; }
            CharKey { label: "б"; shifted: "Б"; }
            CharKey { label: "ю"; shifted: "Ю"; }
            BackspaceKey { padding: 0 }
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                UrlKey        { id: commaKey;    label: ","; shifted: "/";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: commaKey.right; anchors.right: dotKey.left; }
                UrlKey        { id: dotKey;       label: "."; shifted: "."; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;  label: "Enter"; shifted: "Enter";                            anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
				UrlKey         { id: atKey;    label: "@"; shifted: "@";     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: atKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ru", ".ua", ".su", ".kg", ".рф", ".укр", ".by", ".tj"]; anchors.right: dotKey.left; }
                UrlKey         { id: dotKey;       label: "."; shifted: "."; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey; label: "Enter"; shifted: "Enter";                              anchors.right: parent.right }
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                SymbolShiftKey { id: symShiftKey;                            anchors.left: parent.left; }
                LanguageKey    { id: languageMenuButton;                     anchors.left: symShiftKey.right; }
                UrlKey         { id: slashKey; label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];     anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".ru"; extended: [".ru", ".ua", ".su", ".kg", ".рф", ".укр", ".by", ".tj"]; anchors.right: dotKey.left; }
                UrlKey         { id: dotKey;       label: "."; shifted: "."; anchors.right: enterKey.left; }
                ReturnKey      { id: enterKey;  label: "Enter"; shifted: "Enter";                             anchors.right: parent.right }
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
