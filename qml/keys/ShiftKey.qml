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
    width: UI.keyWidth;
    iconNormal: "shift"
    iconShifted: "shift-on"
    iconCapsLock: "shift-lock"

    imgNormal: UI.currentShiftState === "CAPSLOCK" ? UI.imageShiftLockKey : UI.currentShiftState === "SHIFTED" ? UI.imageShiftKey : UI.imageBlackKey
    imgPressed: UI.currentShiftState === "CAPSLOCK" ? UI.imageShiftLockKeyPressed : UI.currentShiftState === "SHIFTED" ? UI.imageShiftKeyPressed : UI.imageBlackKeyPressed
	
    action: "shift"

    MouseArea {
        anchors.fill: parent
        preventStealing: true

        onClicked: {
            if (UI.currentShiftState === "NORMAL")
                UI.currentShiftState = "SHIFTED";

            else if (UI.currentShiftState === "SHIFTED")
                UI.currentShiftState = "NORMAL"

            else if (UI.currentShiftState === "CAPSLOCK")
                UI.currentShiftState = "NORMAL"
        }

        onPressAndHold: {
            UI.currentShiftState = "CAPSLOCK"
            //imgPressed: UI.imageShiftKey
        }

        onDoubleClicked: {
            if (UI.currentShiftState === "SHIFTED")
                UI.currentShiftState = "CAPSLOCK"
        }
    }
}
