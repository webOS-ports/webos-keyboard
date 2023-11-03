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

import keys 1.0
import LunaNext.Common 0.1

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
    opacity: 0
    scale: 0
    transformOrigin: Item.Bottom

    states: [
        State {
            when: shown
            name: "VISIBLE"
            PropertyChanges { target: root; visible: true; opacity: 1; scale: 1 }
        },
        State {
            when: !shown
            name: "HIDDEN"
            PropertyChanges { target: root; visible: false; opacity: 0; scale: 0 }
        }
    ]

    transitions: [
        Transition {
            from: "VISIBLE"; to: "HIDDEN"
            SequentialAnimation {
                NumberAnimation {
                    target: root
                    properties: "opacity,scale"
                    to: 0
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
                PropertyAction { target: root; property: "visible" }
            }
        }
    ]
    
    Row {
        id: borderImageRow
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: Units.gu(9)
        BorderImage {
            id: leftImage
            source: UI.imagePopupBgLeft
            border {left: 21; top: 21; bottom: 36;}
            height: parent.height
            verticalTileMode: BorderImage.Stretch
        }
        BorderImage {
            id: leftMiddleImage
            source: UI.imagePopupBgBetween
            height: parent.height
            border {top: 21; bottom: 36;}
            verticalTileMode: BorderImage.Stretch
            horizontalTileMode: BorderImage.Stretch
            width: Units.gu(0.8)
        }
        BorderImage {
            id: middleImage
            source: UI.imagePopupBgCaret
            border {top: 21; bottom: 36;}
            verticalTileMode: BorderImage.Stretch
            height: parent.height
            width: Units.gu(3.3)
        }
        BorderImage {
            id: rightMiddleImage
            source: UI.imagePopupBgBetween
            border {top: 21; bottom: 36;}
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            height: parent.height
            width: Units.gu(0.8)
        }
        BorderImage {
            id: rightImage
            border {right: 21; top: 21; bottom: 36;}
            source: UI.imagePopupBgRight
            height: parent.height
        }
    }

    Text {
        id: label
        anchors.centerIn: borderImageRow
        anchors.verticalCenterOffset: Units.gu(-0.5)

        font.family: UI.fontFamily
        font.pixelSize: text.length > 1 ? FontUtils.sizeToPixels(UI.popoverFontSizeLong) : FontUtils.sizeToPixels(UI.popoverFontSize)
        font.bold: UI.fontBold
        color: UI.magnifierFontColor
    }
}
