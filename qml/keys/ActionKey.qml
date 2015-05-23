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
import LunaNext.Common 0.1

CharKey {
    id: actionKeyRoot
    property string iconNormal: "transparent"
    property string iconShifted: "transparent"
    property string iconCapsLock: "transparent"

    noMagnifier: true
    skipAutoCaps: true
    property int padding: UI.actionKeyPadding

    // action keys are a bit wider
    width: panel.keyWidth + Units.gu( padding )

    imgNormal: UI.imageBlackKey
    imgPressed: UI.imageBlackKeyPressed

    property string __icon: iconNormal

    // can be overwritten by keys
    property color colorNormal: "transparent"
    property color colorShifted: "transparent"
    property color colorCapsLock: "transparent"

    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property string fontSize: UI.fontSize
	
	
    // was: Icon (the source is an image from the icons directory)
    Image {

        property color color;

        id: iconImage
        source: Qt.resolvedUrl("../images/" + UI.formFactor + "/" + __icon + ".png")
        anchors.centerIn: parent
        anchors.verticalCenterOffset: UI.formFactor === "tablet" ? Units.gu(-0.25) : Units.gu(0.15)
        visible: (label == "")
        height: UI.formFactor === "tablet" ? parent.height * 0.7 : actionKeyRoot.height > actionKeyRoot.width ? parent.height * 0.5 : parent.width * 0.5
	   	smooth: true
		fillMode: Image.PreserveAspectFit
    }

    Text {
        id: keyLabel
        text: (panel.activeKeypadState === "NORMAL") ? label : shifted;
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: action === "return" && UI.formFactor === "tablet" ? Units.gu(2) : 0
        font.family: UI.fontFamily
        font.pixelSize: FontUtils.sizeToPixels(fontSize);
        font.bold: UI.fontBoldAction
        style: Text.Raised
        styleColor: "black"
        color: UI.greyColor
        smooth: true
    }

    onOskStateChanged: {
        if (panel.activeKeypadState === "NORMAL") {
            __icon = iconNormal;
            iconImage.color = colorNormal;
        } else if (panel.activeKeypadState === "SHIFTED") {
            __icon = iconShifted;
            iconImage.color = colorShifted;
        } else if (panel.activeKeypadState === "CAPSLOCK") {
            __icon = iconCapsLock;
            iconImage.color = colorCapsLock;
        }
    }
}
