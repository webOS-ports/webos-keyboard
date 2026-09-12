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

AnnotatedKey {
    id: key

    property int padding: 0

    /* The reference number row declares its own weight sum (12 for a row of ten
       digits plus the 2-unit trackball), so a digit is a plain 1-unit key. Layouts
       that still use the single global unit keep the old approximation. */
    property real keyWidth: UI.keyWidth * UI.numKeyWidthRatio

    width: unitFromRow ? keyUnit * weight : keyWidth
    height: parent.height

    useHorizontalLayout: true
}
