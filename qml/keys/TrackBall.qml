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

Item {
	id: trackBall

    property Item highlightedKey;

    Item {
        id: origin
        anchors.centerIn: parent
        width: slider.width
        height: slider.height
    }

    Image {
        anchors.centerIn: sliderMouseArea.pressed? undefined : parent
		id: slider
        source: "../images/"+UI.formFactor+"/track-center.png"
        width: sourceSize.width * 0.8
        height: sourceSize.height * 0.8
    }

    Image{
        id: leftArrow
        anchors.rightMargin: Units.gu(-0.5)
        anchors.right: origin.left
        anchors.verticalCenter: origin.verticalCenter
        source: "../images/"+UI.formFactor+"/track-arrow-left.png"
        opacity: highlightedKey === leftArrow ? 1.0 : 0.2
    }

    Image{
        id: rightArrow
        anchors.leftMargin: Units.gu(-0.5)
        anchors.left: origin.right
        anchors.verticalCenter: origin.verticalCenter
        source: "../images/"+UI.formFactor+"/track-arrow-right.png"
        opacity: highlightedKey === rightArrow ? 1.0 : 0.2
    }

    Image{
        id: upArrow
        anchors.bottomMargin: Units.gu(-0.5)
        anchors.bottom: origin.top
        anchors.horizontalCenter: origin.horizontalCenter
        source: "../images/"+UI.formFactor+"/track-arrow-up.png"
        opacity: highlightedKey === upArrow ? 1.0 : 0.2
    }

    Image{
        id: downArrow
        anchors.topMargin: Units.gu(-0.5)
        anchors.top: origin.bottom
        anchors.horizontalCenter: origin.horizontalCenter
        source: "../images/"+UI.formFactor+"/track-arrow-down.png"
        opacity: highlightedKey === downArrow ? 1.0 : 0.2
    }

    MouseArea {
        id: sliderMouseArea
        anchors.fill: parent

        function sendKey(key) {
            event_handler.onKeyPressed("", key);
            event_handler.onKeyReleased("", key);
        }

        property real _startX;
        property real _startY;

        onPressed: {
            _startX = mouse.x;
            _startY = mouse.y;
        }
        onPositionChanged: {
            if( mouse.x - _startX > 15 ) {
                highlightedKey = rightArrow;
                sendKey("keyRight");

                _startX = mouse.x;
                _startY = mouse.y;
            }
            else if( mouse.x - _startX  < -15 ) {
                highlightedKey = leftArrow;
                sendKey("keyLeft");

                _startX = mouse.x;
                _startY = mouse.y;
            }
            else if( mouse.y - _startY > 15 ) {
                highlightedKey = downArrow;
                sendKey("keyDown");

                _startX = mouse.x;
                _startY = mouse.y;
            }
            else if( mouse.y - _startY < -15 ) {
                highlightedKey = upArrow;
                sendKey("keyUp");

                _startX = mouse.x;
                _startY = mouse.y;
            }
        }
        onReleased: {
            highlightedKey = null;
        }
    }
}
