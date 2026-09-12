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
 * Weights follow tabletkeymaps/uk.h, sUkQwertyLayout. Row sums: 12 / 12.5 / 12.5 / 12.5 / 11.
 * The reference builds Ukrainian on the Russian base and reaches the four
 * Ukrainian letters through long-press only: yi behind short i, ghe-upturn
 * behind ghe, i behind yeru and ie behind e. Ported as-is for parity; a native
 * layout would put them on the main rows in place of yeru, e and hard sign.
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

        // UK_QWERTY_NUMBERS(1) + KEY_1(2, cKey_Trackball)  [sum 12]
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

        // UK_QWERTY_TOP(1) + KEY_1(1.5, Backspace)  [sum 12.5]
        KeyRow {
            height: keyHeight

            CharKey { label: "й"; shifted: "Й"; extended: ["й", "ї"]; extendedShifted: ["Й", "Ї"] }
            CharKey { label: "ц"; shifted: "Ц" }
            CharKey { label: "у"; shifted: "У"; extended: ["ў"]; extendedShifted: ["Ў"] }
            CharKey { label: "к"; shifted: "К" }
            CharKey { label: "е"; shifted: "Е"; extended: ["е", "ё"]; extendedShifted: ["Е", "Ё"] }
            CharKey { label: "н"; shifted: "Н" }
            CharKey { label: "г"; shifted: "Г"; extended: ["г", "ґ"]; extendedShifted: ["Г", "Ґ"] }
            CharKey { label: "ш"; shifted: "Ш" }
            CharKey { label: "щ"; shifted: "Щ" }
            CharKey { label: "з"; shifted: "З" }
            CharKey { label: "х"; shifted: "Х" }
            BackspaceKey { weight: 1.5 }
        }

        // UK_QWERTY_MID(1) + KEY_1(1.5, Return)  [sum 12.5]
        KeyRow {
            height: keyHeight

            CharKey { label: "ф"; shifted: "Ф" }
            CharKey { label: "ы"; shifted: "Ы"; extended: ["ы", "і"]; extendedShifted: ["Ы", "І"] }
            CharKey { label: "в"; shifted: "В" }
            CharKey { label: "а"; shifted: "А" }
            CharKey { label: "п"; shifted: "П" }
            CharKey { label: "р"; shifted: "Р" }
            CharKey { label: "о"; shifted: "О" }
            CharKey { label: "л"; shifted: "Л" }
            CharKey { label: "д"; shifted: "Д" }
            CharKey { label: "ж"; shifted: "Ж" }
            CharKey { label: "э"; shifted: "Э"; extended: ["э", "є"]; extendedShifted: ["Э", "Є"] }
            ReturnKey { weight: 1.5; label: "Enter"; shifted: "Enter"; alignTextRight: true }
        }

        // KEY_1(1, Shift) + UK_QWERTY_LOW(1) + KEY_1(1.5, Shift)  [sum 12.5]
        KeyRow {
            height: keyHeight

            ShiftKey { }
            CharKey { label: "я"; shifted: "Я" }
            CharKey { label: "ч"; shifted: "Ч" }
            CharKey { label: "с"; shifted: "С" }
            CharKey { label: "м"; shifted: "М" }
            CharKey { label: "и"; shifted: "И" }
            CharKey { label: "т"; shifted: "Т" }
            CharKey { label: "ь"; shifted: "Ь"; extended: ["ь", "ъ"]; extendedShifted: ["Ь", "Ъ"] }
            CharKey { label: "б"; shifted: "Б" }
            CharKey { label: "ю"; shifted: "Ю" }
            AnnotatedKey { label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"] }
            ShiftKey { weight: 1.5 }
        }

        // Bottom row, default field. cKey_Symbol is 2 units and gives up its right half
        // only when updateLanguageKey() has a language to show.
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

        // Bottom row, email field: space drops to SPACE_SIZE - 2.
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonEmail.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonEmail }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".ua"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"] }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
            }
        }

        // Bottom row, URL field.
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                TabKey         { label: "Tab"; shifted: "Tab" }
                SymbolShiftKey { weight: languageMenuButtonUrl.visible ? 1 : 2 }
                LanguageKey    { id: languageMenuButtonUrl }
                CharKey { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; label: "/"; shifted: "/"; extended: ["http://", "https://", "www."] }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".ua"; extended: [".ua",".su",".kg",".рф","укр",".by",".tj"] }
                AnnotatedKey { label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"] }
                AnnotatedKey { label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"] }
                DismissKey     { }
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
