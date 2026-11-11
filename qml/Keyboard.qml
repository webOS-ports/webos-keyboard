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
import QtQuick.Window 2.2
import QtMultimedia 6.3
import keys 1.0
import LuneOSKeyboard 1.0

import LunaNext.Common 0.1

Item {
    id: fullScreenItem

    property variant input_method: maliit_input_method
    property variant event_handler: maliit_event_handler

    //! The Pre Keyboard is the phone keyboard; a tablet keeps LuneOS's own
    //! layouts. With a physical keyboard in use the panel is the candidate
    //! strip, which is also where the keys are brought back from.
    readonly property bool preShown: pre_keyboard.active && !fullScreenItem.input_method.keysCollapsed

    Binding {
        target: pre_keyboard
        property: "active"
        value: UI.formFactor === "phone"
    }

    Item {
        id: canvas

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: fullScreenItem.preShown ? preKeyboard.height : keyboardSurface.height
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

            // Stays enabled once the keys are gone, because the drag then works
            // the other way: up over the candidate bar to bring them back. The
            // bar is the only affordance for that which does not cost the field
            // its focus - the shell's toggle is behind the system menu, and
            // opening the menu takes input focus off the field, leaving nothing
            // for the panel to come back for.
            enabled: UI.formFactor === "phone" && !UI.extendedKeysShown && !fullScreenItem.preShown
            visible: !fullScreenItem.preShown

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: keyboardComp.visible ? (parent.height - keypad.height) + wordRibbon.height
                                         : wordRibbon.height

            drag.target: keyboardSurface
            drag.axis: Drag.YAxis;
            // Down to dismiss while the keys are up, up to bring them back while
            // they are not. One direction at a time, so neither gesture can be
            // started in the direction that has nothing to do.
            // The floor rather than keypad.height alone: the keys have to be
            // draggable further than jumpBackThreshold for the gesture to count,
            // and a keypad that has not been laid out yet reports no height.
            drag.minimumY: keyboardComp.visible ? 0
                                               : -Math.max(keypad.height, Units.gu(20))
            drag.maximumY: keyboardComp.visible ? parent.height : 0
            drag.filterChildren: true

            onReleased: {
                if (keyboardComp.visible) {
                    if (keyboardSurface.y > jumpBackThreshold) {
                        // Drags the keys away and leaves the candidate bar, rather
                        // than taking the whole panel down: the suggestions are
                        // still wanted. collapseKeys() decides - where the word
                        // engine is off there is no bar to be left with, and it
                        // dismisses the panel instead.
                        fullScreenItem.input_method.collapseKeys();
                    }
                } else if (keyboardSurface.y < -jumpBackThreshold) {
                    fullScreenItem.input_method.expandKeys();
                }

                // Always: the surface has been dragged off where it belongs and
                // has to come back, whether the drag decided anything or not.
                bounceBackAnimation.from = keyboardSurface.y
                bounceBackAnimation.start();
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

                    //! And while the emoji panel is up, whether or not the
                    //! word engine is on: the bar carries the button that puts
                    //! the panel away again.
                    visible: maliit_word_engine.enabled || wordRibbon.emojiShown

                    anchors.bottom: keyboardComp.top
                    width: parent.width;

                    height: visible ? Units.gu(UI.wordribbonHeight) : 0
                }

                Item {
                    id: keyboardComp
                    objectName: "keyboardComp"

                    // Nothing to draw when the user is typing on a physical
                    // keyboard: the panel is then just the candidate bar above,
                    // and keyboardSurface collapses to its height, which is what
                    // the application is told to leave room for.
                    visible: !fullScreenItem.input_method.keysCollapsed

                    height: visible ? keyboardCompColumn.height : 0
                    width: parent.width
                    anchors.bottom: parent.bottom

                    onHeightChanged: fullScreenItem.reportKeyboardVisibleRect();

                    Image {
                        id: background

                        anchors.fill: parent
                        source: "images/"+UI.imageDir+"/keyboard-bg.png"
                        fillMode: Image.TileHorizontally
                    }

                    Column {
                        id: keyboardCompColumn
                        width: parent.width

                        Image {
                            source: "images/"+UI.imageDir+"/border_top.png"
                            width: parent.width
                        }
                        Item {
                            width: parent.width
                            height: Units.gu( UI.top_margin )
                        }
                        KeyboardContainer {
                            id: keypad
                            width: parent.width
                            // Its own height binding is left alone; an
                            // invisible item takes no room in the Column but
                            // still reports the height it would have, which is
                            // what the panel below sizes itself from.
                            visible: !wordRibbon.emojiShown
                        }

                        /*!
                         * The emoji panel takes the keys' place rather than
                         * appearing over them, so the application is asked to
                         * leave room for the same rectangle either way and the
                         * panel's area does not have to be re-reported as it
                         * opens.
                         */
                        EmojiPanel {
                            width: parent.width
                            height: visible ? keypad.height : 0
                            visible: wordRibbon.emojiShown

                            onDismissed: wordRibbon.emojiShown = false
                        }
                        Item {
                            width: parent.width
                            height: Units.gu( UI.bottom_margin )
                        }
                        Image {
                            source: "images/"+UI.imageDir+"/border_bottom.png"
                            width: parent.width
                        }
                    }
                } // keyboardComp
            }
        }

        /*!
         * The Pre Keyboard: the Palm Pre's keys, drawn and run in C++
         * (src/pre). It is taller than its keys by the room its popups need;
         * only the keys are reported as the panel, and a popup is added to the
         * window's input mask while it is up.
         */
        PreKeyboardView {
            id: preKeyboard
            visible: fullScreenItem.preShown
            controller: visible ? pre_keyboard : null
            anchors.bottom: parent.bottom
            width: parent.width
            height: headroom + keysHeight
            // The Android Pre Keyboard's proportions on any screen: its sizes are
            // dp on a phone 411 dp wide (1080 px at 2.625), so a key is a tenth of
            // the width and a Medium row 54 dp, 1.31 times as tall as a key is wide.
            // From the screen's dpi instead, the KEY2's 4.5" 1080 px screen came out
            // at 2.41 and its keys squatter than Android's.
            // The screen's, not the panel window's: the window is only as tall as the panel.
            dp: Math.min(Screen.width, Screen.height) / 411.43
            landscape: UI.isLandscape

            onKeysHeightChanged: fullScreenItem.reportKeyboardVisibleRect();
            onYChanged: fullScreenItem.reportKeyboardVisibleRect();
            onVisibleChanged: {
                fullScreenItem.reportKeyboardVisibleRect();
                fullScreenItem.reportPrePopup();
            }
            onPopupRectChanged: fullScreenItem.reportPrePopup();
            onKeyClick: preClick.play()

            // The keyboard's own click (KeyboardContainer's audioFeedback), on
            // the same "key press feedback" setting.
            MediaPlayer {
                id: preClick
                source: Qt.resolvedUrl("styles/ubuntu/sounds/key_tick2_quiet.wav")
                audioOutput: AudioOutput {}
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

            //! A key on the physical keyboard asked for the panel - the Q25's
            //! Sym, tapped on its own. The keys have to come back first: with a
            //! physical keyboard they are not on screen, and the panel goes
            //! where they would be.
            function onEmojiPanelRequested() {
                fullScreenItem.input_method.expandKeys();
                wordRibbon.emojiShown = true;
            }

            //! A capital is for a letter, so back to the letters first: the
            //! symbols page keeps shift until it is tapped off, and shifting
            //! it turned the tablet's number row into !@# for good. The page
            //! has been UI.currentSymbolState since the UI singleton came in;
            //! the keypad's own state, which this used to set, no longer
            //! picks it. Switching pages resets shift, so this goes first.
            function onActivateAutocaps() {
                UI.currentSymbolState = "CHARACTERS";
                UI.currentShiftState = "SHIFTED";
            }
        }
    } // canvas

    // calculates the size of the visible keyboard to report to the window system
    // FIXME get the correct size for enabled extended keys instead of that big area
    function reportKeyboardVisibleRect() {

        if (fullScreenItem.preShown) {
            var keys = fullScreenItem.mapFromItem(preKeyboard, 0, preKeyboard.headroom,
                                                  preKeyboard.width, preKeyboard.keysHeight);
            maliit_geometry.visibleRect = Qt.rect(keys.x, keys.y, keys.width, keys.height);
            return;
        }

        var vx = 0;
        var vy = wordRibbon.y;
        var vwidth = keyboardSurface.width;
        var vheight = keyboardComp.height + wordRibbon.height;

        var obj = fullScreenItem.mapFromItem(keyboardSurface, vx, vy, vwidth, vheight);
        maliit_geometry.visibleRect = Qt.rect(obj.x, obj.y, obj.width, obj.height);
    }

    //! The Pre Keyboard's popup (accents, the globe's menu), for the window's input mask.
    function reportPrePopup() {
        var r = preKeyboard.popupRect;
        if (!fullScreenItem.preShown || r.width <= 0 || r.height <= 0) {
            maliit_geometry.popoverRect = Qt.rect(0, 0, 0, 0);
            return;
        }
        var m = fullScreenItem.mapFromItem(preKeyboard, r.x, r.y, r.width, r.height);
        maliit_geometry.popoverRect = Qt.rect(m.x, m.y, m.width, m.height);
    }

    onPreShownChanged: reportKeyboardVisibleRect();

    Component.onCompleted: {
        UI.isLandscape = Qt.binding( function() { return fullScreenItem.width > fullScreenItem.height; } );
    }
} // fullScreenItem
