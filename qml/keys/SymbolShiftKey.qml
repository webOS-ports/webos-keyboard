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
import LunaNext.Common 0.1

ActionKey {
    label: "+=[]";
    shifted: "+=[]";
    action: "symbols";

    fontSize: UI.xsFontSize;
    // 2 units in the reference keymaps, split 1 + 1 when a language key is shown.
    // Converted layouts set this explicitly; 1 keeps the legacy layouts unchanged.
    weight: 1
    
    PressArea {
        anchors.fill: parent
        onlyExclusive: true

        onKeyPressed: {
            if (UI.currentSymbolState === "CHARACTERS") {
                UI.currentSymbolState = "SYMBOLS";
                // symbolMode = eSymbolMode_Lock, shiftMode = eShiftMode_Off
                UI.currentShiftState = "NORMAL";
            } else {
                UI.currentSymbolState = "CHARACTERS";
            }
        }
    }
}
