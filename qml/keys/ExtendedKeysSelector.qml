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

    property bool isTwoLines: keyRepeater.count>5

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

    Item {
        id: popoverBackground

        property bool isOnLeftSideOfScreen: anchorItem.x < (panel.width/2)

        anchors.bottom: anchorItem.bottom
        anchors.bottomMargin: -8
        x: isOnLeftSideOfScreen ? (anchorItem.x) : (anchorItem.x+anchorItem.width-popoverBackground.width)
        width: Math.max(keypad.keyWidth, rowOfKeys.width + 10*2)
        height: isTwoLines ? 150 : 90

        function __updatePopoverRect() {
            var newPopoverRect = popover.mapToItem(null, x, y, width, height);
            maliit_geometry.popoverRect = Qt.rect(newPopoverRect.x, newPopoverRect.y, newPopoverRect.width, newPopoverRect.height);
        }

        onXChanged: __updatePopoverRect();
        onYChanged: __updatePopoverRect();
        onWidthChanged: __updatePopoverRect();
        onHeightChanged: __updatePopoverRect();

        Row {
            x: 0; y: 0; height: parent.height
            Image {
                source: isTwoLines ? UI.imagePopupBgLeft2[formFactor] : UI.imagePopupBgLeft[formFactor]
                height: parent.height
                width: 21
            }
            Image {
                source: isTwoLines ? UI.imagePopupBgBetween2[formFactor] : UI.imagePopupBgBetween[formFactor]
                height: parent.height
                width: popoverBackground.isOnLeftSideOfScreen ? 8 : (popoverBackground.width - 21 - 33 - 8 - 21)
            }
            Image {
                source: isTwoLines ? UI.imagePopupBgCaret2[formFactor] : UI.imagePopupBgCaret[formFactor]
                height: parent.height
                width: 33
            }
            Image {
                source: isTwoLines ? UI.imagePopupBgBetween2[formFactor] : UI.imagePopupBgBetween[formFactor]
                height: parent.height
                width: popoverBackground.isOnLeftSideOfScreen ? (popoverBackground.width - 21 - 33 - 8 - 21) : 8
            }
            Image {
                source: isTwoLines ? UI.imagePopupBgRight2[formFactor] : UI.imagePopupBgRight[formFactor]
                height: parent.height
                width: 21
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: closePopover();
    }

    Flow {
        id: rowOfKeys
        anchors.centerIn: popoverBackground
        anchors.verticalCenterOffset: -5
        width: Math.min(keyRepeater.count, 5) * 60

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
//                    x: anchorItem.x
                    id: popupKeyImage
                    source: UI.imagePopupKey[formFactor]
                    width: 60
                    height: 60
                }

                Text {
                    id: textCell
                    anchors.centerIn: parent
                    text: modelData
                    font.family: UI.fontFamily
                    font.pixelSize: text.length > 2 ? UI.smallFontSize : UI.fontSize
                    font.bold: UI.fontBold
                    color: key.highlight ? "#4B97DE"  : UI.fontColor[formFactor]
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
        popover.enabled = false;
    }
}


