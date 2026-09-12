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
 * tabletkeymaps/uk.h builds Ukrainian on the Russian keymap and leaves the four
 * Ukrainian letters reachable by long press only - a Ukrainian speaker gets a
 * Russian keyboard. This is the real layout instead: yeru is i, e is ie, and yi
 * closes the top row, with ghe-with-upturn a long press from ghe and the
 * apostrophe from soft sign. Row weights follow the reference idiom.
 */

import QtQuick 2.0
import keys 1.0

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_tablet.qml"
    alternativeLayouts: [ "Thumb" ]

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        // Digits + trackball  [sum 12]
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

        // Top row + Backspace  [sum 13.5]
        KeyRow {
            height: keyHeight

            CharKey { label: "й"; shifted: "Й" }
            CharKey { label: "ц"; shifted: "Ц" }
            CharKey { label: "у"; shifted: "У" }
            CharKey { label: "к"; shifted: "К" }
            CharKey { label: "е"; shifted: "Е" }
            CharKey { label: "н"; shifted: "Н" }
            CharKey { label: "г"; shifted: "Г"; extended: ["г", "ґ"]; extendedShifted: ["Г", "Ґ"] }
            CharKey { label: "ш"; shifted: "Ш" }
            CharKey { label: "щ"; shifted: "Щ" }
            CharKey { label: "з"; shifted: "З" }
            CharKey { label: "х"; shifted: "Х" }
            CharKey { label: "ї"; shifted: "Ї"; extended: ["ї", "і"]; extendedShifted: ["Ї", "І"] }
            BackspaceKey { weight: 1.5 }
        }

        // Half-key inset, home row, Return  [sum 13]
        KeyRow {
            height: keyHeight

            SpacerKey { weight: 0.5; forwardTo: homeFirstKey }
            CharKey { id: homeFirstKey; label: "ф"; shifted: "Ф" }
            CharKey { label: "і"; shifted: "І"; extended: ["і", "ї", "и"]; extendedShifted: ["І", "Ї", "И"] }
            CharKey { label: "в"; shifted: "В" }
            CharKey { label: "а"; shifted: "А" }
            CharKey { label: "п"; shifted: "П" }
            CharKey { label: "р"; shifted: "Р" }
            CharKey { label: "о"; shifted: "О" }
            CharKey { label: "л"; shifted: "Л" }
            CharKey { label: "д"; shifted: "Д" }
            CharKey { label: "ж"; shifted: "Ж" }
            CharKey { label: "є"; shifted: "Є"; extended: ["є", "е"]; extendedShifted: ["Є", "Е"] }
            ReturnKey { weight: 1.5; alignTextRight: true }
        }

        // Shift, bottom row, Shift  [sum 12.5]
        KeyRow {
            height: keyHeight

            ShiftKey { }
            CharKey { label: "я"; shifted: "Я" }
            CharKey { label: "ч"; shifted: "Ч" }
            CharKey { label: "с"; shifted: "С" }
            CharKey { label: "м"; shifted: "М" }
            CharKey { label: "и"; shifted: "И" }
            CharKey { label: "т"; shifted: "Т" }
            CharKey { label: "ь"; shifted: "Ь"; extended: ["ь", "’"]; extendedShifted: ["Ь", "’"] }
            CharKey { label: "б"; shifted: "Б" }
            CharKey { label: "ю"; shifted: "Ю" }
            AnnotatedKey { label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
            ShiftKey { weight: 1.5 }
        }

        // Bottom row, default field  [sum 11]
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButton.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButton }
                SpaceKey       { weight: 5 }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        // Bottom row, email field  [sum 11]
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonEmail.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonEmail }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 4 }
                UrlKey { label: ".ua"; shifted: ".ua" }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        // Bottom row, URL field  [sum 11]
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonUrl.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonUrl }
                UrlKey         { label: "/"; shifted: "/"; extended: ["http://", "https://", "www."] }
                SpaceKey       { weight: 4 }
                UrlKey { label: ".ua"; shifted: ".ua" }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        Loader {
            width: parent.width

            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
