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
import "constants.js" as Const
import "keys/"
import "keys/key_constants.js" as UI

import LunaNext.Common 0.1

Item {
    id: fullScreenItem
    objectName: "fullScreenItem"

    property variant input_method: maliit_input_method
    property variant event_handler: maliit_event_handler

    property string formFactor: Settings.tabletUi ? "tablet" : "phone"

    property QtObject units: QtObject {
        function gu(valueInGridUnits) {
            return Units.gu(valueInGridUnits);
        }
        function dp(valueInLogicalPixels) {
            return Units.dp(valueInLogicalPixels);
        }
    }
    property QtObject i18n: QtObject {
        function tr(inputString) {
            return inputString;
        }
    }

    // When the keyboard initializes, the root QML element returns a zero size.
    // So if no further geometry change event is sent, we might end up with a
    // keyboard of height=0.
    // Therefore we need to survey the root width and height properties to force
    // recompute once the Screen singleton is actually initialized with proper values.
    property real rootItemSizeSurvey: fullScreenItem.width+fullScreenItem.height
    onRootItemSizeSurveyChanged: calculateSize();

Item {
    id: canvas
    objectName: "ubuntuKeyboard" // Allow us to specify a specific keyboard within autopilot.

    anchors.bottom: parent.bottom
    anchors.left: parent.left

    width: parent.width
    height: 0

    property int keypadHeight: height;

    onRotationChanged: console.log("now rotation has changed!!" + rotation)

    visible: true


    property int contentOrientation: maliit_geometry.orientation
    onContentOrientationChanged: fullScreenItem.reportKeyboardVisibleRect();

    property bool wordribbon_visible: maliit_word_engine.enabled
    onWordribbon_visibleChanged: calculateSize();

    property bool languageMenuShown: false
	property bool keyboardSizeMenuShown: false

    onXChanged: fullScreenItem.reportKeyboardVisibleRect();
    onYChanged: fullScreenItem.reportKeyboardVisibleRect();
    onWidthChanged: fullScreenItem.reportKeyboardVisibleRect();
    onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

    MouseArea {
        id: swipeArea

        property int jumpBackThreshold: units.gu(10)

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: (parent.height - canvas.keypadHeight) + wordRibbon.height +
                borderTop.height + units.gu(UI.top_margin) * 3

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
            height: canvas.height

            onXChanged: fullScreenItem.reportKeyboardVisibleRect();
            onYChanged: fullScreenItem.reportKeyboardVisibleRect();
            onWidthChanged: fullScreenItem.reportKeyboardVisibleRect();
            onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

            WordRibbon {
                id: wordRibbon
                objectName: "wordRibbon"

                visible: canvas.wordribbon_visible

                anchors.bottom: keyboardComp.top
                width: parent.width;

                height: canvas.wordribbon_visible ? UI.wordribbonHeight : 0
                onHeightChanged: calculateSize();
            }

            Item {
                id: keyboardComp
                objectName: "keyboardComp"

                height: canvas.keypadHeight - wordRibbon.height
                width: parent.width
                anchors.bottom: parent.bottom

                onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

                Image {
                    id: background

                    anchors.fill: parent
                    source: "images/"+formFactor+"/keyboard-bg.png"
                    fillMode: Image.TileHorizontally
                }

                Image {
                    id: borderTop
                    source: "images/"+formFactor+"/border_top.png"
                    width: parent.width
                    anchors.top: parent.top.bottom
                }

                Image {
                    id: borderBottom
                    source: "images/"+formFactor+"/border_bottom.png"
                    width: parent.width
                    anchors.bottom: background.bottom
                }

                KeyboardContainer {
                    id: keypad

                    anchors.top: borderTop.bottom
                    anchors.bottom: borderBottom.top
                    anchors.topMargin: units.gu( UI.top_margin )
                    anchors.bottomMargin: units.gu( UI.bottom_margin )
                    width: parent.width

                    onPopoverEnabledChanged: fullScreenItem.reportKeyboardVisibleRect();
                }

                LanguageMenu {
                    id: languageMenu
                    anchors.centerIn: parent
                    width: 400;
                    height: keypad.height;
                    enabled: canvas.languageMenuShown
                    opacity: canvas.languageMenuShown ? 1.0 : 0.0
                }
				
				KeyboardSizeMenu {
                    id: keyboardSizeMenu
                    anchors.centerIn: parent
                    //anchors.bottom: dismissKey.top
                    width: 400;
                    height: keypad.height;
                    enabled: canvas.keyboardSizeMenuShown
                    opacity: canvas.keyboardSizeMenuShown ? 1.0 : 0.0
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
                canvas.languageMenuShown = false;
                keyboardSurface.y = 0;
                keypad.closeExtendedKeys();
                keypad.activeKeypadState = "NORMAL";
                keypad.state = "CHARACTERS";
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
        onActivateAutocaps: {
            keypad.state = "CHARACTERS";
            keypad.activeKeypadState = "SHIFTED";
        }
    }

} // canvas

function calculateSize()
{
    // warning: the following line is wrong if the vkb height doesn't always cover the screen current height
    var isLandscape = (fullScreenItem.width > fullScreenItem.height);

    // TODO add tablet ratios
    var newHeight;
    if( Settings.tabletUi ) {
        if( isLandscape ) {
            newHeight = fullScreenItem.height * UI.tabletKeyboardHeightLandscapeL
        }
        else {
            newHeight = fullScreenItem.height * UI.tabletKeyboardHeightPortrait
        }
    }
    else {
        if( isLandscape ) {
            newHeight = fullScreenItem.height * UI.phoneKeyboardHeightLandscape
        }
        else {
            newHeight = fullScreenItem.height * UI.phoneKeyboardHeightPortrait
        }
    }

    canvas.height = newHeight + wordRibbon.height;

    reportKeyboardVisibleRect();
}

// calculates the size of the visible keyboard to report to the window system
// FIXME get the correct size for enabled extended keys instead of that big area
function reportKeyboardVisibleRect() {

    var vx = 0;
    var vy = wordRibbon.y;
    var vwidth = keyboardSurface.width;
    var vheight = keyboardComp.height + wordRibbon.height;

    // height of the drawable region (can be bigger than the height dedicated to the clickable region)
    maliit_geometry.canvasHeight = keyboardComp.height + Math.max(wordRibbon.height, keypad.keyHeight);;

    var obj = fullScreenItem.mapFromItem(keyboardSurface, vx, vy, vwidth, vheight);
    maliit_geometry.visibleRect = Qt.rect(obj.x, obj.y, obj.width, obj.height);
}

} // fullScreenItem
