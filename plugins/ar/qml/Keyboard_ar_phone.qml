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
 * Weights follow PhoneKeymap.cpp: letters stay one unit wide so they line up
 * with the row above, shift and backspace take 1.25 with a quarter-unit pad
 * beside them, and the bottom row is Sym 1.5 / comma 1.5 / space / period 1.5 /
 * Enter 1.5. There is no language key and no hide key on this keyboard - the
 * shift key's symbol-layer identity is cKey_ToggleLanguage, so language
 * switching lives on the 123 page.
 */

import QtQuick 2.0
import keys 1.0

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_phone.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        // Letters at one unit  [sum 11]
        KeyRow {
            height: keyHeight

            CharKey { label: "ض" }
            CharKey { label: "ص" }
            CharKey { label: "ث" }
            CharKey { label: "ق" }
            CharKey { label: "ف" }
            CharKey { label: "غ"; shifted: "إ" }
            CharKey { label: "ع" }
            CharKey { label: "ه" }
            CharKey { label: "خ" }
            CharKey { label: "ح" }
            CharKey { label: "ج" }
        }

        // Letters at one unit  [sum 11]
        KeyRow {
            height: keyHeight

            CharKey { label: "ش" }
            CharKey { label: "س" }
            CharKey { label: "ي" }
            CharKey { label: "ب" }
            CharKey { label: "ل" }
            CharKey { label: "ا"; shifted: "أ" }
            CharKey { label: "ت" }
            CharKey { label: "ن" }
            CharKey { label: "م" }
            CharKey { label: "ك" }
            CharKey { label: "د" }
        }

        // Shift 0.75 + a quarter-unit pad, the letters at 1, then the same pad and Backspace 0.75  [sum 11]
        KeyRow {
            height: keyHeight

            ShiftKey { weight: 0.75 }
            SpacerKey { weight: 0.25; forwardTo: lowFirstKey }
            CharKey { id: lowFirstKey; label: "ئ" }
            CharKey { label: "ء" }
            CharKey { label: "ؤ" }
            CharKey { label: "ر" }
            CharKey { label: "ى"; shifted: "آ" }
            CharKey { label: "ة" }
            CharKey { label: "و" }
            CharKey { label: "ز" }
            CharKey { label: "ظ" }
            SpacerKey { weight: 0.25; forwardTo: backspaceKey }
            BackspaceKey { id: backspaceKey; weight: 0.75 }
        }

        // cCustom_*_plain  [sum 11]
        Component {
            id: contentTypeNormal
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                SpaceKey       { weight: 5 }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        // cCustom_*_email: @ and .com flank the space key  [sum 11]
        Component {
            id: contentTypeEmail
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                AnnotatedKey { weight: 1.5; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"] }
                UrlKey         { label: "@"; shifted: "@" }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".com"; shifted: ".com" }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        // cCustom_*_url: a plain slash, then a colon carrying the scheme prefixes  [sum 10.5]
        Component {
            id: contentTypeUrl
            KeyRow {
                height: keyHeight

                SymbolShiftKey { label: "123"; shifted: "123"; weight: 1.5 }
                UrlKey         { label: "/"; shifted: "/" }
                UrlKey         { label: ":"; shifted: ":"; extended: ["://", "http://", "https://"] }
                SpaceKey       { weight: 3 }
                UrlKey { label: ".com"; shifted: ".com" }
                AnnotatedKey { weight: 1.5; label: "."; shifted: "." }
                ReturnKey      { label: "Enter"; shifted: "Enter"; weight: 1.5 }
            }
        }

        Loader {
            width: parent.width
            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
