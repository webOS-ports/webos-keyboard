/*
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
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


import QtQuick
import QtQuick.Controls

import LunaNext.Common 0.1
import keys 1.0

import "../../qml" as App

Rectangle {
    id: testRoot

    property bool isRotated: false

    width: 1024
    height: 800

    Rectangle {
        z: 10

        id: deviceScreenRect

        color: "black"

        x: 15; y: 15

        height: 200
        width: keyboardLoader.width * 200 / keyboardLoader.height

        Rectangle {
            color: "blue"

            property real widthRatio: keyboardLoader.width / deviceScreenRect.width
            property real heightRatio: keyboardLoader.height / deviceScreenRect.height
            x: maliit_geometry.visibleRect.x / widthRatio
            y: maliit_geometry.visibleRect.y / heightRatio
            width:  maliit_geometry.visibleRect.width / widthRatio
            height: maliit_geometry.visibleRect.height / heightRatio
        }
        Rectangle {
            color: "red"

            property real widthRatio: keyboardLoader.width / deviceScreenRect.width
            property real heightRatio: keyboardLoader.height / deviceScreenRect.height
            x: maliit_geometry.popoverRect.x / widthRatio
            y: maliit_geometry.popoverRect.y / heightRatio
            width:  maliit_geometry.popoverRect.width / widthRatio
            height: maliit_geometry.popoverRect.height / heightRatio
        }
    }

    QtObject {
        id: maliit_geometry
        property rect popoverRect: Qt.rect(0,0,10,20);
        property rect visibleRect: Qt.rect(0,0,700,300);
        property int orientation: 0
        property bool shown: true

        onVisibleRectChanged: console.log("visibleRect is now " + visibleRect);
    }
    QtObject {
        id: maliit_event_handler

        function onKeyPressed(valueToSubmit, action) { console.log("onKeyPressed : " + valueToSubmit + " -> action: " + action); }
        function onKeyReleased(valueToSubmit, action) { console.log("onKeyReleased : " + valueToSubmit + " -> action: " + action); inputtextarea.lastKey = valueToSubmit; keyReleased(); }
        function onWordCandidatePressed(word) { console.log("onWordCandidatePressed : " + word); }
        function onWordCandidateReleased(word) { console.log("onWordCandidateReleased : " + word); inputtextarea.lastKey = word; }

        signal keyReleased();
    }
    QtObject {
        id: maliit_word_engine
        property bool enabled: true
    }
    ListModel {
        id: maliit_wordribbon
        ListElement { word: "first" }
        ListElement { word: "second"}
    }
    QtObject {
        id: maliit_input_method

        signal activateAutocaps();
        signal hide();

        property int contentType: 0 // 0 ->  text, 1 -> number, 2 -> telephone, 3 -> email, 4 -> url
        property bool testEnvironment: true
        property string activeLanguage: "en"
        property string keyboardSize: "M"
        property string keyboardLayout: "LuneOS"
        property variant enabledLanguages: [ "en", "de", "nl", "fr", "sv", "ar", "cs", "da", "es", "fi", "he", "hu", "it", "pl", "pt", "ru" ]
    }

    Rectangle {
        anchors.fill: parent
        anchors.bottomMargin: maliit_geometry.visibleRect.height

        border {
            color: "black"
            width: 10
        }
        color: "midnightblue"
        clip: true

        Flickable {
            anchors.fill: parent
            anchors.margins: 50

            flickableDirection: Flickable.VerticalFlick

            Column {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                Text {
                    id: inputtextarea
                    anchors.horizontalCenter: parent.horizontalCenter
                    property string lastKey: ""
                    text: "Last received text from keyboard: " + lastKey
                    font.bold:true
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "contentType = " + maliit_input_method.contentType
                    onClicked: maliit_input_method.contentType = (maliit_input_method.contentType + 1)%5;
                }
                CheckBox {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "word engine : " + (maliit_word_engine.enabled ? "enabled" : "disabled")
                    checked: maliit_word_engine.enabled
                    onClicked: maliit_word_engine.enabled = !maliit_word_engine.enabled;
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "rotate orientation (current is " + (((!isRotated) && (Settings.displayWidth > Settings.displayHeight)) ? "landscape" : "portrait") + ")";
                    onClicked: testRoot.isRotated = !testRoot.isRotated
                }
                //ExclusiveGroup { id: tabPositionGroup }
                Repeater {
                    id: listSimulatedEnvs
                    model: Settings.testEnvs
                    delegate: RadioButton {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "switch to : " + model.name
                        //exclusiveGroup: tabPositionGroup
                        onClicked: {
                            keyboardLoader.sourceComponent = undefined;
                            Settings.currentTestEnv = index;
                            // reset UI singleton values
                            UI.keyboardSizeChoice = "M";
                            keyboardLoader.sourceComponent = kbdComponent;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: kbdComponent
        App.Keyboard {}
    }
    Loader {
        id: keyboardLoader

        // make it depend on currentTestEnv property binding
        width: isRotated ? Settings.displayHeight : Settings.displayWidth
        height: isRotated ? Settings.displayWidth : Settings.displayHeight

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        sourceComponent: kbdComponent
    }

}

