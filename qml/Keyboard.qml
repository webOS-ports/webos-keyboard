/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) 2012 Openismus GmbH
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
 * Copyright (C) 2015 Herman van Hazendonk <github.com@herrie.org>
 *
 * Contact: maliit-discuss@lists.maliit.org
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

import QtQuick 2.0
import keys 1.0

import LunaNext.Common 0.1

Item {
    id: fullScreenItem

    property variant input_method: maliit_input_method
    property variant event_handler: maliit_event_handler

    Item {
        id: canvas

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: keyboardSurface.height
        property int keypadHeight: height;

        visible: true

        property int contentOrientation: maliit_geometry.orientation
        onContentOrientationChanged: fullScreenItem.reportKeyboardVisibleRect();

        property bool languageMenuShown: false
        property bool keyboardSizeMenuShown: false

        onXChanged: fullScreenItem.reportKeyboardVisibleRect();
        onYChanged: fullScreenItem.reportKeyboardVisibleRect();
        onWidthChanged: fullScreenItem.reportKeyboardVisibleRect();
        onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

        MouseArea {
            id: swipeArea

            property int jumpBackThreshold: Units.gu(10)

            enabled: UI.formFactor === "phone" && !UI.extendedKeysShown

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: (parent.height - keypad.height) + wordRibbon.height

            drag.target: keyboardSurface
            drag.axis: Drag.YAxis;
            drag.minimumY: 0
            drag.maximumY: parent.height
            drag.filterChildren: true

            onReleased: {
                if (keyboardSurface.y > jumpBackThreshold) {
                    maliit_geometry.shown = false;
                } else {
                    bounceBackAnimation.from = keyboardSurface.y
                    bounceBackAnimation.start();
                }
            }

            Item {
                id: keyboardSurface
                objectName: "keyboardSurface"

                x:0
                y:0
                width: parent.width
                height: wordRibbon.height + keyboardComp.height

                onXChanged: fullScreenItem.reportKeyboardVisibleRect();
                onYChanged: fullScreenItem.reportKeyboardVisibleRect();
                onWidthChanged: fullScreenItem.reportKeyboardVisibleRect();
                onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

                WordRibbon {
                    id: wordRibbon
                    objectName: "wordRibbon"

                    visible: maliit_word_engine.enabled

                    anchors.bottom: keyboardComp.top
                    width: parent.width;

                    height: visible ? Units.gu(UI.wordribbonHeight) : 0
                }

                Item {
                    id: keyboardComp
                    objectName: "keyboardComp"

                    height: keyboardCompColumn.height
                    width: parent.width
                    anchors.bottom: parent.bottom

                    onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

                    Image {
                        id: background

                        anchors.fill: parent
                        source: "images/"+UI.formFactor+"/keyboard-bg.png"
                        fillMode: Image.TileHorizontally
                    }

                    Column {
                        id: keyboardCompColumn
                        width: parent.width

                        Image {
                            source: "images/"+UI.formFactor+"/border_top.png"
                            width: parent.width
                        }
                        Item {
                            width: parent.width
                            height: Units.gu( UI.top_margin )
                        }
                        KeyboardContainer {
                            id: keypad
                            width: parent.width
                        }
                        Item {
                            width: parent.width
                            height: Units.gu( UI.bottom_margin )
                        }
                        Image {
                            source: "images/"+UI.formFactor+"/border_bottom.png"
                            width: parent.width
                        }
                    }
                } // keyboardComp
            }
        }

        PropertyAnimation {
            id: bounceBackAnimation
            target: keyboardSurface
            properties: "y"
            easing.type: Easing.OutBounce;
            easing.overshoot: 2.0
            to: 0
        }

        state: "HIDDEN"

        states: [
            State {
                name: "SHOWN"
                PropertyChanges { target: canvas; y: 0; }
                when: maliit_geometry.shown === true
            },

            State {
                name: "HIDDEN"
                PropertyChanges { target: canvas; y: height; }
                onCompleted: {
                    keyboardSurface.y = 0;
                    UI.hideCurrentPopover();
                    UI.currentShiftState = "NORMAL";
                    UI.currentSymbolState = "CHARACTERS";
                    maliit_input_method.hide();
                }
                when: maliit_geometry.shown === false
            }
        ]
        transitions: Transition {
            PropertyAnimation { target: canvas; properties: "y"; easing.type: Easing.InOutQuad }
        }

        Connections {
            target: input_method
            function onActivateAutocaps() {
                keypad.state = "CHARACTERS";
                UI.currentShiftState = "SHIFTED";
            }
        }
    } // canvas

    // calculates the size of the visible keyboard to report to the window system
    // FIXME get the correct size for enabled extended keys instead of that big area
    function reportKeyboardVisibleRect() {

        var vx = 0;
        var vy = wordRibbon.y;
        var vwidth = keyboardSurface.width;
        var vheight = keyboardComp.height + wordRibbon.height;

        var obj = fullScreenItem.mapFromItem(keyboardSurface, vx, vy, vwidth, vheight);
        maliit_geometry.visibleRect = Qt.rect(obj.x, obj.y, obj.width, obj.height);
    }

    Component.onCompleted: {
        UI.isLandscape = Qt.binding( function() { return fullScreenItem.width > fullScreenItem.height; } );
    }
} // fullScreenItem
