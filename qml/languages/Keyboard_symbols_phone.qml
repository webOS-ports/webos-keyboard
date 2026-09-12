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
 * The alternate layer of PhoneKeymap.cpp sQwerty: every key's altkey column, with
 * cCustom_QWERT_symbol on the bottom row. Same geometry as the letter page, so the
 * rows sum to 10 here too.
 *
 * The shift key's altkey is cKey_ToggleLanguage, which is why this page - and only
 * this page - carries the language key.
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

        // alt of QWERTY_TOP_10: q..p -> 1..0
        KeyRow {
            height: keyHeight

            CharKey { label: "1"; shifted: "1"; }
            CharKey { label: "2"; shifted: "2"; }
            CharKey { label: "3"; shifted: "3"; }
            CharKey { label: "4"; shifted: "4"; }
            CharKey { label: "5"; shifted: "5"; }
            CharKey { label: "6"; shifted: "6"; }
            CharKey { label: "7"; shifted: "7"; }
            CharKey { label: "8"; shifted: "8"; }
            CharKey { label: "9"; shifted: "9"; }
            CharKey { label: "0"; shifted: "0"; }
        }

        // alt of QWERTY_MID_9: a..l -> ! @ # $ % & * ( )
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: exclamKey }
            CharKey { id: exclamKey; label: "!"; shifted: "!"; }
            CharKey { label: "@"; shifted: "@"; }
            CharKey { label: "#"; shifted: "#"; }
            CharKey { label: "$"; shifted: "$"; }
            CharKey { label: "%"; shifted: "%"; }
            CharKey { label: "&"; shifted: "&"; }
            CharKey { label: "*"; shifted: "*"; }
            CharKey { label: "("; shifted: "("; }
            CharKey { id: parenRightKey; label: ")"; shifted: ")"; }
            SpacerKey { weight: 0.5; forwardTo: parenRightKey }
        }

        // alt of QWERTY_LOW_7: z..m -> ; : = + - ' " , and the shift position becomes
        // the language key.
        KeyRow {
            height: keyHeight

            LanguageKey { weight: 1.25 }
            SpacerKey { weight: 0.25; forwardTo: semicolonKey }
            CharKey { id: semicolonKey; label: ";"; shifted: ";"; }
            CharKey { label: ":"; shifted: ":"; }
            CharKey { label: "="; shifted: "="; }
            CharKey { label: "+"; shifted: "+"; }
            CharKey { label: "-"; shifted: "-"; }
            CharKey { label: "'"; shifted: "'"; }
            CharKey { label: "\""; shifted: "\""; }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 1.25 }
        }

        // cCustom_QWERT_symbol: cKey_Emoticon_Options before the space key,
        // cKey_MorePopup after it.
        KeyRow {
            height: keyHeight

            SymbolShiftKey { label: "ABC"; shifted: "ABC"; weight: 1.5 }
            CharKey { label: ":)"; shifted: ":)"; weight: 1.5
                      imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed
                      extended: [":-)", ";-)", ":-(", ":'(", ":-P", ":-O", "<3"]
                      extendedShifted: [":-)", ";-)", ":-(", ":'(", ":-P", ":-O", "<3"] }
            SpaceKey { weight: 4 }
            CharKey { label: "…"; shifted: "…"; weight: 1.5
                      imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed
                      extended: ["•", "…", "±", "¬", "¦", "µ", "¤"]
                      extendedShifted: ["•", "…", "±", "¬", "¦", "µ", "¤"] }
            ReturnKey { weight: 1.5 }
        }
    } // column
}
