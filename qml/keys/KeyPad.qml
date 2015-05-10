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

Item {
    id: keyPadRoot

    state: "NORMAL"

    property var content: c1
    property string symbols: "languages/Keyboard_symbols.qml"
    property bool capsLock: false

    property int nbNumericalRows: 0

    Column {
        id: c1
    }

    Component.onCompleted:
    {
        calculateKeyWidth();
        calculateKeyHeight();
    }

    onWidthChanged: calculateKeyWidth()
    onHeightChanged: calculateKeyHeight();

    function numberOfRows() {
        if( !content ) return 1;

        return content.children.length;
    }

    // we don´t use a QML layout, because we want all keys to be equally sized
    function calculateKeyWidth() {
        var width = panel.width;

        var maxNrOfKeys = 0;
        for (var i = 0; i < numberOfRows(); ++i) {
            if (content.children[i].children.length > maxNrOfKeys)
                maxNrOfKeys = content.children[i].children.length;
        }

        var maxSpaceForKeys = panel.width / maxNrOfKeys;

        panel.keyWidth = maxSpaceForKeys;
    }

    function calculateKeyHeight() {
        // numKey height ratio is 0.74 (see NumKey.qml) and normal key height ratio is 1, so we get
        // panel.height = (0.74 + (nbRows-1)*1) * panel.keyHeight
        panel.keyHeight = panel.height / (0.74*nbNumericalRows + (numberOfRows()-nbNumericalRows));
    }
}
