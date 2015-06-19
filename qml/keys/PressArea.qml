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

/*!
  MultiPointTouchArea is similar to the MouseArea
  But to enable multiple PressAreas to be touched at the same time, this is based
  on MultiPointTouchArea

  FIXME this compoment assumes, that only one finger touches this single area at
  the same time.
 */
Item {
    id: root

    objectName: "pressArea"

    property bool onlyExclusive: false         // when true, only a press (and not a move) on that area will trigger it
    property bool compatibleWithPopover: false // when true, won't hide popover if pressed

    // used as slots
    signal pressed(bool afterMove);
    signal released(bool afterMove);
    signal moved();

    /// Same as MouseArea pressAndHold()
    signal keyPressed()
    signal keyPressedAndHold()
    signal keyReleased()

    /// Is true while the area is touched, and the finger did not yet lift
    property bool isPressed: false

    /// Cancels the current pressed state of the mouse are
    function cancelPress() {
        isPressed = false;
        holdTimer.stop();
    }

    Timer {
        id: holdTimer
        interval: 1000
        onTriggered: {
            if (root.isPressed)
                root.keyPressedAndHold();
        }
    }

    onPressed: {
        if( onlyExclusive && afterMove ) return;

        isPressed = true;
        if( !afterMove ) {
            holdTimer.restart();
            root.keyPressed();
        }
    }

    onReleased: {
        if( isPressed )
        {
            isPressed = false;
            holdTimer.stop();
            if( !afterMove ) {
                root.keyReleased();
            }
        }
    }

    onMoved: {
        if( isPressed )
        {
            holdTimer.stop();
        }
    }
}
