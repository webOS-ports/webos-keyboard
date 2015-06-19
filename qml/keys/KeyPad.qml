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
    property int keyHeight: Units.gu(UI.keyHeight);           // as a convenience for the objects inheriting KeyPad
    property real keyWidthRatio: 1.0
    property int currentContentType: maliit_input_method.contentType
    property var alternativeLayouts: [ ]
    property ExtendedListSelector currentVisibleExtendedList

    property Column content

    height: content.height

    Component.onCompleted:
    {
        calculateKeyWidth();
    }
    onWidthChanged: calculateKeyWidth()

    MultiTouchKeyArea {
        id: keyPadTouchArea
        anchors.fill: content
        z: content.z - 1    // put the multi-touch area *behind* the keys, so that they can overload the behavior if needed, like for the TrackBall

        keyRootItem: content

        onPressed: {
            if( currentVisibleExtendedList ) {
                for( var i = 0; i < touchPoints.length; ++i ) {
                    var keyArea = keyAt(touchPoints[i].x, touchPoints[i].y);
                    touchPoints[i].currentKeyArea = keyArea;
                    if( keyArea && !keyArea.compatibleWithPopover ) {
                        UI.hideExtendedKeys();
                        UI.hideKeyboardSizeMenu();
                        UI.hideLanguagesMenu();
                        UI.hideAlternativeLayoutsMenu();

                        break;
                    }
                }
            }
        }
    }

    ExtendedListSelector {
        id: extendedKeysSelector
        z: 2;
        keyPad: keyPadRoot

        onItemSelected: {
            event_handler.onKeyReleased(modelData);
            UI.hideExtendedKeys();
        }
        onExtendedListDismissed: UI.extendedKeysShown = false;
    }

    ExtendedListSelector {
        id: keyboardSizeMenu
        z: 2;
        keyPad: keyPadRoot

        onItemSelected: {
            UI.keyboardSizeChoice = modelData;
            UI.hideKeyboardSizeMenu();
        }
    }

    ExtendedListSelector {
        id: languagesMenu
        z: 2;
        keyPad: keyPadRoot

        onItemSelected: {
            maliit_input_method.activeLanguage = modelData;
            UI.hideLanguagesMenu();
        }
    }

    ExtendedListSelector {
        id: alternativeLayoutsMenu
        z: 2;
        keyPad: keyPadRoot

        onItemSelected: {
            UI.currentAlternativeLayout = ((modelData === "LuneOS") ? "" : modelData);
            UI.hideAlternativeLayoutsMenu();
        }
    }

    Connections {
        target: UI
        onShowExtendedKeys: {
            extendedKeysSelector.currentlyAssignedKey = keyItem;
            extendedKeysSelector.extendedListModel = Qt.binding(function() { return extendedKeysSelector.currentlyAssignedKey.activeExtendedModel });
            extendedKeysSelector.enabled = true;
            currentVisibleExtendedList = extendedKeysSelector;
            UI.extendedKeysShown = true;
        }
        onShowKeyboardSizeMenu: {
            keyboardSizeMenu.extendedListModel = UI.keyboardSizeChoices;
            keyboardSizeMenu.currentlyAssignedKey = keyItem;
            keyboardSizeMenu.enabled = true;
            currentVisibleExtendedList = keyboardSizeMenu;
        }
        onShowLanguagesMenu: {
            languagesMenu.extendedListModel = maliit_input_method.enabledLanguages;
            languagesMenu.currentlyAssignedKey = keyItem;
            languagesMenu.enabled = true;
            currentVisibleExtendedList = languagesMenu;
        }
        onShowAlternativeLayoutsMenu: {
            if( alternativeLayouts.length > 0 ) {
                alternativeLayoutsMenu.extendedListModel = [ "LuneOS" ].concat(alternativeLayouts);
                alternativeLayoutsMenu.currentlyAssignedKey = keyItem;
                alternativeLayoutsMenu.enabled = true;
                currentVisibleExtendedList = alternativeLayoutsMenu;
            }
        }
        onHideExtendedKeys : {
            extendedKeysSelector.closePopover();
            currentVisibleExtendedList = null;
        }
        onHideKeyboardSizeMenu : {
            keyboardSizeMenu.closePopover();
            currentVisibleExtendedList = null;
        }
        onHideLanguagesMenu : {
            languagesMenu.closePopover();
            currentVisibleExtendedList = null;
        }
        onHideAlternativeLayoutsMenu : {
            alternativeLayoutsMenu.closePopover();
            currentVisibleExtendedList = null;
        }
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

        UI.keyWidth = keyWidthRatio*(keyPadRoot.width / maxNrOfKeys);
    }
}
