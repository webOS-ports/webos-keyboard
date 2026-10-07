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
import QtMultimedia 6.3
import QtQuick.Window 2.0

import keys 1.0
import LunaNext.Common 0.1

Item {
    id: panel

    height: characterKeypadLoader.height

    property string currentKeyboardSize: maliit_input_method.keyboardSize
    property string currentAlternativeLayout: UI.currentAlternativeLayout

    function closeExtendedKeys() {
        extendedKeysSelector.closePopover();
    }

    Loader {
        id: characterKeypadLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: item ? item.height : 0
        asynchronous: false
        source: UI.currentSymbolState === "CHARACTERS" ? internal.characterKeypadSource : internal.symbolKeypadSource
        onLoaded: UI.currentShiftState = "NORMAL"

        // A layout picked in Text Assist need not exist for this language and
        // form factor - Dvorak and Thumb are tablet layouts, the Pre ones phone
        // layouts. Rather than leave no keyboard at all, fall back to the
        // stock layout.
        onStatusChanged: {
            if (status === Loader.Error && UI.currentSymbolState === "CHARACTERS"
                    && panel.currentAlternativeLayout !== "" && !internal.alternativeLayoutMissing) {
                console.warn("No '" + panel.currentAlternativeLayout + "' layout here, using the stock one");
                // Not from here: the error is reported while the source is
                // still being assigned, and changing characterKeypadSource
                // inside that is a binding loop that leaves no layout at all.
                Qt.callLater(function() { internal.alternativeLayoutMissing = true; });
            }
        }
    }

    // A different layout or language is worth trying again.
    onCurrentAlternativeLayoutChanged: internal.alternativeLayoutMissing = false
    Connections {
        target: maliit_input_method
        function onActiveLanguageChanged() { internal.alternativeLayoutMissing = false; }
    }

    // The Pre look goes with the Pre layout files themselves, so a fallback
    // from one does not keep it.
    Binding {
        target: UI
        property: "preVariant"
        value: internal.characterKeypadSource.indexOf("_preorange.qml") >= 0 ? "orange"
             : internal.characterKeypadSource.indexOf("_prewhite.qml") >= 0 ? "white" : ""
    }

    MediaPlayer {
        id: audioFeedback
        source: Qt.resolvedUrl("styles/ubuntu/sounds/key_tick2_quiet.wav")
        audioOutput: AudioOutput {}
    }

    QtObject {
        id: internal

        property Item activeKeypad: characterKeypadLoader.item
        property bool alternativeLayoutMissing: false
        property string characterKeypadSource: loadLayout(maliit_input_method.contentType,
                                                          maliit_input_method.activeLanguage,
                                                          alternativeLayoutMissing ? "" : panel.currentAlternativeLayout)
        property string symbolKeypadSource: ""

        onCharacterKeypadSourceChanged: {
            UI.currentSymbolState = "CHARACTERS";
        }
        onActiveKeypadChanged: {
            // don't do property binding, to avoid a binding loop with characterKeypadLoader.source
            if( UI.currentSymbolState === "CHARACTERS" ) {
                symbolKeypadSource = activeKeypad ? activeKeypad.symbols : "";
            }
        }

        /// Returns if the given language is supported
        function languageIsSupported(locale) {
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
                "no",
                "pl",
                "pt",
                "ru",
                "sv",
                "uk",
                "zh"
            ];
            return (supportedLocales.indexOf( locale ) > -1);
        }

        /// Returns the relative path to the keyboard QML file for a given language for free text
        function freeTextLanguageKeyboard(language, alternativeLayout) {
            language = language .slice(0,2).toLowerCase();
            // Layout names are what Text Assist shows - "Dvorak", "Pre (Orange)" -
            // and file names take only their letters and digits: "_preorange".
            alternativeLayout = alternativeLayout.toLowerCase().replace(/[^a-z0-9]/g, "")

            if (!languageIsSupported(language)) {
                console.log("Language '"+language+"' not supported - using 'en' instead");
                language = "en";
            }
            if( alternativeLayout.length > 0 ) {
                alternativeLayout = "_" + alternativeLayout;
            }

            var selectedLanguageFile = "lib/en/Keyboard_en.qml";

            // results in something like "lib/en/Keyboard_en_tablet.qml"
            selectedLanguageFile = "lib/" + language + "/Keyboard_" + language + "_" + UI.formFactor + alternativeLayout + ".qml";

            return selectedLanguageFile;
        }

        function loadLayout(contentType, activeLanguage, alternativeLayout) {
            var selectedLayoutFile;

            if (contentType === 1) {
                selectedLayoutFile = "languages/Keyboard_numbers.qml";
            }

            else if (contentType === 2) {
                selectedLayoutFile = "languages/Keyboard_telephone.qml";
            }

            else {
                selectedLayoutFile = freeTextLanguageKeyboard(activeLanguage, alternativeLayout);

                // for testing on desktop
                if( maliit_input_method.testEnvironment )
                {
                    // in a test environment, the "lib/<locale>/" directory is indeed a "plugins/<locale>/qml" directory
                    // ... except for Chinese, whose plugin directory is named after the
                    // input method rather than the locale.
                    var pluginDirs = { "zh": "pinyin" };
                    var regexp = /lib\/(..)\//;
                    selectedLayoutFile = selectedLayoutFile.replace(regexp, function(match, locale) {
                        return '../plugins/' + (pluginDirs[locale] || locale) + '/qml/';
                    });
                }
            }

            return selectedLayoutFile;
        }
    }
}
