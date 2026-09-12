/*
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 * Stand-ins for the objects the Maliit plugin puts on the QML engine's root
 * context. main.cpp creates this file and promotes each property below to a
 * context property under the same name.
 *
 * They cannot simply be declared with an id inside keyboard-test.qml: UI.qml is a
 * singleton, so it is instantiated in the engine's root context and cannot see
 * anything scoped to another component. That is what made the preview render with
 * NaN key sizes - UI.keyboardSizeChoice came back undefined, so every ratio lookup
 * in key_constants.js missed.
 */

import QtQuick

QtObject {
    id: stubs

    /*! Text the keyboard has sent, for the harness to display. */
    property string lastKey: ""

    /*! Set from the command line; -1 keeps SettingsStub's own default (tenderloin,
        which is the TouchPad and therefore the reference device). */
    property int startEnv: -1

    /*! Everything KeyboardContainer.languageIsSupported() accepts, for the picker
        in the harness. */
    readonly property var allLanguages: [
        "ar", "cs", "da", "de", "en", "es", "fi", "fr", "he", "hu",
        "it", "nl", "no", "pl", "pt", "ru", "sv", "uk", "zh"
    ]

    /*! Alternative layouts, per language. An entry missing here means the language
        only has the plain one. */
    readonly property var altLayouts: {
        "en": [ "LuneOS", "Dvorak", "Thumb" ],
        "sv": [ "LuneOS", "Dvorak" ],
        "ru": [ "LuneOS", "Thumb" ]
    }

    property QtObject maliit_input_method: QtObject {
        signal activateAutocaps()
        signal hide()

        // 0 text, 1 number, 2 telephone, 3 email, 4 url
        property int contentType: 0
        property bool testEnvironment: true
        property bool useAudioFeedback: false
        property string activeLanguage: "en"
        property string keyboardSize: "M"
        property string keyboardLayout: "LuneOS"
        property variant enabledLanguages: [ "en" ]
    }

    property QtObject maliit_geometry: QtObject {
        property rect popoverRect: Qt.rect(0, 0, 10, 20)
        property rect visibleRect: Qt.rect(0, 0, 700, 300)
        property int orientation: 0
        property bool shown: true
    }

    /* The plugin registers this under the maliit_ prefix; Keyboard.qml aliases it
       to a property called event_handler, which is the name the key components
       resolve through their component scope. */
    property QtObject maliit_event_handler: QtObject {
        signal keyReleased()

        function onKeyPressed(valueToSubmit, action) {
            console.log("onKeyPressed: " + valueToSubmit + " -> action: " + action);
        }
        function onKeyReleased(valueToSubmit, action) {
            console.log("onKeyReleased: " + valueToSubmit + " -> action: " + action);
            stubs.lastKey = valueToSubmit;
            keyReleased();
        }
        function onWordCandidatePressed(word) {
            console.log("onWordCandidatePressed: " + word);
        }
        function onWordCandidateReleased(word) {
            console.log("onWordCandidateReleased: " + word);
            stubs.lastKey = word;
        }
    }

    property QtObject maliit_word_engine: QtObject {
        property bool enabled: true
    }

    property var maliit_wordribbon: ListModel {
        ListElement { word: "first" }
        ListElement { word: "second" }
    }

    /*! The real one is a SoundEffect; the preview has no click sample to play. */
    property QtObject audioFeedback: QtObject {
        function play() { }
    }
}
