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

    MultiPointTouchArea {
        id: keyPadTouchArea
        anchors.fill: keyPadRoot
        z: 3

        touchPoints: [
            KeyTouchPoint { id: touchPoint0 },
            KeyTouchPoint { id: touchPoint1 },
            KeyTouchPoint { id: touchPoint2 },
            KeyTouchPoint { id: touchPoint3 },
            KeyTouchPoint { id: touchPoint4 },
            KeyTouchPoint { id: touchPoint5 },
            KeyTouchPoint { id: touchPoint6 },
            KeyTouchPoint { id: touchPoint7 },
            KeyTouchPoint { id: touchPoint8 },
            KeyTouchPoint { id: touchPoint9 }
        ]

        function keyAt(x, y)
        {
            var child;
            var childParent = keyPadRoot;
            var posChild = { "x":x, "y":y };

            /*
             * Item.childAt uses a pretty dumb algorithm (last child to first, checking only boundingRect).
             * So if there are stacked brother items, it won't necessarily take the one with highest z-order.
             * Therefore, we test the extendedList by hand first.
             */
            if( currentVisibleExtendedList ) {
                var currentPosChildRelative = childParent.mapToItem(currentVisibleExtendedList, posChild.x, posChild.y);
                child = currentVisibleExtendedList.childAt(currentPosChildRelative.x, currentPosChildRelative.y);
                if (child) {
                    posChild = currentPosChildRelative;
                    childParent = child;
                }
            }

            do {
                child = childParent.childAt(posChild.x, posChild.y);
                if (!child) {
                    return null
                } else if (child && child.objectName === "pressArea") {
                    return child
                }
                posChild = childParent.mapToItem(child, posChild.x, posChild.y)
                childParent = child;
            }
            while (child);
        }

        onPressed: {
            for( var i = 0; i < touchPoints.length; ++i ) {
                var keyArea = keyAt(touchPoints[i].x, touchPoints[i].y);
                touchPoints[i].currentKeyArea = keyArea;
                if( keyArea ) {
                    keyArea.pressed(false);
                }
                else {
                    // we clicked where there is no key: hide any remaining popup
                    UI.hideExtendedKeys();
                    UI.hideKeyboardSizeMenu();
                    UI.hideLanguagesMenu();
                    UI.hideAlternativeLayoutsMenu();
                }
            }
        }

        onReleased: {
            for( var i = 0; i < touchPoints.length; ++i ) {
                var keyArea = touchPoints[i].currentKeyArea;
                if( keyArea ) {
                    keyArea.released(false);
                }
            }
        }

        onUpdated: {
            for( var i = 0; i < touchPoints.length; ++i ) {
                var currentKeyArea = touchPoints[i].currentKeyArea
                var keyArea = keyAt(touchPoints[i].x, touchPoints[i].y);

                if( keyArea !== currentKeyArea ) {
                    if( currentKeyArea ) {
                        currentKeyArea.released(true);
                    }
                    if( keyArea ) {
                        keyArea.pressed(true);
                    }
                    touchPoints[i].currentKeyArea = keyArea;
                }
                if( keyArea ) {
                    keyArea.moved();
                }
            }
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
