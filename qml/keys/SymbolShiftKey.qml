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
import LunaNext.Common 0.1
import "key_constants.js" as UI

ActionKey {
    label: "+=[]";
    shifted: "+=[]";
    action: "symbols";

    fontSize: UI.symbolShiftKeyFontSize;
    width: panel.keyWidth //* 2
    
    MouseArea {
        anchors.fill: parent
        preventStealing: true
        onClicked: {
            if (panel.state === "CHARACTERS")
                panel.state = "SYMBOLS";
            else
                panel.state = "CHARACTERS";
        }
    }
}
