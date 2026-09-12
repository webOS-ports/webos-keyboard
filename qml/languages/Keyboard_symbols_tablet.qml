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

/*
 * The alternate layer of tabletkeymaps/en.h sUsQwertyLayout: every key's altkey
 * column. Geometry is identical to the letter page - rows sum to 12 / 11 / 11 / 11 / 11.
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
        anchors.margins: 0

        spacing: 0

        KeyRow {
            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"] }
            NumKey { label: "2"; shifted: "@"; extended: ["2", "@", "²"] }
            NumKey { label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"] }
            NumKey { label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"] }
            NumKey { label: "5"; shifted: "%"; extended: ["5", "%", "‰"] }
            NumKey { label: "6"; shifted: "^"; extended: ["6", "^"] }
            NumKey { label: "7"; shifted: "&"; extended: ["7", "&"] }
            NumKey { label: "8"; shifted: "*"; extended: ["8", "*"] }
            NumKey { label: "9"; shifted: "("; extended: ["9", "(", "[", "{"] }
            NumKey { label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"] }
            TrackBall { }
        }

        // alt of US_QWERTY_TOP_10
        KeyRow {
            height: keyHeight

            CharKey { label: "`"; shifted: "`"; }
            CharKey { label: "~"; shifted: "~"; }
            CharKey { label: "€"; shifted: "€"; }
            CharKey { label: "£"; shifted: "£"; }
            CharKey { label: "\\"; shifted: "\\"; }
            CharKey { label: "|"; shifted: "|"; }
            CharKey { label: "{"; shifted: "{"; }
            CharKey { label: "}"; shifted: "}"; }
            CharKey { label: "["; shifted: "["; }
            CharKey { label: "]"; shifted: "]"; }
            BackspaceKey { }
        }

        // alt of US_QWERTY_MID_9
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: lessKey }
            CharKey { id: lessKey; label: "<"; shifted: "<"; }
            CharKey { label: ">"; shifted: ">"; }
            CharKey { label: "="; shifted: "="; }
            CharKey { label: "+"; shifted: "+"; }
            CharKey { label: "×"; shifted: "×"; }
            CharKey { label: "÷"; shifted: "÷"; }
            CharKey { label: "°"; shifted: "°"; }
            CharKey { label: ";"; shifted: ";"; }
            CharKey { label: ":"; shifted: ":"; }
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; }
        }

        // alt of US_QWERTY_LOW_9: the seven webOS emoticons, then comma and period
        KeyRow {
            height: keyHeight

            ShiftKey { }
            CharKey { label: ":-)"; shifted: ":-)"; }
            CharKey { label: ";-)"; shifted: ";-)"; }
            CharKey { label: ":-("; shifted: ":-("; }
            CharKey { label: ":'("; shifted: ":'("; }
            CharKey { label: ":-P"; shifted: ":-P"; }
            CharKey { label: ":-O"; shifted: ":-O"; }
            CharKey { label: "<3"; shifted: "<3"; }
            AnnotatedKey { label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; }
            AnnotatedKey { label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; }
            ShiftKey { }
        }

        KeyRow {
            height: keyHeight

            TabKey         { label: "Tab"; shifted: "Tab"; }
            SymbolShiftKey { label: "ABC"; shifted: "ABC"; weight: languageMenuButton.visible ? 1 : 2 }
            LanguageKey    { id: languageMenuButton }
            SpaceKey       { weight: 5 }
            AnnotatedKey   { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; }
            AnnotatedKey   { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; }
            DismissKey     { }
        }
    } // column
}
