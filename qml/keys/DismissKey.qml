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

ActionKey {
    iconNormal: "icon-hide-keyboard"
    iconShifted: "icon-hide-keyboard"
    iconCapsLock: "icon-hide-keyboard"

    width: panel.keyWidth;

    Image {

        property color color;

        id: iconImage
        source: Qt.resolvedUrl("../images/" + __icon + ".png")
        anchors.centerIn: parent
        visible: (label == "")
        width: units.gu(2.5)
        height: units.gu(2.5)
    }

    MouseArea {
        anchors.fill: parent
        preventStealing: true

        onClicked: {
            maliit_geometry.shown = false;
        }

    }
}
