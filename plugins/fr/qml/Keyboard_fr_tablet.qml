/*
 * Copyright 2013 Canonical Ltd.
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
    //alternativeLayouts: [ "Bepo" ] // list of alternative layouts, like Dvorak, Bepo, Splitted...
    
    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { label: "&"; shifted: "1"; extended: ["&", "1", "¹", "¼", "½", "¡"]}
            NumKey { label: "é"; shifted: "2"; extended: ["é", "2", "²"]}
            NumKey { label: "\""; shifted: "3"; extended: ["\"", "3", "³", "¾", "“", "”", "«", "»"]}
            NumKey { label: "'"; shifted: "4"; extended: ["'", "4", "‘", "’"]}
            NumKey { label: "("; shifted: "5"; extended: ["(", "5", "[","{"]}
            NumKey { label: "-"; shifted: "6"; extended: ["-", "6", "±", "¬"]}
            NumKey { label: "è"; shifted: "7"; extended: ["è", "7", "`"]}
            NumKey { label: ")"; shifted: "8"; extended: [")", "8", "]","}"]}
            NumKey { label: "ç"; shifted: "9"; extended: ["ç", "9", "¢", "$", "€", "£", "¥", "¤"]}
            NumKey { label: "à"; shifted: "0"; extended: ["à", "0", "%", "‰"]}
            TrackBall { width: keypadRoot.width - (UI.keyWidth*UI.numKeyWidthRatio*10); anchors.verticalCenter: parent.verticalCenter }
        }        
        
        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"] }
            CharKey { label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"]}
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€"] }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®"]; extendedShifted: ["R","®"]}
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ"]; extendedShifted: ["T", "™", "Þ"]}
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"] }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "ù", "ú", "û", "û", "ü", "ű"]; extendedShifted: ["U", "Ù", "Ú", "Û", "Ü", "Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"] }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"] }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "¶", "§", "Π"]}
            BackspaceKey {}
        }

        Row {
            anchors.right: parent.right
            spacing: 0

            height: keyHeight;

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$"] }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡"]; extendedShifted: ["D", "Ð", "†", "‡"] }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]}
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł"]; extendedShifted: ["L", "Ł"]}
            CharKey { label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]}
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; width: UI.keyWidth * 0.75; fontSize: UI.isLandscape ? UI.xsFontSize : "10pt"}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey {}
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]}
            CharKey { label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢"] }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["n", "ñ", "ń"]; extendedShifted: ["N", "Ñ", "Ń"] }
            AnnotatedKey { label: ","; shifted: "?"; extended: [",", "?", "¿"]; extendedShifted: [",", "?", "¿"]; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            AnnotatedKey { label: "."; shifted: ";"; extended: [".", ";", "•", "…"]; extendedShifted: [".", ";", "•", "…"]; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            AnnotatedKey { label: ":"; shifted: "/"; extended: [":", "/", "\\"]; extendedShifted: [":", "/", "\\"]; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
            ShiftKey {}
        }

        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey; 				label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: atKey.left; }
                AnnotatedKey   { id: atKey; label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"]; anchors.right: exclamationKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: exclamationKey;      label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey; extended: "Tab";  shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: emailKey;    label: "@"; shifted: "@";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: emailKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; shifted: ".com"; extended: [".com", "fr", ".net", ".org", ".edu", ".gov"]; anchors.right: atKey.left; }
                AnnotatedKey   { id: atKey; label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"]; anchors.right: exclamationKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: exclamationKey;      label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                TabKey         { id: tabKey; extended: "Tab";  shifted: "Tab";       label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; extended: [".com", ".fr", ".net", ".org", ".edu", ".gov"]; anchors.right: atKey.left; }
                AnnotatedKey   { id: atKey; label: "@"; shifted: "_"; extended: ["@", "_"]; extendedShifted: ["@", "_"]; anchors.right: exclamationKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: exclamationKey;      label: "!"; shifted: "*"; extended: ["!", "*", "¡"]; extendedShifted: ["!", "*", "¡"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
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
