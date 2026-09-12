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
  A row of keys sized the way webOS sized them.

  TabletKeymap::updateLimits() and PhoneKeymap::updateLimits() sum the absolute
  weight of every key in a row - invisible keys included - and normalise that one
  sum to the full keyboard width. Rows are therefore sized independently of each
  other: a row whose weights add up to 13 has narrower keys than a row adding up
  to 11, which is why the reference keyboard looks deliberately ragged.

  Children declare a \l weight instead of a width. KeyRow hands each of them the
  resulting unit through their keyUnit property, so a key is always
  weight x (rowWidth / weightSum) pixels wide.
 */
Row {
    id: keyRow

    spacing: 0

    /*! Width the row normalises into - the full keyboard width. */
    property real rowWidth: parent ? parent.width : 0

    /*! Leave at 0 to sum the children's weights. Set it explicitly only to
        reserve width for keys that are not present in the row. */
    property real weightSum: 0

    /*! Pixels per weight unit. Read-only for callers. */
    property real u: 0

    Component.onCompleted: { watchChildren(); relayout(); }
    onRowWidthChanged: relayout()
    onWeightSumChanged: relayout()
    onChildrenChanged: { watchChildren(); relayout(); }

    /*! A key that changes its own weight - the symbol key growing back to 2 units
        when the language key is hidden - has to resize the whole row. */
    property var __watched: []

    function watchChildren() {
        for (var i = 0; i < children.length; ++i) {
            var c = children[i];
            if (c.weightChanged !== undefined && __watched.indexOf(c) < 0) {
                c.weightChanged.connect(relayout);
                __watched.push(c);
            }
        }
    }

    function totalWeight() {
        if (weightSum > 0)
            return weightSum;
        var sum = 0;
        for (var i = 0; i < children.length; ++i) {
            var w = children[i].weight;
            if (w !== undefined)
                sum += Math.abs(w);
        }
        return sum;
    }

    function relayout() {
        var sum = totalWeight();
        if (sum <= 0 || rowWidth <= 0)
            return;

        u = rowWidth / sum;

        // Assigning breaks each key's fallback binding to UI.keyWidth, which is
        // what we want: from here on the row owns the horizontal metric.
        for (var i = 0; i < children.length; ++i) {
            if (children[i].keyUnit !== undefined)
                children[i].keyUnit = u;
            if (children[i].unitFromRow !== undefined)
                children[i].unitFromRow = true;
        }
    }
}
