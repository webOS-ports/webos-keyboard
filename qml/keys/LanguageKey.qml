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

ActionKey {
    id: actionKey

    iconNormal: "language";
    iconShifted: "language";
    iconCapsLock: "language";

    padding: 0

    visible: maliit_input_method.enabledLanguages.length > 1 ? true : false
    width: visible ? UI.keyWidth : 0

    MouseArea {
        anchors.fill: parent
        preventStealing: true
        onPressAndHold: {
            UI.showLanguagesMenu(actionKey);
        }
    }
}
