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
import LunaNext.Common 0.1


KeyPad {
    id: keypadRoot

    content: c1
    symbols: "languages/Keyboard_symbols_tablet_thumb.qml"
    alternativeLayouts: [ "Thumb" ] // list of alternative layouts, like Dvorak, Bepo, Splitted...

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

		Item {
			height: keyHeight / 2 
			anchors.left: parent.left
            anchors.right: parent.right
            NumKey { id: oneKey; label: "1"; shifted: "ё"; extended: ["1", "ё", "!", "¹", "¼", "½", "¡"]; anchors.left: parent.left; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: twoKey; label: "2"; shifted: "@"; extended: ["2", "@", "²"]; anchors.left: oneKey.right; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: threeKey; label: "3"; shifted: "№"; extended: ["3", "№", "#", "³", "¾"]; anchors.left: twoKey.right; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: fourKey; label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; anchors.left: threeKey.right; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: fiveKey; label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; anchors.left: fourKey.right; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: sixKey; label: "6"; shifted: "^"; extended: ["6", "^"]; anchors.right: sevenKey.left; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: sevenKey; label: "7"; shifted: "&"; extended: ["7", "&"]; anchors.right: eightKey.left; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: eightKey; label: "8"; shifted: "х"; extended: ["8", "х", "*"]; extendedShifted: ["8", "Х", "*"]; anchors.right: nineKey.left; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: nineKey; label: "9"; shifted: "ъ"; extended: ["9", "ъ", "(", "[", "{"]; extendedShifted: ["9", "Ъ", "(", "[", "{"]; anchors.right: zeroKey.left; width: Units.gu(3.9); thumbKeyboard: true; }
            NumKey { id: zeroKey; label: "0"; shifted: "э"; extended: ["0", "э", ")", "]", "}"]; extendedShifted: ["0", "Э", ")", "]", "}"]; anchors.right: parent.right; width: Units.gu(3.9); thumbKeyboard: true; }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight * 2/3

            CharKey { id: qKey; width: Units.gu(3.9); label: "й"; shifted: "Й"; anchors.left: parent.left; thumbKeyboard: true;}
            CharKey { id: wKey; width: Units.gu(3.9); label: "ц"; shifted: "Ц"; anchors.left: qKey.right; thumbKeyboard: true;}
            CharKey { id: eKey; width: Units.gu(3.9); label: "у"; shifted: "У"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€"]; anchors.left: wKey.right; thumbKeyboard: true;}
            CharKey { id: rKey; width: Units.gu(3.9); label: "к"; shifted: "К"; extended: ["r", "®"]; extendedShifted: ["R","®"]; anchors.left: eKey.right; thumbKeyboard: true;}
            CharKey { id: tKey; width: Units.gu(3.9); label: "е"; shifted: "Е"; extended: ["t", "™", "þ"]; extendedShifted: ["T", "™", "Þ"]; anchors.left: rKey.right; thumbKeyboard: true;}
            CharKey { id: yKey; width: Units.gu(3.9); label: "н"; shifted: "Н"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; anchors.right: uKey.left; thumbKeyboard: true;}
            CharKey { id: uKey; width: Units.gu(3.9); label: "г"; shifted: "Г"; extended: ["u", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Ù","Ú","Û", "Ü", "Ű"]; anchors.right: iKey.left; thumbKeyboard: true;}
            CharKey { id: iKey; width: Units.gu(3.9); label: "ш"; shifted: "Ш"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; anchors.right: oKey.left; thumbKeyboard: true;}
            CharKey { id: oKey; width: Units.gu(3.9); label: "щ"; shifted: "Щ"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; anchors.right: pKey.left; thumbKeyboard: true;}
            CharKey { id: pKey; width: Units.gu(3.9); label: "з"; shifted: "З"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"]; anchors.right: parent.right; thumbKeyboard: true;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight * 2/3

            CharKey { id: aKey; width: Units.gu(3.9); label: "ф"; shifted: "Ф"; anchors.left: parent.left; thumbKeyboard: true;}
            CharKey { id: sKey; width: Units.gu(3.9); label: "ы"; shifted: "Ы"; anchors.left: aKey.right; thumbKeyboard: true;}
            CharKey { id: dKey; width: Units.gu(3.9); label: "в"; shifted: "В"; anchors.left: sKey.right; thumbKeyboard: true;}
            CharKey { id: fKey; width: Units.gu(3.9); label: "а"; shifted: "А"; anchors.left: dKey.right; thumbKeyboard: true;}
            CharKey { id: gKey; width: Units.gu(3.9); label: "п"; shifted: "П"; anchors.left: fKey.right; thumbKeyboard: true;}
            CharKey { id: hKey; width: Units.gu(3.9); label: "р"; shifted: "Р"; anchors.right: jKey.left; thumbKeyboard: true;}
            CharKey { id: jKey; width: Units.gu(3.9); label: "о"; shifted: "О"; anchors.right: kKey.left; thumbKeyboard: true;}
            CharKey { id: kKey; width: Units.gu(3.9); label: "л"; shifted: "Л"; anchors.right: lKey.left; thumbKeyboard: true;}
            CharKey { id: lKey; width: Units.gu(3.9); label: "д"; shifted: "Д"; anchors.right: semicolonKey.left; thumbKeyboard: true;}
            CharKey { id: semicolonKey; width: Units.gu(3.9); label: "ж"; shifted: "Ж"; anchors.right: parent.right; thumbKeyboard: true;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight * 2/3

            CharKey { id: zKey; width: Units.gu(3.9); label: "я"; shifted: "Я"; anchors.left: parent.left; thumbKeyboard: true; }
            CharKey { id: xKey; width: Units.gu(3.9); label: "ч"; shifted: "Ч"; anchors.left: zKey.right; thumbKeyboard: true; }
            CharKey { id: cKey; width: Units.gu(3.9); label: "с"; shifted: "С"; anchors.left: xKey.right; thumbKeyboard: true; }
            CharKey { id: vKey; width: Units.gu(3.9); label: "м"; shifted: "М"; anchors.left: cKey.right; thumbKeyboard: true; }
            CharKey { id: bKey; width: Units.gu(3.9); label: "и"; shifted: "И"; anchors.left: vKey.right; thumbKeyboard: true; }
            CharKey { id: nKey; width: Units.gu(3.9); label: "т"; shifted: "Т"; anchors.right: mKey.left; thumbKeyboard: true; }
            CharKey { id: mKey; width: Units.gu(3.9); label: "ь"; shifted: "Ь"; anchors.right: leftBracketKey.left; thumbKeyboard: true; }
            CharKey { id: leftBracketKey; width: Units.gu(3.9); label: "б"; shifted: "б"; anchors.right: rightBracketKey.left; thumbKeyboard: true; }
            CharKey { id: rightBracketKey; width: Units.gu(3.9); label: "ю"; shifted: "Ю"; anchors.right: backspaceKey.left; thumbKeyboard: true;}
            BackspaceKey {id: backspaceKey; width: Units.gu(3.9); anchors.right: parent.right; thumbKeyboard: true;}
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight * 2/3
				
                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true;}
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true;}
                SpaceKey       { id: spaceKey1;                                 anchors.left: minusKey.right; thumbKeyboard: true;}
                TabKey         { id: tabKey1; fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: spaceKey1.right; anchors.right: symShiftKey.left; thumbKeyboard: true;}
                SymbolShiftKey { id: symShiftKey;                           anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true;}
                TabKey         { id: tabKey2; fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right; anchors.right: spaceKey2.left; thumbKeyboard: true;}
                SpaceKey       { id: spaceKey2;                                  anchors.right: dotKey.left; thumbKeyboard: true;}
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; width: Units.gu(3.9); thumbKeyboard: true; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight"; width: Units.gu(3.9); thumbKeyboard: true;  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8); thumbKeyboard: true;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight * 2/3

                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true;}
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true;}
                UrlKey         { id: emailKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "@"; shifted: "@";  anchors.left: minusKey.right; thumbKeyboard: true; }
                SpaceKey       { id: spaceKey1; width: Units.gu(3.9);                                 anchors.left: emailKey.right; anchors.right: tabKey1.left; thumbKeyboard: true;}
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.right: symShiftKey.left; thumbKeyboard: true;}
                SymbolShiftKey { id: symShiftKey;                         anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true;}
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right; thumbKeyboard: true;}
                SpaceKey       { id: spaceKey2; width: Units.gu(3.9);                                 anchors.left: tabKey2.right; anchors.right: urlKey.left; thumbKeyboard: true;}
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: ".com"; shifted: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk",".ac.uk"]; anchors.right: dotKey.left; thumbKeyboard: true; }
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; width: Units.gu(3.9); thumbKeyboard: true; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight"; width: Units.gu(3.9); thumbKeyboard: true;  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8); thumbKeyboard: true;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight * 2/3

                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left; thumbKeyboard: true;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right; thumbKeyboard: true;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right; thumbKeyboard: true;}
                AnnotatedKey   { id: minusKey; width: Units.gu(3.9); label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right; thumbKeyboard: true;}
                UrlKey         { id: slashKey; width: UI.isLandscape ? Units.gu(5.5) : Units.gu(4.2); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: "www."; shifted: "https://"; extended: ["www.", "https://", "http://"];  anchors.left: minusKey.right; thumbKeyboard: true; }
                SpaceKey       { id: spaceKey1;                                 anchors.left: slashKey.right; anchors.right: tabKey1.left; thumbKeyboard: true;}
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize: UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.right: symShiftKey.left; thumbKeyboard: true;}
                SymbolShiftKey { id: symShiftKey;                            anchors.horizontalCenter: parent.horizontalCenter; thumbKeyboard: true;}
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right; thumbKeyboard: true;}
                SpaceKey       { id: spaceKey2;                                  anchors.left: tabKey2.right; anchors.right: urlKey.left; thumbKeyboard: true;}
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.5) : Units.gu(4.2); fontSize: UI.isLandscape ? UI.xsFontSize : UI.thumbAnnotationFontSize; label: ".com"; shifted: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk",".ac.uk"]; anchors.right: dotKey.left; thumbKeyboard: true; }
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; width: Units.gu(3.9); anchors.right: arrowLeftKey.left; thumbKeyboard: true;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; width: Units.gu(3.9); thumbKeyboard: true; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight"; width: Units.gu(3.9); thumbKeyboard: true;  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8); thumbKeyboard: true;}
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
