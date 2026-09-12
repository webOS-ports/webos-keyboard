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
 * Split layout for holding the tablet in both hands. There is no keymap for this in
 * the reference - webOS had no thumb keyboard - so it keeps the shape of the English
 * one: fixed thumb-reachable key widths rather than weights, each row split into a
 * half anchored to each edge with the gap in the middle.
 *
 * The alphabet is this language's, taken from its tablet layout.
 */

import QtQuick 2.0
import keys 1.0
import LunaNext.Common 0.1

KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_tablet_thumb.qml"

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Item {
            width: parent.width
            height: keyHeight / 2

            NumKey { id: numKey0; label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]; width: Units.gu(3.9); anchors.left: parent.left; thumbKeyboard: true }
            NumKey { id: numKey1; label: "2"; shifted: "@"; extended: ["2", "@", "²"]; width: Units.gu(3.9); anchors.left: numKey0.right; thumbKeyboard: true }
            NumKey { id: numKey2; label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"]; width: Units.gu(3.9); anchors.left: numKey1.right; thumbKeyboard: true }
            NumKey { id: numKey3; label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; width: Units.gu(3.9); anchors.left: numKey2.right; thumbKeyboard: true }
            NumKey { id: numKey4; label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; width: Units.gu(3.9); anchors.left: numKey3.right; thumbKeyboard: true }
            NumKey { id: numKey5; label: "6"; shifted: "^"; extended: ["6", "^"]; width: Units.gu(3.9); anchors.right: numKey6.left; thumbKeyboard: true }
            NumKey { id: numKey6; label: "7"; shifted: "&"; extended: ["7", "&"]; width: Units.gu(3.9); anchors.right: numKey7.left; thumbKeyboard: true }
            NumKey { id: numKey7; label: "8"; shifted: "*"; extended: ["8", "*"]; width: Units.gu(3.9); anchors.right: numKey8.left; thumbKeyboard: true }
            NumKey { id: numKey8; label: "9"; shifted: "("; extended: ["9", "(", "[", "{"]; width: Units.gu(3.9); anchors.right: numKey9.left; thumbKeyboard: true }
            NumKey { id: numKey9; label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"]; width: Units.gu(3.9); anchors.right: parent.right; thumbKeyboard: true }
        }

        Item {
            width: parent.width
            height: keyHeight * 2/3

            CharKey { id: topKey0; label: "q"; shifted: "Q"; width: Units.gu(3.9); anchors.left: parent.left; thumbKeyboard: true }
            CharKey { id: topKey1; label: "w"; shifted: "W"; width: Units.gu(3.9); anchors.left: topKey0.right; thumbKeyboard: true }
            CharKey { id: topKey2; label: "e"; shifted: "E"; extended: ["è","é","ë","ê","€"]; extendedShifted: ["È","É", "Ë","Ê","€"]; width: Units.gu(3.9); anchors.left: topKey1.right; thumbKeyboard: true }
            CharKey { id: topKey3; label: "r"; shifted: "R"; width: Units.gu(3.9); anchors.left: topKey2.right; thumbKeyboard: true }
            CharKey { id: topKey4; label: "t"; shifted: "T"; extended: ["þ"]; extendedShifted: ["Þ"]; width: Units.gu(3.9); anchors.left: topKey3.right; thumbKeyboard: true }
            CharKey { id: topKey5; label: "y"; shifted: "Y"; extended: ["¥"]; extendedShifted: ["¥"]; width: Units.gu(3.9); anchors.right: topKey6.left; thumbKeyboard: true }
            CharKey { id: topKey6; label: "u"; shifted: "U"; extended: ["ù","ü","û","ú"]; extendedShifted: ["Ù","Ü","Ú","Û"]; width: Units.gu(3.9); anchors.right: topKey7.left; thumbKeyboard: true }
            CharKey { id: topKey7; label: "i"; shifted: "I"; extended: ["ì","î","ï","í"]; extendedShifted: ["Ì","Î","Ï","Í"]; width: Units.gu(3.9); anchors.right: topKey8.left; thumbKeyboard: true }
            CharKey { id: topKey8; label: "o"; shifted: "O"; extended: ["ò","º","ó","ö","ô","õ","ø"]; extendedShifted: ["Ò","º","Ó","Ö","Ô","Õ","Ø"]; width: Units.gu(3.9); anchors.right: topKey9.left; thumbKeyboard: true }
            CharKey { id: topKey9; label: "p"; shifted: "P"; width: Units.gu(3.9); anchors.right: parent.right; thumbKeyboard: true }
        }

        Item {
            width: parent.width
            height: keyHeight * 2/3

            CharKey { id: midKey0; label: "a"; shifted: "A"; extended: ["à","ª","ä","á","â","ã",]; extendedShifted: ["À","ª","Ä","Á","Â","Ã"]; width: Units.gu(3.9); anchors.left: parent.left; thumbKeyboard: true }
            CharKey { id: midKey1; label: "s"; shifted: "S"; extended: ["ß","$"]; extendedShifted: ["$"]; width: Units.gu(3.9); anchors.left: midKey0.right; thumbKeyboard: true }
            CharKey { id: midKey2; label: "d"; shifted: "D"; width: Units.gu(3.9); anchors.left: midKey1.right; thumbKeyboard: true }
            CharKey { id: midKey3; label: "f"; shifted: "F"; width: Units.gu(3.9); anchors.left: midKey2.right; thumbKeyboard: true }
            CharKey { id: midKey4; label: "g"; shifted: "G"; width: Units.gu(3.9); anchors.left: midKey3.right; thumbKeyboard: true }
            CharKey { id: midKey5; label: "h"; shifted: "H"; width: Units.gu(3.9); anchors.right: midKey6.left; thumbKeyboard: true }
            CharKey { id: midKey6; label: "j"; shifted: "J"; width: Units.gu(3.9); anchors.right: midKey7.left; thumbKeyboard: true }
            CharKey { id: midKey7; label: "k"; shifted: "K"; width: Units.gu(3.9); anchors.right: midKey8.left; thumbKeyboard: true }
            CharKey { id: midKey8; label: "l"; shifted: "L"; width: Units.gu(3.9); anchors.right: parent.right; thumbKeyboard: true }
        }

        Item {
            width: parent.width
            height: keyHeight * 2/3

            CharKey { id: lowKey0; label: "z"; shifted: "Z"; width: Units.gu(3.9); anchors.left: parent.left; thumbKeyboard: true }
            CharKey { id: lowKey1; label: "x"; shifted: "X"; width: Units.gu(3.9); anchors.left: lowKey0.right; thumbKeyboard: true }
            CharKey { id: lowKey2; label: "c"; shifted: "C"; extended: ["ç"]; extendedShifted: ["Ç"]; width: Units.gu(3.9); anchors.left: lowKey1.right; thumbKeyboard: true }
            CharKey { id: lowKey3; label: "v"; shifted: "V"; width: Units.gu(3.9); anchors.left: lowKey2.right; thumbKeyboard: true }
            CharKey { id: lowKey4; label: "b"; shifted: "B"; width: Units.gu(3.9); anchors.left: lowKey3.right; thumbKeyboard: true }
            CharKey { id: lowKey5; label: "n"; shifted: "N"; extended: ["ñ"]; extendedShifted: ["Ñ"]; width: Units.gu(3.9); anchors.right: lowKey6.left; thumbKeyboard: true }
            CharKey { id: lowKey6; label: "m"; shifted: "M"; width: Units.gu(3.9); anchors.right: lowKey7.left; thumbKeyboard: true }
            AnnotatedKey { id: lowKey7; label: ","; shifted: "/"; extended: [",", "/", "\\"]; extendedShifted: [",", "/", "\\"]; width: Units.gu(3.9); anchors.right: lowKey8.left; thumbKeyboard: true }
            AnnotatedKey { id: lowKey8; label: "."; shifted: "?"; extended: [".", "?", "•", "…", "¿"]; extendedShifted: [".", "?", "•", "…", "¿"]; width: Units.gu(3.9); anchors.right: lowKey9.left; thumbKeyboard: true }
            BackspaceKey { id: lowKey9; width: Units.gu(3.9); anchors.right: parent.right; thumbKeyboard: true }
        }

        Component {
            id: contentTypeNormal
            Item {
                width: parent.width
                height: keyHeight * 2/3

                ShiftKey       { id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true }
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true }
                LanguageKey    { id: languageMenuButton; width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true }
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey1; anchors.left: minusKey.right; anchors.right: tabKey1.left; thumbKeyboard: true }
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.right: symShiftKey.left; thumbKeyboard: true }
                SymbolShiftKey { id: symShiftKey; anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true }
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.left: symShiftKey.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey2; anchors.left: tabKey2.right; anchors.right: dotKey.left; thumbKeyboard: true }
                AnnotatedKey   { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowLeftKey; label: "\u2190"; shifted: "\u2190"; action: "keyLeft"; width: Units.gu(3.9); anchors.right: arrowRightKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowRightKey; label: "\u2192"; shifted: "\u2192"; action: "keyRight"; width: Units.gu(3.9); anchors.right: enterKey.left; thumbKeyboard: true }
                ReturnKey      { id: enterKey; alignTextRight: true; width: Units.gu(7.8); anchors.right: parent.right; thumbKeyboard: true }
            }
        }
        Component {
            id: contentTypeEmail
            Item {
                width: parent.width
                height: keyHeight * 2/3

                ShiftKey       { id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true }
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true }
                LanguageKey    { id: languageMenuButton; width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true }
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true }
                UrlKey         { id: emailKey; width: Units.gu(3.9); label: "@"; shifted: "@"; anchors.left: minusKey.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey1; anchors.left: emailKey.right; anchors.right: tabKey1.left; thumbKeyboard: true }
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.right: symShiftKey.left; thumbKeyboard: true }
                SymbolShiftKey { id: symShiftKey; anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true }
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.left: symShiftKey.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey2; anchors.left: tabKey2.right; anchors.right: urlKey.left; thumbKeyboard: true }
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: ".com"; shifted: ".com"; anchors.right: dotKey.left; thumbKeyboard: true }
                AnnotatedKey   { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowLeftKey; label: "\u2190"; shifted: "\u2190"; action: "keyLeft"; width: Units.gu(3.9); anchors.right: arrowRightKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowRightKey; label: "\u2192"; shifted: "\u2192"; action: "keyRight"; width: Units.gu(3.9); anchors.right: enterKey.left; thumbKeyboard: true }
                ReturnKey      { id: enterKey; alignTextRight: true; width: Units.gu(7.8); anchors.right: parent.right; thumbKeyboard: true }
            }
        }
        Component {
            id: contentTypeUrl
            Item {
                width: parent.width
                height: keyHeight * 2/3

                ShiftKey       { id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true }
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true }
                LanguageKey    { id: languageMenuButton; width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true }
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true }
                UrlKey         { id: slashKey; width: Units.gu(3.9); label: "www."; shifted: "https://"; extended: ["www.", "https://", "http://"]; anchors.left: minusKey.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey1; anchors.left: slashKey.right; anchors.right: tabKey1.left; thumbKeyboard: true }
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.right: symShiftKey.left; thumbKeyboard: true }
                SymbolShiftKey { id: symShiftKey; anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true }
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "Tab"; shifted: "Tab"; anchors.left: symShiftKey.right; thumbKeyboard: true }
                SpaceKey       { id: spaceKey2; anchors.left: tabKey2.right; anchors.right: urlKey.left; thumbKeyboard: true }
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: ".com"; shifted: ".com"; anchors.right: dotKey.left; thumbKeyboard: true }
                AnnotatedKey   { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowLeftKey; label: "\u2190"; shifted: "\u2190"; action: "keyLeft"; width: Units.gu(3.9); anchors.right: arrowRightKey.left; thumbKeyboard: true }
                ActionKey      { id: arrowRightKey; label: "\u2192"; shifted: "\u2192"; action: "keyRight"; width: Units.gu(3.9); anchors.right: enterKey.left; thumbKeyboard: true }
                ReturnKey      { id: enterKey; alignTextRight: true; width: Units.gu(7.8); anchors.right: parent.right; thumbKeyboard: true }
            }
        }

        Loader {
            width: parent.width

            sourceComponent: currentContentType === 0 ? contentTypeNormal :
                             currentContentType === 3 ? contentTypeEmail : contentTypeUrl
        }
    } // column
}
