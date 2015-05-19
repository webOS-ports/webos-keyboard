/*
 * Copyright 2013 Canonical Ltd.
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
 * Copyright (C) 2015 Herman van Hazendonk <github.com@herrie.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import LunaNext.Common 0.1
import "key_constants.js" as UI

/*!
  Item to show a "bubble" with a text in the center.
  The bottom center is where the bubble points to, and it animates to that position
  when hiding it.
 */
Item {
    id: root

    /*! Text to show in the magnifier */
    property alias text: label.text

    /*! Sets the Magnifier visible or invisible*/
    property bool shown: false

    visible: false

    onShownChanged: {
        if (shown) {
            root.visible = true
            leftImage.animationStep = 1
            leftMiddleImage.animationStep = 1
            middleImage.animationStep = 1
            rightMiddleImage.animationStep = 1
            rightImage.animationStep = 1
        } else {
            hideLeftAnimation.start();
            hideLeftMiddleAnimation.start();
            hideMiddleAnimation.start();
            hideRightMiddleAnimation.start();
            hideRightAnimation.start();
        }
    }

    
	Row {
        id: borderImageRow
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            height: Units.gu(9)
            BorderImage {
				property real animationStep: 0
                id: leftImage
				source: UI.imagePopupBgLeft[formFactor]
                border {left: 21; top: 21; bottom: 36;}
				height: parent.height
                verticalTileMode: BorderImage.Stretch
                scale: animationStep
                transformOrigin: Item.Bottom
                opacity: animationStep

                width: Units.gu(2.1)
            }
            BorderImage {
				property real animationStep: 0
                id: leftMiddleImage
                source: UI.imagePopupBgBetween[formFactor]
                height: parent.height
                border {top: 21; bottom: 36;}
                verticalTileMode: BorderImage.Stretch
                horizontalTileMode: BorderImage.Stretch
                scale: animationStep
                transformOrigin: Item.Bottom
                opacity: animationStep

                width: Units.gu(0.8)
            }
            BorderImage {
				property real animationStep: 0
                id: middleImage
				source: UI.imagePopupBgCaret[formFactor]
                border {top: 21; bottom: 36;}
                verticalTileMode: BorderImage.Stretch
                height: parent.height
                scale: animationStep
                transformOrigin: Item.Bottom
                opacity: animationStep

                width: Units.gu(3.3)
            }
            BorderImage {
				property real animationStep: 0
                id: rightMiddleImage
				source: UI.imagePopupBgBetween[formFactor]
                border {top: 21; bottom: 36;}
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
                height: parent.height
                scale: animationStep
                transformOrigin: Item.Bottom
                opacity: animationStep

                width: Units.gu(0.8)
            }
            BorderImage {
				property real animationStep: 0
                id: rightImage
                border {right: 21; top: 21; bottom: 36;}
				source: UI.imagePopupBgRight[formFactor]
                height: parent.height
                scale: animationStep
                transformOrigin: Item.Bottom
                opacity: animationStep

                width: Units.gu(2.1)
            }
        }
		
        Text {
            id: label
            anchors.centerIn: borderImageRow
			anchors.verticalCenterOffset: Units.gu(-0.5)

            font.family: UI.fontFamily
            font.pixelSize: FontUtils.sizeToPixels(UI.popoverFontSize)
            font.bold: UI.fontBold[formFactor]
            color: UI.magnifierFontColor[formFactor]
        }

        NumberAnimation {
            id: hideLeftAnimation
            target: leftImage
            property: "animationStep"
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
            onStopped: {
                root.visible = false;
            }
        }
        NumberAnimation {
            id: hideLeftMiddleAnimation
            target: leftMiddleImage
            property: "animationStep"
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
            onStopped: {
                root.visible = false;
            }
        }

        NumberAnimation {
            id: hideMiddleAnimation
            target: middleImage
            property: "animationStep"
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
            onStopped: {
                root.visible = false;
            }
        }

        NumberAnimation {
            id: hideRightMiddleAnimation
            target: rightMiddleImage
            property: "animationStep"
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
            onStopped: {
                root.visible = false;
            }
        }

        NumberAnimation {
            id: hideRightAnimation
            target: rightImage
            property: "animationStep"
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
            onStopped: {
                root.visible = false;
            }
        }
}
