/*
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

MultiPointTouchArea {
    property Item keyRootItem;

    signal pressOnNoKeyArea();
    signal releaseOnNoKeyArea();

    touchPoints: [
        KeyTouchPoint { id: touchPoint0 },
        KeyTouchPoint { id: touchPoint1 },
        KeyTouchPoint { id: touchPoint2 },
        KeyTouchPoint { id: touchPoint3 },
        KeyTouchPoint { id: touchPoint4 },
        KeyTouchPoint { id: touchPoint5 },
        KeyTouchPoint { id: touchPoint6 },
        KeyTouchPoint { id: touchPoint7 },
        KeyTouchPoint { id: touchPoint8 },
        KeyTouchPoint { id: touchPoint9 }
    ]

    function keyAt(x, y)
    {
        var child;
        var childParent = keyRootItem;
        var posChild = { "x":x, "y":y };

        /*
         * Note: Item.childAt uses a pretty dumb algorithm (last child to first, checking only boundingRect).
         * So if there are stacked brother items, it won't necessarily take the one with highest z-order.
         */
        do {
            child = childParent.childAt(posChild.x, posChild.y);
            if (!child) {
                return null;
            } else if (child && child.objectName === "pressArea") {
                return child;
            }
            posChild = childParent.mapToItem(child, posChild.x, posChild.y);
            childParent = child;
        }
        while (child);
    }

    onPressed: {
        for( var i = 0; i < touchPoints.length; ++i ) {
            var keyArea = keyAt(touchPoints[i].x, touchPoints[i].y);
            touchPoints[i].currentKeyArea = keyArea;
            if( keyArea ) {
                keyArea.pressed(false);
            }
            else {
                pressOnNoKeyArea();
            }
        }
    }

    onReleased: {
        for( var i = 0; i < touchPoints.length; ++i ) {
            var keyArea = touchPoints[i].currentKeyArea;
            if( keyArea ) {
                keyArea.released(false);
            }
            else {
                releaseOnNoKeyArea();
            }
        }
    }

    onUpdated: {
        for( var i = 0; i < touchPoints.length; ++i ) {
            var currentKeyArea = touchPoints[i].currentKeyArea
            var keyArea = keyAt(touchPoints[i].x, touchPoints[i].y);

            if( keyArea !== currentKeyArea ) {
                if( currentKeyArea ) {
                    currentKeyArea.released(true);
                }
                if( keyArea ) {
                    keyArea.pressed(true);
                }
                touchPoints[i].currentKeyArea = keyArea;
                if( keyArea ) {
                    keyArea.moved();
                }
            }
        }
    }
}
