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
            TrackBall { width: c1.width - (UI.keyWidth*UI.numKeyWidthRatio*10); anchors.verticalCenter: parent.verticalCenter }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight

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
            spacing: 0

            height: keyHeight

            CharKey { label: "<"; shifted: "<";}
            CharKey { label: ">"; shifted: ">";}
            CharKey { label: "="; shifted: "=";}
            CharKey { label: "+"; shifted: "+";}
            CharKey { label: "×"; shifted: "×"; }
            CharKey { label: "÷"; shifted: "÷"; }
            CharKey { label: "°"; shifted: "°"; }
            CharKey { label: ";"; shifted: ";"; }
            CharKey { label: ":"; shifted: ":"; }
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true;}

        }

		Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight

			ShiftKey {}
            CharKey { label: ":-)"; shifted: ":-)";}
            CharKey { label: ";-)"; shifted: ";-)";}
            CharKey { label: ":-("; shifted: ":-(";}
            CharKey { label: ":'("; shifted: ":'(";}
            CharKey { label: ":-P"; shifted: ":-P"; }
            CharKey { label: ":-O"; shifted: ":-O"; }
            CharKey { label: "<3"; shifted: "<3"; }
            AnnotatedKey { label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            AnnotatedKey { label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            ShiftKey {}
        }
        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight

            TabKey         { id: tabKey; shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
			SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC"; anchors.left: tabKey.right; }
            LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
            SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: apostropheKey.left; }
            AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
        }
       
    } // column
}
