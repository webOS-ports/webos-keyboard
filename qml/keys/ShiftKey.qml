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

ActionKey {
    weight: 1
    iconNormal: "shift"
    iconShifted: "shift-on"
    iconCapsLock: "shift-lock"

    pressed: shiftKeyPressArea.isPressed

    imgNormal: UI.currentShiftState === "CAPSLOCK" ? UI.imageShiftLockKey : UI.currentShiftState === "SHIFTED" ? UI.imageShiftKey : UI.imageBlackKey
    imgPressed: UI.currentShiftState === "CAPSLOCK" ? UI.imageShiftLockKeyPressed : UI.currentShiftState === "SHIFTED" ? UI.imageShiftKeyPressed : UI.imageBlackKeyPressed

    action: "shift"

    PressArea {
        id: shiftKeyPressArea
        anchors.fill: parent
        compatibleWithPopover: true
        property bool keySentDuringShiftState: false;

        /* Caps lock is a double tap within DOUBLE_TAP_DURATION, not a long press.
           The unlock case records its time so that a quick tap straight after
           unlocking is swallowed rather than locking again immediately - without
           that, unlocking and carrying on typing re-locks. */
        readonly property int doubleTapDuration: 500
        property double lastShiftTime: 0
        property double lastUnlockTime: 0

        onKeyPressed: {
            keySentDuringShiftState = false; // reset state

            var now = new Date().getTime();

            if (lastUnlockTime + doubleTapDuration > now) {
                // quick tap after unlocking: eat it, and start over
                lastUnlockTime = 0;
                now = 0;
            } else if (lastShiftTime + doubleTapDuration > now) {
                UI.currentShiftState = "CAPSLOCK";
            } else if (UI.currentShiftState === "CAPSLOCK") {
                UI.currentShiftState = "NORMAL";
                lastUnlockTime = now;
            } else if (UI.currentShiftState === "NORMAL") {
                UI.currentShiftState = "SHIFTED";
            } else {
                UI.currentShiftState = "NORMAL";
            }

            lastShiftTime = now;
            UI.isShiftKeyPressed = true;
        }

        onKeyPressedAndHold: {
            if( !keySentDuringShiftState ) {
                UI.currentShiftState = "CAPSLOCK"
            }
        }

        onKeyReleased: {
            // reset state to normal
            if( keySentDuringShiftState && UI.currentShiftState === "SHIFTED" ) {
                UI.currentShiftState = "NORMAL"
            }
        }

        /* There used to be a second double-click path here, on its own 300ms timer,
           racing the one above. The timing above is the reference's
           DOUBLE_TAP_DURATION and it handles the unlock case, so this only keeps the
           part the other handler did not cover: a press arriving from a slide onto
           the shift key still shifts. */
        onPressed: {
            if (afterMove && UI.currentShiftState === "NORMAL")
                UI.currentShiftState = "SHIFTED";

            UI.isShiftKeyPressed = true;
        }
        onReleased: {
            UI.isShiftKeyPressed = false;
        }

        Connections {
            target: UI
            function onShiftedKeySent() {
                if( UI.isShiftKeyPressed ) {
                    shiftKeyPressArea.keySentDuringShiftState = true;
                }
                else {
                    UI.currentShiftState = "NORMAL"
                }
            }
        }
    }
}
