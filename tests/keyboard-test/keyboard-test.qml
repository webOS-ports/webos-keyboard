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
import QtQuick.Controls.Basic as B

import LunaNext.Common 0.1
import keys 1.0

import "../../qml" as App

Rectangle {
    id: testRoot

    /* maliit_input_method, maliit_geometry, maliit_event_handler, maliit_word_engine,
       maliit_wordribbon and audioFeedback all arrive as context properties from
       main.cpp, which creates them from Stubs.qml. They have to be context
       properties rather than objects with an id here, because UI.qml is a singleton
       and singletons only see the root context. */

    property bool isRotated: false

    width: 1024
    height: 800

    /*! main.cpp resizes the window to these so the whole simulated screen is
        visible - a portrait profile is taller than the default window. */
    property int wantedWidth: keyboardLoader.width
    property int wantedHeight: keyboardLoader.height

    /*! Defaults if nothing is passed on the command line: the TouchPad profile,
        English, size M, the plain qwerty layout. */
    readonly property int defaultEnv: 4   // tenderloin - 1024x768, gridUnit 10

    Component.onCompleted: {
        Settings.currentTestEnv = stubs.startEnv >= 0 ? stubs.startEnv : defaultEnv;
    }

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


    // The simulated application area above the keyboard.
    Rectangle {
        anchors.fill: parent
        anchors.bottomMargin: maliit_geometry.visibleRect.height

        border { color: "black"; width: 10 }
        color: "#14183a"
        clip: true

        Flickable {
            id: controlFlick
            anchors.fill: parent
            anchors.margins: 12
            anchors.rightMargin: 20
            contentHeight: controlCard.height + 24
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: B.ScrollBar { policy: ScrollBar.AlwaysOn }

            // The controls sit on their own light surface. They used to be default
            // Controls on midnightblue, which meant black text on a dark ground.
            Rectangle {
                id: controlCard
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, 400)
                height: controls.height + 24
                radius: 8
                color: "#eef1f5"
                border { color: "#c3cad3"; width: 1 }

                Column {
                    id: controls
                    y: 12
                    x: 12
                    width: parent.width - 24
                    spacing: 6

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: "#1a1d23"
                        font.bold: true
                        text: stubs.lastKey.length > 0
                              ? "Last key: \u201c" + stubs.lastKey + "\u201d"
                              : "Tap a key"
                    }

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: "#55606d"
                        font.pixelSize: 12
                        text: Settings.currentTestEnvName
                              + "  \u00b7  " + maliit_input_method.activeLanguage
                              + "  \u00b7  size " + maliit_input_method.keyboardSize
                              + "  \u00b7  " + maliit_input_method.keyboardLayout
                              + "  \u00b7  " + UI.formFactor
                              + "  \u00b7  " + Settings.displayWidth + "x" + Settings.displayHeight
                              + " @ gu " + Settings.gridUnit
                    }

                    Rectangle { width: parent.width; height: 1; color: "#d5dae1" }

                    // Language. Switching also drops back to the plain layout: not
                    // every language has a Dvorak or Thumb file, and asking for one
                    // that does not exist just loads nothing.
                    ComboBox {
                        id: langCombo
                        width: parent.width
                        implicitHeight: 32
                        model: stubs.allLanguages
                        currentIndex: Math.max(0, stubs.allLanguages.indexOf(
                                                     maliit_input_method.activeLanguage))
                        onActivated: {
                            maliit_input_method.keyboardLayout = "LuneOS";
                            maliit_input_method.activeLanguage = stubs.allLanguages[currentIndex];
                        }
                        contentItem: Text {
                            leftPadding: 10
                            text: "Language: " + langCombo.displayText
                            color: "#1a1d23"
                            verticalAlignment: Text.AlignVCenter
                        }
                        delegate: ItemDelegate {
                            width: langCombo.width
                            contentItem: Text {
                                text: modelData
                                color: "#1a1d23"
                                verticalAlignment: Text.AlignVCenter
                            }
                            highlighted: langCombo.highlightedIndex === index
                        }
                    }

                    Button {
                        width: parent.width
                        implicitHeight: 30
                        enabled: __layouts.length > 1
                        readonly property var __layouts:
                            stubs.altLayouts[maliit_input_method.activeLanguage] !== undefined
                            ? stubs.altLayouts[maliit_input_method.activeLanguage]
                            : [ "LuneOS" ]
                        text: __layouts.length > 1
                              ? "Layout: " + maliit_input_method.keyboardLayout
                              : "Layout: LuneOS (no alternatives)"
                        onClicked: {
                            var i = __layouts.indexOf(maliit_input_method.keyboardLayout);
                            maliit_input_method.keyboardLayout = __layouts[(i + 1) % __layouts.length];
                        }
                    }

                    CheckBox {
                        width: parent.width
                        text: "Several languages enabled (shows the language key)"
                        checked: maliit_input_method.enabledLanguages.length > 1
                        onClicked: maliit_input_method.enabledLanguages =
                                   checked ? stubs.allLanguages : [ "en" ]
                        contentItem: Text {
                            text: parent.text
                            color: "#1a1d23"
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            leftPadding: parent.indicator.width + parent.spacing
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: "#d5dae1" }

                    Button {
                        width: parent.width
                        implicitHeight: 30
                        text: "Content type: " + [ "text", "number", "telephone",
                                                   "email", "url" ][maliit_input_method.contentType]
                        onClicked: maliit_input_method.contentType =
                                   (maliit_input_method.contentType + 1) % 5
                    }

                    Button {
                        width: parent.width
                        implicitHeight: 30
                        text: "Keyboard size: " + maliit_input_method.keyboardSize
                        onClicked: {
                            var sizes = UI.keyboardSizeChoices;
                            var i = sizes.indexOf(maliit_input_method.keyboardSize);
                            maliit_input_method.keyboardSize = sizes[(i + 1) % sizes.length];
                        }
                    }

                    Button {
                        width: parent.width
                        implicitHeight: 30
                        text: "Orientation: " + (((!isRotated) && (Settings.displayWidth > Settings.displayHeight))
                                                 ? "landscape" : "portrait")
                        onClicked: testRoot.isRotated = !testRoot.isRotated
                    }

                    CheckBox {
                        width: parent.width
                        text: "Word engine"
                        checked: maliit_word_engine.enabled
                        onClicked: maliit_word_engine.enabled = !maliit_word_engine.enabled
                        contentItem: Text {
                            text: parent.text
                            color: "#1a1d23"
                            leftPadding: parent.indicator.width + parent.spacing
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: "#d5dae1" }

                    Text {
                        color: "#55606d"
                        font.pixelSize: 12
                        text: "Device profile"
                    }

                    Repeater {
                        model: Settings.testEnvs
                        delegate: RadioButton {
                            width: controls.width
                            implicitHeight: 26
                            text: model.name + "  (" + model.displayWidth + "x"
                                  + model.displayHeight + ", gu " + model.gridUnit
                                  + ", " + (model.tabletUi ? "tablet" : "phone") + ")"
                            checked: Settings.currentTestEnv === index
                            onClicked: {
                                keyboardLoader.sourceComponent = undefined;
                                Settings.currentTestEnv = index;
                                UI.keyboardSizeChoice = "M";
                                keyboardLoader.sourceComponent = kbdComponent;
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "#1a1d23"
                                font.pixelSize: 13
                                leftPadding: parent.indicator.width + parent.spacing
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }
        }
    }

    /* The keyboard reports its own height through maliit_geometry. If that stays at
       nothing, the pad failed to build - almost always a QML error in the layout or
       in one of the key components, which the console will have printed. Say so on
       screen rather than showing an empty strip. */
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 64
        z: 100
        color: "#7a1d16"
        visible: keyboardLoader.status === Loader.Ready
                 && maliit_geometry.visibleRect.height < 40

        Text {
            anchors.centerIn: parent
            width: parent.width - 40
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: "white"
            font.bold: true
            text: "The keyboard built with no height (" + maliit_geometry.visibleRect.height
                  + "px). Check the console for a QML error in the layout or in qml/keys."
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

