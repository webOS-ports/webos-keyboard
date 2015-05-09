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

import "key_constants.js" as UI

ActionKey {
    width: panel.keyWidth;
    iconNormal: "shift"
    iconShifted: "shift-on"
    iconCapsLock: "shift-lock"

    imgNormal: panel.activeKeypadState === "CAPSLOCK" ? UI.imageShiftLockKey[formFactor] : panel.activeKeypadState === "SHIFTED" ? UI.imageShiftKey[formFactor] : UI.imageBlackKey[formFactor]
    imgPressed: panel.activeKeypadState === "CAPSLOCK" ? UI.imageShiftLockKeyPressed[formFactor] : panel.activeKeypadState === "SHIFTED" ? UI.imageShiftKeyPressed[formFactor] : UI.imageBlackKeyPressed[formFactor]
	
    //colorShifted: "#4B97DE"
	colorCapsLock: "#4B97DE"

    action: "shift"

    MouseArea {
        anchors.fill: parent
        preventStealing: true

        onClicked: {
            if (panel.activeKeypadState === "NORMAL")
                panel.activeKeypadState = "SHIFTED";

            else if (panel.activeKeypadState === "SHIFTED")
                panel.activeKeypadState = "NORMAL"

            else if (panel.activeKeypadState === "CAPSLOCK")
                panel.activeKeypadState = "NORMAL"
        }

        onPressAndHold: {
            panel.activeKeypadState = "CAPSLOCK"
            //imgPressed: UI.imageShiftKey[formFactor]
        }

        onDoubleClicked: {
            if (panel.activeKeypadState === "SHIFTED")
                panel.activeKeypadState = "CAPSLOCK"
        }
    }
}
