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

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight * UI.topRowKeyHeightRatio

            NumKey { label: "1"; shifted: "!"; extended: ["1", "!", "¹", "¼", "½", "¡"]; keyWidth: UI.keyWidth;}
            NumKey { label: "2"; shifted: "\""; extended: ["2", "\"", "²", "”", "„", "“", "«", "»"]; keyWidth: UI.keyWidth;}
            NumKey { label: "3"; shifted: "@"; extended: ["3", "@", "³", "¾"]; keyWidth: UI.keyWidth;}
            NumKey { label: "4"; shifted: "$"; extended: ["4", "$", "€", "£", "¥", "¢", "¤"]; keyWidth: UI.keyWidth;}
            NumKey { label: "5"; shifted: "%"; extended: ["5", "%", "‰"]; keyWidth: UI.keyWidth;}
            NumKey { label: "6"; shifted: "&"; extended: ["6", "&"]; keyWidth: UI.keyWidth;}
            NumKey { label: "7"; shifted: "/"; extended: ["7", "/", "\\"]; keyWidth: UI.keyWidth;}
            NumKey { label: "8"; shifted: "("; extended: ["8", "(", "[","{"]; keyWidth: UI.keyWidth;}
            NumKey { label: "9"; shifted: ")"; extended: ["9", ")", "]", "}"]; keyWidth: UI.keyWidth;}
            NumKey { label: "0"; shifted: "="; extended: ["0", "="]; keyWidth: UI.keyWidth;}
            TrackBall { width: keypadRoot.width - (UI.keyWidth*10); anchors.verticalCenter: parent.verticalCenter }        
            }
    
        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            CharKey { label: "q"; shifted: "Q"; }
            CharKey { label: "w"; shifted: "W"; }
            CharKey { label: "e"; shifted: "E"; extended: ["e", "è", "é", "ê", "ë", "ę", "ē", "€", "ě"]; extendedShifted: ["E", "È", "É", "Ê", "Ë", "Ę", "Ē", "€", "Ě"] }
            CharKey { label: "r"; shifted: "R"; extended: ["r", "®", "ř", "ŕ"]; extendedShifted: ["R","®", "Ř", "Ŕ"] }
            CharKey { label: "t"; shifted: "T"; extended: ["t", "™", "þ", "ť", "ţ"]; extendedShifted: ["T", "™", "Þ", "Ť", "Ţ"] }
            CharKey { label: "z"; shifted: "Z"; extended: ["z", "ž", "ź", "ż"]; extendedShifted: ["Z", "Ž", "Ź", "Ż"] }
            CharKey { label: "u"; shifted: "U"; extended: ["u", "û", "ù", "ú", "û", "ü", "ű"]; extendedShifted: ["U", "Û","Ù","Ú","Û", "Ü", "Ű"] }
            CharKey { label: "i"; shifted: "I"; extended: ["i", "ì","í", "î", "ï", "İ", "ı"]; extendedShifted: ["I", "Ì", "Í", "Î", "Ï", "İ", "ı"] }
            CharKey { label: "o"; shifted: "O"; extended: ["o", "ò", "ó", "ô", "õ", "ö", "ø", "ő", "œ", "º", "ω"]; extendedShifted: ["O", "Ò", "Ó", "Ô", "Õ", "Ö", "Ø", "Ő", "Œ", "º", "Ω"] }
            CharKey { label: "p"; shifted: "P"; extended: ["p", "¶", "§", "π"]; extendedShifted: ["P", "§", "Π"]}
            CharKey { label: "ü"; shifted: "Ü"; }
            BackspaceKey {}
        }

        Row {
            anchors.right: parent.right
            spacing: 0

            height: keyHeight;

            CharKey { label: "a"; shifted: "A"; extended: ["a", "à", "á", "â", "ã" , "ä", "å", "æ", "ª"]; extendedShifted: ["A", "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "ª"] }
            CharKey { label: "s"; shifted: "S"; extended: ["s", "š", "ş", "ß", "σ", "$", "ś"]; extendedShifted: ["S", "Š", "Ş", "ß", "Σ", "$", "Ś"] }
            CharKey { label: "d"; shifted: "D"; extended: ["d", "ð", "†", "‡", "ď", "đ"]; extendedShifted: ["D", "Ð", "†", "‡", "Ď", "Đ"] }
            CharKey { label: "f"; shifted: "F"; }
            CharKey { label: "g"; shifted: "G"; extended: ["g", "ğ"]; extendedShifted: ["G", "Ğ"]}
            CharKey { label: "h"; shifted: "H"; }
            CharKey { label: "j"; shifted: "J"; }
            CharKey { label: "k"; shifted: "K"; }
            CharKey { label: "l"; shifted: "L"; extended: ["l", "ł", "ĺ"]; extendedShifted: ["L", "Ł", "Ĺ"]}
            CharKey { label: "ö"; shifted: "Ö"; }
            CharKey { label: "ä"; shifted: "Ä"; }
            ReturnKey { id: enterKey; label: "Enter"; shifted: "Enter"; width: UI.keyWidth * 0.8; fontSize: UI.isLandscape ? UI.xsFontSize : "10pt"}
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            spacing: 0

            height: keyHeight;

            ShiftKey { width: UI.keyWidth * 0.8 }
            CharKey { label: "y"; shifted: "Y"; extended: ["y", "ý", "ÿ", "¥"]; extendedShifted: ["Y", "Ý", "Ÿ", "¥"] }
            CharKey { label: "x"; shifted: "X"; extended: ["x", "Rec", "Mute"]; extendedShifted: ["X", "Rec", "Mute"]}
            CharKey { label: "c"; shifted: "C"; extended: ["c", "ç", "ć", "©", "¢", "č"]; extendedShifted: ["C", "Ç", "Ć", "©", "¢", "Č"] }
            CharKey { label: "v"; shifted: "V"; }
            CharKey { label: "b"; shifted: "B"; }
            CharKey { label: "n"; shifted: "N"; extended: ["n", "ñ", "ń", "ň"]; extendedShifted: ["N", "Ñ", "Ń", "Ň"] }
            CharKey { label: "m"; shifted: "M"; extended: ["m", "μ"]; extendedShifted: ["M", "Μ"]}
            AnnotatedKey { label: ","; shifted: ";"; extended: [",", ";"]; extendedShifted: [",", ";"];}
            AnnotatedKey { id: dotKey; label: "."; shifted: ":"; extended: [".", ":", "•", "…"]; extendedShifted: [".", ":", "•", "…"]}
            ShiftKey { width: UI.keyWidth * 2.2 }
         }


        Component {
            id: contentTypeNormal
            Item {
                height: keyHeight

                TabKey         { id: tabKey;                 label: "Tab"; shifted: "Tab";                       anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                SpaceKey       { id: spaceKey;                                 anchors.left: languageMenuButton.right; anchors.right: ssKey.left; }
                AnnotatedKey   { id: ssKey; label: "ß"; shifted: "?"; extended: ["ß", "?", "¿"]; extendedShifted: ["ß", "?", "¿"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "'"; extended: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; extendedShifted: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeEmail

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab"; label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                UrlKey         { id: emailKey;    label: "@"; shifted: "@";  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: emailKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; shifted: ".com"; extended: [".com", ".de", ".at", ".ch", ".net", ".org", ".edu", ".gov"]; anchors.right: ssKey.left; }
                AnnotatedKey   { id: ssKey; label: "ß"; shifted: "?"; extended: ["ß", "?", "¿"]; extendedShifted: ["ß", "?", "¿"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "'"; extended: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; extendedShifted: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                DismissKey     { id: dismissKey;                               anchors.right: parent.right;}
            }
        }
        Component {
            id: contentTypeUrl

            Item {
                height: keyHeight

                TabKey         { id: tabKey; shifted: "Tab"; label: "Tab";              anchors.left: parent.left; }
                SymbolShiftKey { id: symShiftKey;                             anchors.left: tabKey.right; }
                LanguageKey    { id: languageMenuButton;                       anchors.left: symShiftKey.right; }
                CharKey        { imgNormal: UI.imageGreyKey; imgPressed: UI.imageGreyKeyPressed; id: slashKey;    label: "/"; shifted: "/"; extended: ["http://", "https://", "www."];  anchors.left: languageMenuButton.right; }
                SpaceKey       { id: spaceKey;                               anchors.left: slashKey.right; anchors.right: urlKey.left; }
                UrlKey         { id: urlKey; label: ".com"; extended: [".com", ".de", ".at", ".ch", ".net", ".org", ".edu", ".gov"]; anchors.right: ssKey.left; }
                AnnotatedKey   { id: ssKey; label: "ß"; shifted: "?"; extended: ["ß", "?", "¿"]; extendedShifted: ["ß", "?", "¿"]; anchors.right: minusKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
                AnnotatedKey   { id: minusKey;      label: "-"; shifted: "'"; extended: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; extendedShifted: ["-", "'", "±", "¬", "`", "‚", "‘","’"]; anchors.right: dismissKey.left; useHorizontalLayout: (UI.keyboardSizeChoice === "XS" || UI.keyboardSizeChoice === "S") ? true : false;}
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
