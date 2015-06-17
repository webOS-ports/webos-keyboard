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
    alternativeLayouts: [ "Dvorak", "Thumb" ] // list of alternative layouts, like Dvorak, Bepo, Splitted...
    keyWidthRatio: 0.38

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

		Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { id: oneKey; label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]; keyWidth: UI.keyWidth; anchors.left: parent.left; }
            NumKey { id: twoKey; label: "2"; shifted: "@"; extended: ["2", "@", "²"]; keyWidth: UI.keyWidth; anchors.left: oneKey.right; }
            NumKey { id: threeKey; label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"]; keyWidth: UI.keyWidth; anchors.left: twoKey.right; }
            NumKey { id: fourKey; label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; keyWidth: UI.keyWidth; anchors.left: threeKey.right; }
            NumKey { id: fiveKey; label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; keyWidth: UI.keyWidth; anchors.left: fourKey.right; }
            NumKey { id: sixKey; label: "6"; shifted: "^"; extended: ["6", "^"]; keyWidth: UI.keyWidth; anchors.right: sevenKey.left; }
            NumKey { id: sevenKey; label: "7"; shifted: "&"; extended: ["7", "&"]; keyWidth: UI.keyWidth; anchors.right: eightKey.left; }
            NumKey { id: eightKey; label: "8"; shifted: "*"; extended: ["8", "*"]; keyWidth: UI.keyWidth; anchors.right: nineKey.left; }
            NumKey { id: nineKey; label: "9"; shifted: "("; extended: ["9", "(", "[", "{"]; keyWidth: UI.keyWidth; anchors.right: zeroKey.left; }
            NumKey { id: zeroKey; label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"]; keyWidth: UI.keyWidth; anchors.right: parent.right; }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: qKey; label: "q"; shifted: "Q"; anchors.left: parent.left;}
            CharKey { id: wKey; label: "w"; shifted: "W"; anchors.left: qKey.right;}
            CharKey { id: eKey; label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€"]; anchors.left: wKey.right;}
            CharKey { id: rKey; label: "r"; shifted: "R"; extended: ["r", "®"]; extendedShifted: ["R","®"]; anchors.left: eKey.right;}
            CharKey { id: tKey; label: "t"; shifted: "T"; extended: ["t", "™", "þ"]; extendedShifted: ["T", "™", "Þ"]; anchors.left: rKey.right;}
            CharKey { id: yKey; label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; anchors.right: uKey.left;}
            CharKey { id: uKey; label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Ù","Ú","Û", "Ü", "Ű"]; anchors.right: iKey.left;}
            CharKey { id: iKey; label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; anchors.right: oKey.left;}
            CharKey { id: oKey; label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; anchors.right: pKey.left;}
            CharKey { id: pKey; label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"]; anchors.right: parent.right;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: aKey; label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"]; anchors.left: parent.left;}
            CharKey { id: sKey; label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$"]; anchors.left: aKey.right;}
            CharKey { id: dKey; label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡"]; extendedShifted: ["D", "Ð", "†", "‡"]; anchors.left: sKey.right;}
            CharKey { id: fKey; label: "f"; shifted: "F"; anchors.left: dKey.right;}
            CharKey { id: gKey; label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]; anchors.left: fKey.right;}
            CharKey { id: hKey; label: "h"; shifted: "H"; anchors.right: jKey.left;}
            CharKey { id: jKey; label: "j"; shifted: "J"; anchors.right: kKey.left;}
            CharKey { id: kKey; label: "k"; shifted: "K"; anchors.right: lKey.left;}
            CharKey { id: lKey; label: "l"; shifted: "L"; extended: ["l", "ł"]; extendedShifted: ["L", "Ł"]; anchors.right: semicolonKey.left;}
            AnnotatedKey { id: semicolonKey; label: ";"; shifted: ":"; extended: [";", ":"]; extendedShifted: [";", ":"]; anchors.right: parent.right;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: zKey; label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; anchors.left: parent.left; }
            CharKey { id: xKey; label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]; anchors.left: zKey.right; }
            CharKey { id: cKey; label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢"]; anchors.left: xKey.right; }
            CharKey { id: vKey; label: "v"; shifted: "V"; anchors.left: cKey.right; }
            CharKey { id: bKey; label: "b"; shifted: "B"; anchors.left: vKey.right; }
            CharKey { id: nKey; label: "n"; shifted: "N"; extended: ["n", "ñ", "ń"]; extendedShifted: ["N", "Ñ", "Ń"]; anchors.right: mKey.left; }
            CharKey { id: mKey; label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]; anchors.right: leftBracketKey.left; }
            CharKey { id: leftBracketKey; label: "["; shifted: "["; anchors.right: rightBracketKey.left; }
            CharKey { id: rightBracketKey; label: "]"; shifted: "]"; anchors.right: backspaceKey.left;}
            BackspaceKey {id: backspaceKey; anchors.right: parent.right;}
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight
				
                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right;}
                AnnotatedKey   { id: minusKey; label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right;}
                SpaceKey       { id: spaceKey1;                                 anchors.left: minusKey.right;}
                TabKey         { id: tabKey1; fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: spaceKey1.right; anchors.right: symShiftKey.left;}
                SymbolShiftKey { id: symShiftKey;                           anchors.horizontalCenter: parent.horizontalCenter;}
                TabKey         { id: tabKey2; fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right; anchors.right: spaceKey2.left;}
                SpaceKey       { id: spaceKey2;                                  anchors.right: dotKey.left;}
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; anchors.right: arrowLeftKey.left;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight";  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8);}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right;}
                AnnotatedKey   { id: minusKey; label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right;}
                UrlKey         { id: emailKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize; label: "@"; shifted: "@";  anchors.left: minusKey.right; }
                SpaceKey       { id: spaceKey1;                                 anchors.left: emailKey.right; anchors.right: tabKey1.left;}
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.right: symShiftKey.left;}
                SymbolShiftKey { id: symShiftKey;                         anchors.horizontalCenter: parent.horizontalCenter;}
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right;}
                SpaceKey       { id: spaceKey2;                                 anchors.left: tabKey2.right; anchors.right: urlKey.left;}
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.2) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize; label: ".com"; shifted: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk",".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; anchors.right: arrowLeftKey.left;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight";  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8);}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                ShiftKey {id: shiftKey; width: Units.gu(5.2); anchors.left: parent.left;}
                DismissKey     { id: dismissKey; width: Units.gu(5.2); anchors.left: shiftKey.right;}
                LanguageKey    { id: languageMenuButton;  width: Units.gu(5.2); anchors.left: dismissKey.right;}
                AnnotatedKey   { id: minusKey; label: "/"; shifted: "-"; extended: ["/", "-"]; extendedShifted: ["/", "-"]; anchors.left: languageMenuButton.right;}
                UrlKey         { id: slashKey; width: UI.isLandscape ? Units.gu(5.5) : Units.gu(4.2); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize; label: "www."; shifted: "https://"; extended: ["www.", "https://", "http://"];  anchors.left: minusKey.right; }
                SpaceKey       { id: spaceKey1;                                 anchors.left: slashKey.right; anchors.right: tabKey1.left;}
                TabKey         { id: tabKey1; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize: UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.right: symShiftKey.left;}
                SymbolShiftKey { id: symShiftKey;                            anchors.horizontalCenter: parent.horizontalCenter;}
                TabKey         { id: tabKey2; width: UI.isLandscape ? Units.gu(7.8) : Units.gu(3.9); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize;				label: "Tab"; shifted: "Tab";                       anchors.left: symShiftKey.right;}
                SpaceKey       { id: spaceKey2;                                  anchors.left: tabKey2.right; anchors.right: urlKey.left;}
                UrlKey         { id: urlKey; width: UI.isLandscape ? Units.gu(5.5) : Units.gu(4.2); fontSize: UI.isLandscape ? UI.xsFontSize : UI.annotationFontSize; label: ".com"; shifted: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk",".ac.uk"]; anchors.right: dotKey.left; }
                AnnotatedKey { id: dotKey; label: "."; shifted: ","; extended: [".", ","]; extendedShifted: [".", ","]; anchors.right: arrowLeftKey.left;}
                ActionKey {id: arrowLeftKey; label: "←"; shifted: "←"; action: "keyLeft"; anchors.right: arrowRightKey.left;}
                ActionKey {id: arrowRightKey; label: "→"; shifted: "→";action: "keyRight";  anchors.right: enterKey.left;}
                ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; alignTextRight: true; anchors.right: parent.right; width: Units.gu(7.8);}
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
