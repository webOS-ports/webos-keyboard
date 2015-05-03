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
import QtMultimedia 5.0
import QtQuick.Window 2.0
import "languages/"
import "keys/"
import UbuntuKeyboard 1.0

Item {
    id: panel

    property int keyWidth: 0
    property int keyHeight: 0

    property string activeKeypadState: "NORMAL"
    property alias popoverEnabled: extendedKeysSelector.enabled

    state: "CHARACTERS"

    function closeExtendedKeys()
    {
        extendedKeysSelector.closePopover();
    }

    Loader {
        id: characterKeypadLoader
        objectName: "characterKeyPadLoader"
        anchors.fill: parent
        asynchronous: false
        source: panel.state === "CHARACTERS" ? internal.characterKeypadSource : internal.symbolKeypadSource
        onLoaded: activeKeypadState = "NORMAL"
    }

    ExtendedKeysSelector {
        id: extendedKeysSelector
        objectName: "extendedKeysSelector"
        anchors.fill: parent
    }

    Audio {
        id: audioFeedback
        source: Qt.resolvedUrl("styles/ubuntu/sounds/key_tick2_quiet.wav")
    }

    states: [
        State {
            name: "CHARACTERS"
        },
        State {
            name: "SYMBOLS"
        }
    ]

    QtObject {
        id: internal

        property Item activeKeypad: characterKeypadLoader.item
        property string characterKeypadSource: loadLayout(maliit_input_method.contentType,
                                                          maliit_input_method.activeLanguage)
        property string symbolKeypadSource: activeKeypad ? activeKeypad.symbols : ""

        onCharacterKeypadSourceChanged: {
            panel.state = "CHARACTERS";
        }

        /// Returns if the given language is supported
        /// FIXME the possible languages should be checked in C++
        function languageIsSupported(locale)
        {
            var supportedLocales = [
                "ar",
                "cs",
                "da",
                "de",
                "en",
                "es",
                "fi",
                "fr",
                "he",
                "hu",
                "it",
                "nl",
                "pl",
                "pt",
                "ru",
                "sv",
                "zh",
                "xx"
            ];
            return (supportedLocales.indexOf( locale ) > -1);
        }

        /// Returns the relative path to the keyboard QML file for a given language for free text
        function freeTextLanguageKeyboard(language)
        {
            language = language .slice(0,2).toLowerCase();

            if (!languageIsSupported(language)) {
                console.log("Language '"+language+"' not supported - using 'en' instead");
                language = "en";
            }

            var selectedLanguageFile = "lib/en/Keyboard_en.qml";

            if (language === "ar")
                selectedLanguageFile = "lib/ar/Keyboard_ar.qml";
            else if (language === "cs")
                selectedLanguageFile = "lib/cs/Keyboard_cs.qml";
            else if (language === "da")
                selectedLanguageFile = "lib/da/Keyboard_da.qml";
            else if (language === "de")
                selectedLanguageFile = "lib/de/Keyboard_de.qml";
            else if (language === "en")
                selectedLanguageFile = "lib/en/Keyboard_en.qml";
            else if (language === "es")
                selectedLanguageFile = "lib/es/Keyboard_es.qml";
            else if (language === "fi")
                selectedLanguageFile = "lib/fi/Keyboard_fi.qml";
            else if (language === "fr")
                selectedLanguageFile = "lib/fr/Keyboard_fr.qml";
            else if (language === "he")
                selectedLanguageFile = "lib/he/Keyboard_he.qml";
            else if (language === "hu")
                selectedLanguageFile = "lib/hu/Keyboard_hu.qml";
            else if (language === "it")
                selectedLanguageFile = "lib/it/Keyboard_it.qml";
            else if (language === "nl")
                selectedLanguageFile = "lib/nl/Keyboard_nl.qml";
            else if (language === "pl")
                selectedLanguageFile = "lib/pl/Keyboard_pl.qml";
            else if (language === "pt")
                selectedLanguageFile = "lib/pt/Keyboard_pt.qml";
            else if (language === "ru")
                selectedLanguageFile = "lib/ru/Keyboard_ru.qml";
            else if (language === "sv")
                selectedLanguageFile = "lib/sv/Keyboard_sv.qml";
            else if (language === "zh")
                selectedLanguageFile = "lib/zh/Keyboard_zh_cn_pinyin.qml";

            // for testing on desktop
            if( maliit_input_method.testEnvironment )
            {
                if (language === "xx")
                    selectedLanguageFile = "languages/en_webos/Keyboard_en_webos.qml";

                // in a test environment, the "lib/<language>/" directory is indeed a "plugins/<language>/qml" directory
                var regexp = /lib\/(..)\//;
                selectedLanguageFile = selectedLanguageFile.replace(regexp, '../plugins/$1/qml/');
            }

            return selectedLanguageFile;
        }

        function loadLayout(contentType, activeLanguage)
        {
            //            if (contentType === InputMethod.NumberContentType) {
            if (contentType === 1) {
                return "languages/Keyboard_numbers.qml";
            }

            //            if (contentType === InputMethod.PhoneNumberContentType) {
            if (contentType === 2) {
                return "languages/Keyboard_telephone.qml";
            }

            var locale = activeLanguage.slice(0,2).toLowerCase();
            if (!languageIsSupported(locale)) {
                console.log("System language '"+locale+"' can't be used in OSK - using 'en' instead")
                locale = "en"
            }

            //            if (contentType === InputMethod.EmailContentType) {
            if (contentType === 3) {
                return "lib/"+locale+"/Keyboard_"+locale+"_email.qml";
            }

            //            if (contentType === InputMethod.UrlContentType) {
            if (contentType === 4) {
                return "lib/"+locale+"/Keyboard_"+locale+"_url_search.qml";
            }

            // FreeTextContentType used as fallback
            return freeTextLanguageKeyboard(activeLanguage);
        }
    }
}
