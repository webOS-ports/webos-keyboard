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
import "../keys"

KeyPad {
    content: c1
    symbols: ""

    Column {
        id: c1
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 0;

        spacing: 0

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "1"; }
            CharKey { label: "2"; }
            CharKey { label: "3"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "4"; }
            CharKey { label: "5"; }
            CharKey { label: "6"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "7"; }
            CharKey { label: "8"; }
            CharKey { label: "9"; }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.margins: 50;
            spacing: 0

            height: keyHeight

            CharKey { label: "."; extended: ["-", "."]}
            CharKey { label: "0"; }
            BackspaceKey { padding: 0; }
        }
    } // column
}

