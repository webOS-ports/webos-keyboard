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
    /* The application can name this key through Maliit's "actionKey" override -
       "Go", "Search", "Send" - which is what PalmIME::EditorState::enterKeyLabel
       carried in the reference. Empty means a plain "Enter". */
    label: maliit_input_method.actionKeyLabel.length > 0
           ? maliit_input_method.actionKeyLabel : "Enter";
    shifted: label;

    weight: 1.5
    action: "return"
    fontSize: UI.xsFontSize;
}
