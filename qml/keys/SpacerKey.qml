/*
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
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

/*!
  An invisible key - webOS spells it as a negative weight.

  keyboardToKeyZone() returns -1 for a negative-weight key, so the renderer skips
  it and a real gap appears in the row. pointToKeyboard() then looks at the eight
  neighbours for a visible key carrying the same keycode and reports that instead,
  which is how the half-key inset beside Shift and Backspace stays comfortable to
  hit without being drawn.

  Set \l forwardTo to the neighbouring key that should receive the touch.
 */
Item {
    id: spacer

    /*! Width in row units. Sign is irrelevant; the row sums absolute weights. */
    property real weight: 0.5

    /*! Assigned by KeyRow. */
    property real keyUnit: UI.keyWidth

    /*! Key the touches belong to. Leave null to swallow them. */
    property Item forwardTo: null

    width: keyUnit * Math.abs(weight)
    height: parent ? parent.height : 0

    Item {
        objectName: "pressArea"
        anchors.fill: parent

        // Same slots MultiTouchKeyArea invokes on a real PressArea.
        signal pressed(bool afterMove)
        signal released(bool afterMove)
        signal moved()

        function target() {
            return spacer.forwardTo ? spacer.forwardTo.pressArea : null;
        }

        onPressed: { var t = target(); if (t) t.pressed(afterMove); }
        onReleased: { var t = target(); if (t) t.released(afterMove); }
        onMoved: { var t = target(); if (t) t.moved(); }
    }
}
