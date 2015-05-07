/*
 * Copyright 2013 Canonical Ltd.
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

import QtQuick.Window 2.0

import "key_constants.js" as UI

Item {
    id: popover
    enabled: false
    visible: enabled

    property variant extendedKeysModel
    property Item currentlyAssignedKey

    property int currentlyAssignedKeyParentY: currentlyAssignedKey ? currentlyAssignedKey.parent.y : 0
    property int currentlyAssignedKeyX: currentlyAssignedKey ? currentlyAssignedKey.x : 0
    property int currentlyAssignedKeyY: currentlyAssignedKey ? currentlyAssignedKey.y : 0

    onCurrentlyAssignedKeyXChanged: __repositionPopoverTo(currentlyAssignedKey)
    onCurrentlyAssignedKeyYChanged: __repositionPopoverTo(currentlyAssignedKey)
    onCurrentlyAssignedKeyParentYChanged: __repositionPopoverTo(currentlyAssignedKey);

    onCurrentlyAssignedKeyChanged:
    {
        if (currentlyAssignedKey == null)
            return;

        __repositionPopoverTo(currentlyAssignedKey);
    }

    ///
    // Item gets repositioned above the currently active key on keyboard.
    // extended keys area will center on top of this

    Item {
        id: anchorItem
        width: panel.keyWidth
        height: panel.keyHeight
    }

    BorderImage {
        id: popoverBackground

        property bool isOnLeftSideOfScreen: anchorItem.x < (panel.width/2)

        y: anchorItem.y
        x: isOnLeftSideOfScreen ? (anchorItem.x) : (anchorItem.x+anchorItem.width-popoverBackground.width)
        width: Math.max(keypad.keyWidth, rowOfKeys.width + 10*2)
        height: 90

        source: "../images/keyboard_popover.png"
        border {
            left: isOnLeftSideOfScreen ? 65 : 18
            right: isOnLeftSideOfScreen ? 18 : 65
            top: 20
            bottom: 30
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: closePopover();
    }

    Row {
        id: rowOfKeys
        anchors.centerIn: popoverBackground
        anchors.verticalCenterOffset: -5

        // spacing: units.gu( UI.popoverCellPadding )
        Repeater {
            id: keyRepeater
            model: extendedKeysModel

            Item {
                id: key

                property alias commitStr: textCell.text
                property bool highlight: false

                height: popupKeyImage.height //languageTextItem.contentHeight
                width: popupKeyImage.width //languageTextItem.contentWidth
                Image
                {
                    anchors.centerIn: parent
                    id: popupKeyImage
                    source: "../images/popup_key_inactive.png"
                    width: 60
                    height: 60
                }

                Text {
                    id: textCell
                    anchors.centerIn: parent
                    text: modelData
                    font.family: UI.fontFamily
                    font.pixelSize: text.length > 2 ? units.gu( UI.smallFontSize ) : units.gu( UI.fontSize )
                    font.bold: UI.fontBold
                    color: key.highlight ? "orange"  : UI.fontColor
                }

                MouseArea {
                    anchors.fill: parent
                    preventStealing: true

                    onPressed: key.highlight = true;

                    onReleased: {
                        key.highlight = false;
                        event_handler.onKeyReleased(modelData);
                        popover.closePopover();
                    }
                }

            }
        }
    }

    function enableMouseArea()
    {
        extendedKeysMouseArea.enabled = true
    }

    function __repositionPopoverTo(item)
    {
        // item.parent is a row
        var row = item.parent;
        var point = popover.mapFromItem(item, item.x, item.y)

        anchorItem.x = item.x + row.x
        anchorItem.y = point.y - (panel.keyHeight + units.dp(UI.popoverTopMargin));
    }

    function __restoreAssignedKey()
    {
        currentlyAssignedKey.state = "NORMAL"
    }

    function closePopover()
    {
        extendedKeysModel = null;
        popover.enabled = false
    }
}


