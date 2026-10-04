/*
 * Copyright (C) 2026 alan-morford <alan-morford@users.noreply.github.com>
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

/*!
  The Palm Pre's orange key. A tap makes the next key type the character
  printed beside its letter (CharKey.alt); a double tap locks that on, and a
  tap while locked releases it - the same rhythm as ShiftKey.
 */
ActionKey {
    weight: 1
    action: "alt"

    pressed: altKeyPressArea.isPressed

    // The printed orange square: lit with a white rim while active, a heavier
    // rim when locked.
    Rectangle {
        id: altSquare
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: width * 1.3
        radius: width * 0.16
        // Orange on "Pre (Orange)"; on "Pre (White)" a white square with a grey
        // rim, so it still reads as lit when active.
        readonly property bool white: UI.preVariant === "white"
        gradient: Gradient {
            GradientStop { position: 0.0; color: altSquare.white ? "#FFFFFF"
                                                              : (UI.currentAltState === "NORMAL" ? "#FF5A40" : "#FF7A60") }
            GradientStop { position: 1.0; color: altSquare.white ? (UI.currentAltState === "NORMAL" ? "#D0D4D6" : "#E8ECEE")
                                                              : (UI.currentAltState === "NORMAL" ? "#E0301E" : "#FF4A30") }
        }
        border.color: white ? "#7A8085" : "white"
        border.width: UI.currentAltState === "ALTLOCK" ? 4 : UI.currentAltState === "ALT" ? 2 : 0
    }

    PressArea {
        id: altKeyPressArea
        anchors.fill: parent
        compatibleWithPopover: true

        readonly property int doubleTapDuration: 500
        property double lastTapTime: 0

        onKeyPressed: {
            var now = new Date().getTime();
            if (UI.currentAltState === "ALTLOCK")
                UI.currentAltState = "NORMAL";
            else if (lastTapTime + doubleTapDuration > now)
                UI.currentAltState = "ALTLOCK";
            else if (UI.currentAltState === "ALT")
                UI.currentAltState = "NORMAL";
            else
                UI.currentAltState = "ALT";
            lastTapTime = UI.currentAltState === "NORMAL" ? 0 : now;
        }
    }
}
