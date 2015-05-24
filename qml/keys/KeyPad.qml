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

Item {
    id: keyPadRoot

    state: "NORMAL"

    property string symbols: "languages/Keyboard_symbols.qml"
    property bool capsLock: false
    property int keyHeight: Units.gu(UI.keyHeight); // as a convenience for the objects inheriting KeyPad

    property Column content: Column {}

    height: content.height

    Component.onCompleted:
    {
        calculateKeyWidth();
    }
    onWidthChanged: calculateKeyWidth()

    Connections {
        target: UI
        onShowExtendedKeys: {
            extendedKeysSelector.extendedListModel = keysExtendedModel;
            extendedKeysSelector.currentlyAssignedKey = keyItem;
            extendedKeysSelector.enabled = true;
            UI.extendedKeysShown = true;
        }
        onShowKeyboardSizeMenu: {
            keyboardSizeMenu.extendedListModel = UI.keyboardSizeChoices;
            keyboardSizeMenu.currentlyAssignedKey = keyItem;
            keyboardSizeMenu.enabled = true;
        }
        onShowLanguagesMenu: {
            languagesMenu.extendedListModel = maliit_input_method.enabledLanguages;
            languagesMenu.currentlyAssignedKey = keyItem;
            languagesMenu.enabled = true;
        }
        onHideExtendedKeys : {
            extendedKeysSelector.closePopover();
            UI.extendedKeysShown = false;
        }
        onHideKeyboardSizeMenu : {
            keyboardSizeMenu.closePopover();
        }
        onHideLanguagesMenu : {
            languagesMenu.closePopover();
        }
    }

    ExtendedListSelector {
        id: extendedKeysSelector
        anchors.fill: parent
        z: 2;

        onItemSelected: event_handler.onKeyReleased(modelData);
    }

    ExtendedListSelector {
        id: keyboardSizeMenu
        anchors.fill: parent
        z: 2;

        onItemSelected: UI.keyboardSizeChoice = modelData;
    }

    ExtendedListSelector {
        id: languagesMenu
        anchors.fill: parent
        z: 2;

        onItemSelected: maliit_input_method.activeLanguage = modelData;
    }

    function numberOfRows() {
        return content.children.length;
    }

    // we don´t use a QML layout, because we want all keys to be equally sized
    function calculateKeyWidth() {
        var width = keyPadRoot.width;

        var maxNrOfKeys = 0;
        for (var i = 0; i < numberOfRows(); ++i) {
            if (content.children[i].children.length > maxNrOfKeys)
                maxNrOfKeys = content.children[i].children.length;
        }

        UI.keyWidth = keyPadRoot.width / maxNrOfKeys;
    }
}
