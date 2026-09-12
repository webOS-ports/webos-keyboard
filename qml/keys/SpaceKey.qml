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
    /* drawKeyCap opens with
         if (key == Qt::Key_Space) text = m_candidateBar.autoSelectCandidate();
       so the space bar shows the word that pressing it would commit. The action,
       not the label, is what gets sent, so this is safe to display. */
    label: maliit_input_method.primaryCandidate;
    shifted: label;

    fontSize: UI.smallFontSize;

    imgNormal: UI.imageSpaceKey
    imgPressed: UI.imageSpaceKeyPressed

    // SPACE_SIZE is 5 on tablet and SPACE_KEY_WEIGHT 4 on phone, less any key
    // flanking it. Converted layouts set this explicitly.
    weight: 1

    action: "space"
}
