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

	BorderImage {
        anchors.centerIn: parent

		id: slider
        source: "../images/track-center.png"
        width: sourceSize.width * 0.8
        height: sourceSize.height * 0.8

		Image{
			id: leftArrow
            anchors.rightMargin: units.gu(-0.5)
            anchors.right: slider.left
            anchors.verticalCenter: slider.verticalCenter
			source: "../images/track-arrow-left.png"
            opacity: 0.2
		}
		
		Image{
			id: rightArrow
            anchors.leftMargin: units.gu(-0.5)
			anchors.left: slider.right
            anchors.verticalCenter: slider.verticalCenter
			source: "../images/track-arrow-right.png"
            opacity: 0.2
		}

		Image{
			id: upArrow
            anchors.bottomMargin: units.gu(-0.5)
			anchors.bottom: slider.top
            anchors.horizontalCenter: slider.horizontalCenter
			source: "../images/track-arrow-up.png"
            opacity: 0.2
		}

		Image{
			id: downArrow
            anchors.topMargin: units.gu(-0.5)
			anchors.top: slider.bottom
            anchors.horizontalCenter: slider.horizontalCenter
			source: "../images/track-arrow-down.png"
            opacity: 0.2
		}
		
    }
}
