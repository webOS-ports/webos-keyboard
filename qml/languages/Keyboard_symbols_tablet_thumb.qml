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
    keyWidthRatio: 0.38

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

		Item {
            height: keyHeight * UI.topRowKeyHeightRatio

            anchors.left: parent.left
            anchors.right: parent.right
            NumKey { id: oneKey; label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]; anchors.left: parent.left; keyWidth: UI.keyWidth; }
            NumKey { id: twoKey; label: "2"; shifted: "@"; extended: ["2", "@", "²"]; anchors.left: oneKey.right; keyWidth: UI.keyWidth; }
            NumKey { id: threeKey; label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"]; anchors.left: twoKey.right; keyWidth: UI.keyWidth; }
            NumKey { id: fourKey; label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; anchors.left: threeKey.right; keyWidth: UI.keyWidth; }
            NumKey { id: fiveKey; label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; anchors.left: fourKey.right; keyWidth: UI.keyWidth; }
            NumKey { id: sixKey; label: "6"; shifted: "^"; extended: ["6", "^"]; anchors.right: sevenKey.left; keyWidth: UI.keyWidth; }
            NumKey { id: sevenKey; label: "7"; shifted: "&"; extended: ["7", "&"]; anchors.right: eightKey.left; keyWidth: UI.keyWidth; }
            NumKey { id: eightKey; label: "8"; shifted: "*"; extended: ["8", "*"]; anchors.right: nineKey.left; keyWidth: UI.keyWidth; }
            NumKey { id: nineKey; label: "9"; shifted: "("; extended: ["9", "(", "[", "{"]; anchors.right: zeroKey.left; keyWidth: UI.keyWidth; }
            NumKey { id: zeroKey; label: "0"; shifted: ")"; extended: ["0", ")", "]", "}"]; anchors.right: parent.right; keyWidth: UI.keyWidth; }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: qKey; label: "`"; shifted: "`"; anchors.left: parent.left;}
            CharKey { id: wKey; label: "~"; shifted: "~"; anchors.left: qKey.right;}
            CharKey { id: eKey; label: "€"; shifted: "€"; anchors.left: wKey.right;}
            CharKey { id: rKey; label: "£"; shifted: "£"; anchors.left: eKey.right;}
            CharKey { id: tKey; label: "\""; shifted: "\""; anchors.left: rKey.right;}
            CharKey { id: yKey; label: "|"; shifted: "|"; anchors.right: uKey.left;}
            CharKey { id: uKey; label: "{"; shifted: "{"; anchors.right: iKey.left;}
            CharKey { id: iKey; label: "}"; shifted: "}"; anchors.right: oKey.left;}
            CharKey { id: oKey; label: "["; shifted: "["; anchors.right: pKey.left;}
            CharKey { id: pKey; label: "]"; shifted: "]"; anchors.right: parent.right;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: aKey; label: "<"; shifted: "<"; anchors.left: parent.left;}
            CharKey { id: sKey; label: ">"; shifted: ">"; anchors.left: aKey.right;}
            CharKey { id: dKey; label: "="; shifted: "="; anchors.left: sKey.right;}
            CharKey { id: fKey; label: "+"; shifted: "+"; anchors.left: dKey.right;}
            CharKey { id: gKey; label: "×"; shifted: "×"; anchors.left: fKey.right;}
            CharKey { id: hKey; label: "÷"; shifted: "÷"; anchors.right: jKey.left;}
            CharKey { id: jKey; label: "°"; shifted: "°"; anchors.right: kKey.left;}
            CharKey { id: kKey; label: ";"; shifted: ";"; anchors.right: lKey.left;}
            CharKey { id: lKey; label: "\""; shifted: "\""; anchors.right: semicolonKey.left;}
            AnnotatedKey { id: semicolonKey; label: ";"; shifted: ":"; extended: [";", ":"]; extendedShifted: [";", ":"]; anchors.right: parent.right;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: keyHeight

            CharKey { id: zKey; label: ":-)"; shifted: ":-)"; anchors.left: parent.left; }
            CharKey { id: xKey; label: ";-)"; shifted: ";-)"; anchors.left: zKey.right; }
            CharKey { id: cKey; label: ":-("; shifted: ":-("; anchors.left: xKey.right; }
            CharKey { id: vKey; label: ":'("; shifted: ":'("; anchors.left: cKey.right; }
            CharKey { id: bKey; label: ":-P"; shifted: ":-P"; anchors.left: vKey.right; }
            CharKey { id: nKey; label: ":-O"; shifted: ":-O"; anchors.right: mKey.left; }
            CharKey { id: mKey; label: "<3"; shifted: "<3"; anchors.right: leftBracketKey.left; }
            CharKey { id: leftBracketKey; label: "{"; shifted: "["; anchors.right: rightBracketKey.left; }
            CharKey { id: rightBracketKey; label: "}"; shifted: "]"; anchors.right: backspaceKey.left;}
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
                SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC";                           anchors.horizontalCenter: parent.horizontalCenter;}
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
                SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC";                        anchors.horizontalCenter: parent.horizontalCenter;}
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
                SymbolShiftKey { id: symShiftKey; label: "ABC"; shifted: "ABC";                           anchors.horizontalCenter: parent.horizontalCenter;}
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
