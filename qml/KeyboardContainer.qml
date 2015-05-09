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
import QtMultimedia 5.0
import QtQuick.Window 2.0
import "languages/"
import "keys/"
import UbuntuKeyboard 1.0
import LunaNext.Common 0.1

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
                "zh"
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
            var formFactor = Settings.tabletUi ? "tablet" : "phone";

            // results in something like "lib/en/Keyboard_en_tablet.qml"
            selectedLanguageFile = "lib/" + language + "/Keyboard_" + language + "_" + formFactor + ".qml";

            return selectedLanguageFile;
        }

        function loadLayout(contentType, activeLanguage)
        {
            var selectedLayoutFile;

            //            if (contentType === InputMethod.NumberContentType) {
            if (contentType === 1) {
                selectedLayoutFile = "languages/Keyboard_numbers.qml";
            }

            //            if (contentType === InputMethod.PhoneNumberContentType) {
            else if (contentType === 2) {
                selectedLayoutFile = "languages/Keyboard_telephone.qml";
            }

            else {
                var locale = activeLanguage.slice(0,2).toLowerCase();
                if (!languageIsSupported(locale)) {
                    console.log("System language '"+locale+"' can't be used in OSK - using 'en' instead")
                    locale = "en"
                }

                var formFactor = Settings.tabletUi ? "tablet" : "phone";

                //            if (contentType === InputMethod.EmailContentType) {
                if (contentType === 3) {
                    selectedLayoutFile = "lib/"+locale+"/Keyboard_"+locale +"_"+formFactor+"_email.qml";
                }

                //            if (contentType === InputMethod.UrlContentType) {
                if (contentType === 4) {
                    selectedLayoutFile = "lib/"+locale+"/Keyboard_"+locale + "_"+formFactor+"_url_search.qml";
                }

                else {
                    // FreeTextContentType used as fallback
                    selectedLayoutFile = freeTextLanguageKeyboard(activeLanguage);
                }

                // for testing on desktop
                if( maliit_input_method.testEnvironment )
                {
                    // in a test environment, the "lib/<locale>/" directory is indeed a "plugins/<locale>/qml" directory
                    var regexp = /lib\/(..)\//;
                    selectedLayoutFile = selectedLayoutFile.replace(regexp, '../plugins/$1/qml/');
                }
            }

            return selectedLayoutFile;
        }
    }
}
