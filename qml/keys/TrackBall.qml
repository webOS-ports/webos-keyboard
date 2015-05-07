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

Item {
	id: trackBall
	width: panel.keyWidth * 2
	anchors.verticalCenter: parent.verticalCenter

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
        source: "../images/track-center.png"
        width: sourceSize.width * 0.8
        height: sourceSize.height * 0.8

        MouseArea {
            id: sliderMouseArea
            anchors.fill: slider

            drag.target: slider
            drag.axis: Drag.XAndYAxis
            drag.minimumX: leftArrow.x
            drag.maximumX: rightArrow.x + rightArrow.width - slider.width
            drag.minimumY: upArrow.y
            drag.maximumY: downArrow.y + downArrow.height - slider.height
            drag.threshold: 0

            property string _currentKey: "";
            function sendKey(key) {
                if( _currentKey !== "" ) {
                    event_handler.onKeyReleased("", _currentKey);
                }
                if( key !== "" ) {
                    event_handler.onKeyPressed("", key);
                }
                _currentKey = key;
            }

            onPositionChanged: {
                var mousePosInTrackBall = trackBall.mapFromItem(sliderMouseArea, mouse.x, mouse.y);
                if( mousePosInTrackBall.x-5 > rightArrow.x ) {
                    highlightedKey = rightArrow;
                    sendKey("keyRight");
                }
                else if( mousePosInTrackBall.x+5 < leftArrow.x + leftArrow.width ) {
                    highlightedKey = leftArrow;
                    sendKey("keyLeft");
                }
                else if( mousePosInTrackBall.y-5 > downArrow.y ) {
                    highlightedKey = downArrow;
                    sendKey("keyDown");
                }
                else if( mousePosInTrackBall.y+5 < upArrow.y + upArrow.height ) {
                    highlightedKey = upArrow;
                    sendKey("keyUp");
                }
                else {
                    highlightedKey = null;
                    sendKey("");
                }
            }
            onReleased: {
                highlightedKey = null;
                sendKey("");
            }
        }
    }

    Image{
        id: leftArrow
        anchors.rightMargin: units.gu(-0.5)
        anchors.right: origin.left
        anchors.verticalCenter: origin.verticalCenter
        source: "../images/track-arrow-left.png"
        opacity: highlightedKey === leftArrow ? 1.0 : 0.2
    }

    Image{
        id: rightArrow
        anchors.leftMargin: units.gu(-0.5)
        anchors.left: origin.right
        anchors.verticalCenter: origin.verticalCenter
        source: "../images/track-arrow-right.png"
        opacity: highlightedKey === rightArrow ? 1.0 : 0.2
    }

    Image{
        id: upArrow
        anchors.bottomMargin: units.gu(-0.5)
        anchors.bottom: origin.top
        anchors.horizontalCenter: origin.horizontalCenter
        source: "../images/track-arrow-up.png"
        opacity: highlightedKey === upArrow ? 1.0 : 0.2
    }

    Image{
        id: downArrow
        anchors.topMargin: units.gu(-0.5)
        anchors.top: origin.bottom
        anchors.horizontalCenter: origin.horizontalCenter
        source: "../images/track-arrow-down.png"
        opacity: highlightedKey === downArrow ? 1.0 : 0.2
    }
}
