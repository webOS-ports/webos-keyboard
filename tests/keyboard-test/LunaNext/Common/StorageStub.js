/*
 * Copyright (C) 2013 Christophe Chapuis <chris.chapuis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

.pragma library

var DEFAULT_GRID_UNIT_PX = 8;
var gridUnit = 10;
var layoutScale = 10;

function dp(value) {
    var ratio = gridUnit / DEFAULT_GRID_UNIT_PX;
    if (value <= 2.0)
        // for values under 2dp, return only multiples of the value
        return Math.round(value * Math.floor(ratio));
    return Math.round(value * ratio);
}

function gu(value) {
    return Math.round(value * gridUnit);
}
