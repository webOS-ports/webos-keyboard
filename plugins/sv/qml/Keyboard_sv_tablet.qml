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
    symbols: "languages/Keyboard_symbols_tablet.qml"
    alternativeLayouts: [ ] // list of alternative layouts, like Dvorak, Bepo, Splitted...

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { width: UI.keyWidth * 1.03; label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]}
            NumKey { width: UI.keyWidth * 1.03; label: "2"; shifted: "@"; extended: ["2", "@", "²"]}
            NumKey { width: UI.keyWidth * 1.03; label: "3"; shifted: "#"; extended: ["3", "#", "³", "¾"]}
            NumKey { width: UI.keyWidth * 1.03; label: "4"; shifted: "¤"; extended: ["4", "¤", "€", "£", "¥", "¢", "$"]}
            NumKey { width: UI.keyWidth * 1.03; label: "5"; shifted: "%"; extended: ["5", "%", "‰"]}
            NumKey { width: UI.keyWidth * 1.03; label: "6"; shifted: "&"; extended: ["6", "&"]}
            NumKey { width: UI.keyWidth * 1.03; label: "7"; shifted: "/"; extended: ["7", "/"]}
            NumKey { width: UI.keyWidth * 1.03; label: "8"; shifted: "("; extended: ["8", "(", ]}
            NumKey { width: UI.keyWidth * 1.03; label: "9"; shifted: ")"; extended: ["9", ")", "[", "{"]}
            NumKey { width: UI.keyWidth * 1.03; label: "0"; shifted: "?"; extended: ["0", "?", "]", "}"]}
            TrackBall { width: keypadRoot.width - (UI.keyWidth * 1.03 * 10); anchors.verticalCenter: parent.verticalCenter }
        }

        Row {
            anchors.left:parent.left;
            anchors.right: parent.right;
          
            spacing: 0

            height: keyHeight

            CharKey { label: "q"; shifted: "Q"; width: UI.keyWidth * 0.95; }
            CharKey { label: "w"; shifted: "W"; width: UI.keyWidth * 0.95;}
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®"]; extendedShifted: ["R","®"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ"]; extendedShifted: ["T", "™", "Þ"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Ù","Ú","Û", "Ü", "Ű"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"]; width: UI.keyWidth * 0.95; }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"]; width: UI.keyWidth * 0.95;}
            CharKey { label: "å"; shifted: "Å"; width: UI.keyWidth * 0.95;}
            BackspaceKey { width: keypadRoot.width - (UI.keyWidth * 0.95 * 11); }
        }

        Row {
            anchors.right: parent.right
            spacing: 0

            height: keyHeight

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"]; width: UI.keyWidth * 0.98;}
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$"]; width: UI.keyWidth * 0.98;}
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡"]; extendedShifted: ["D", "Ð", "†", "‡"]; width: UI.keyWidth * 0.98; }
            CharKey { label: "f"; shifted: "F"; width: UI.keyWidth * 0.98;}
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]; width: UI.keyWidth * 0.98;}
            CharKey { label: "h"; shifted: "H"; width: UI.keyWidth * 0.98;}
            CharKey { label: "j"; shifted: "J"; width: UI.keyWidth * 0.98;}
            CharKey { label: "k"; shifted: "K"; width: UI.keyWidth * 0.98;}
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł"]; extendedShifted: ["L", "Ł"]; width: UI.keyWidth * 0.98;}
            CharKey { label: "ö"; shifted: "Ö"; width: UI.keyWidth * 0.98;}
            CharKey { label: "ä"; shifted: "Ä"; width: UI.keyWidth * 0.98;}
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; width: UI.keyWidth * 0.98;}
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right

            height: keyHeight

            ShiftKey {id: shiftKeyLeft; anchors.left: parent.left;}
            CharKey {id: zKey; label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]; anchors.left: shiftKeyLeft.right;}
            CharKey {id: xKey; label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]; anchors.left: zKey.right;}
            CharKey {id: cKey; label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢"]; anchors.left: xKey.right; }
            CharKey {id: vKey; label: "v"; shifted: "V"; anchors.left: cKey.right;}
            CharKey {id: bKey; label: "b"; shifted: "B"; anchors.left: vKey.right;}
            CharKey {id: nKey; label: "n"; shifted: "N"; extended: ["n", "ñ", "ń"]; extendedShifted: ["N", "Ñ", "Ń"]; anchors.left: bKey.right;}
            CharKey {id: mKey; label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]; anchors.left: nKey.right;}
            AnnotatedKey { id: commaKey; label: ","; shifted: ";"; extended: [",", ";", "/", "\\"]; extendedShifted: ["," , ";", "/", "\\"]; anchors.left: mKey.right; }
            AnnotatedKey { id: dotkey; label: "."; shifted: ":"; extended: [".", ":", "?", "•", "…", "¿"]; extendedShifted: [".", ":", "?", "•", "…", "¿"]; anchors.left: commaKey.right; }
            ShiftKey {id: shiftKeyRight; anchors.left: dotkey.right; anchors.right: parent.right}
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: emailKey;    label: "@"; shifted: "@";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: emailKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; shifted: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"];  anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; extended: [".net", ".org", ".edu", ".gov", ".co.uk", ".ac.uk"]; anchors.right: apostropheKey.left; }
                AnnotatedKey   { id: apostropheKey; label: "'"; shifted: "\""; extended: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; extendedShifted: ["'", "\"", "`", "‘", "’", "“", "”", "«", "»"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "_"; extended: ["-", "_", "±", "¬"]; extendedShifted: ["-", "_", "±", "¬"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false; }
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
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
