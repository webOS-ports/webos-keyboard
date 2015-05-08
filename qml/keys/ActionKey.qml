/*
 * Copyright 2013 Canonical Ltd.
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

CharKey {
    id: actionKeyRoot
    property string iconNormal: "transparent"
    property string iconShifted: "transparent"
    property string iconCapsLock: "transparent"

    noMagnifier: true
    skipAutoCaps: true
    property int padding: UI.actionKeyPadding

    // action keys are a bit wider
    width: panel.keyWidth + units.gu( padding )

    imgNormal: UI.imageBlackKey
    imgPressed: UI.imageBlackKeyPressed

    property string __icon: iconNormal

    // can be overwritten by keys
    property color colorNormal: "transparent"
    property color colorShifted: "transparent"
    property color colorCapsLock: "transparent"

    // was: Icon (the source is an image from the icons directory)
    Image {

        property color color;

        id: iconImage
        source: Qt.resolvedUrl("../images/" + __icon + ".png")
        anchors.centerIn: parent
        visible: (label == "")
        width: units.gu(2.5)
        height: units.gu(2.5)
    }

    Text {
        id: keyLabel
        text: (panel.activeKeypadState === "NORMAL") ? label : shifted;
        anchors.centerIn: parent
        font.family: UI.fontFamily
        font.pixelSize: fontSize
        font.bold: UI.fontBold
        color: UI.greyColor
    }

    onOskStateChanged: {
        if (panel.activeKeypadState == "NORMAL") {
            __icon = iconNormal;
            iconImage.color = colorNormal;
        } else if (panel.activeKeypadState == "SHIFTED") {
            __icon = iconShifted;
            iconImage.color = colorShifted;
        } else if (panel.activeKeypadState == "CAPSLOCK") {
            __icon = iconCapsLock;
            iconImage.color = colorCapsLock;
        }
    }
}
