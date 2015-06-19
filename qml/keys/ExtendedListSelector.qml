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
    id: popover
    enabled: false
    visible: enabled

    y: anchorPoint.y - (popoverBackground.height - 8)
    x: isOnLeftSideOfScreen ? (anchorPoint.x) : (anchorPoint.x+currentlyAssignedKeyWidth-popoverBackground.width)

    width: popoverBackground.width
    height: popoverBackground.height

    property variant extendedListModel
    property Item currentlyAssignedKey

    property int keyHeight: Units.gu(6.3) 
    property int keyWidth: Units.gu(8.0) 

    property Item keyPad

    property bool isOnLeftSideOfScreen: anchorPoint.x < (keyPad.width/2)

    property int currentlyAssignedKeyParentY: currentlyAssignedKey ? currentlyAssignedKey.parent.y : 0
    property int currentlyAssignedKeyX: currentlyAssignedKey ? currentlyAssignedKey.x : 0
    property int currentlyAssignedKeyY: currentlyAssignedKey ? currentlyAssignedKey.y : 0
    property int currentlyAssignedKeyWidth: currentlyAssignedKey ? currentlyAssignedKey.width : 0

    property int numberOfLines: Math.ceil(keyRepeater.count / 4)
	
    onCurrentlyAssignedKeyXChanged: if(currentlyAssignedKey) __repositionPopoverTo(currentlyAssignedKey);
    onCurrentlyAssignedKeyYChanged: if(currentlyAssignedKey) __repositionPopoverTo(currentlyAssignedKey)
    onCurrentlyAssignedKeyParentYChanged: if(currentlyAssignedKey) __repositionPopoverTo(currentlyAssignedKey);

    onCurrentlyAssignedKeyChanged:
    {
        if (currentlyAssignedKey == null)
            return;

        __repositionPopoverTo(currentlyAssignedKey);
    }

    function __updatePopoverRect() {
        var newPopoverRect = keyPad.mapToItem(fullScreenItem, x, y, width, height);
        maliit_geometry.popoverRect = Qt.rect(newPopoverRect.x, newPopoverRect.y, newPopoverRect.width, newPopoverRect.height);
    }
    onXChanged: __updatePopoverRect();
    onYChanged: __updatePopoverRect();
    onWidthChanged: __updatePopoverRect();
    onHeightChanged: __updatePopoverRect();

    signal itemSelected(string modelData);
    signal extendedListDismissed();

    ///
    // Item gets repositioned above the currently active key on keyboard.
    // extended keys area will center on top of this
    property point anchorPoint: "0,0"

    Item {
        id: popoverBackground

        width: Math.max(popover.keyWidth * (keyRepeater.count <=4 ? keyRepeater.count : 4), rowOfKeys.width + 10*2)
        height: ((Units.gu(3.0) + numberOfLines * popover.keyHeight))
	
        Row {
            x: 0; y: 0; height: parent.height
            BorderImage {
                source: UI.imagePopupBgLeft
                border {left: 21; top: 21; bottom: 36;}
				height: parent.height
                verticalTileMode: BorderImage.Stretch
				width: 21
            }
            BorderImage {
                source: UI.imagePopupBgBetween
                height: parent.height
                border {top: 21; bottom: 36;}
                verticalTileMode: BorderImage.Stretch
                width: popover.isOnLeftSideOfScreen ? 8 : (popoverBackground.width - 21 - 33 - 8 - 21)
            }
            BorderImage {
                source: UI.imagePopupBgCaret
                border {top: 21; bottom: 36;}
                verticalTileMode: BorderImage.Stretch
                height: parent.height
                width: 33
            }
            BorderImage {
                source: UI.imagePopupBgBetween
                border {top: 21; bottom: 36;}
                verticalTileMode: BorderImage.Stretch
                height: parent.height
                width: popover.isOnLeftSideOfScreen ? (popoverBackground.width - 21 - 33 - 8 - 21) : 8
            }
            BorderImage {
                border {right: 21; top: 21; bottom: 36;}
                source: UI.imagePopupBgRight
                height: parent.height
                width: 21
            }
        }
    }

    Flow {
        id: rowOfKeys
        anchors.centerIn: popoverBackground
        anchors.verticalCenterOffset: -5
        width: Math.min(keyRepeater.count, 4) * popover.keyWidth

        Repeater {
            id: keyRepeater
            model: extendedListModel

            Item {
                id: key

                property alias commitStr: textCell.text
                property bool highlight: popupKeyPressArea.isPressed

                height: popupKeyImage.height
                width: popupKeyImage.width
                Image
                {
                    anchors.centerIn: parent
                    id: popupKeyImage
                    source: UI.imagePopupKey
                    width: popover.keyWidth
                    height: popover.keyHeight
                }

                Text {
                    id: textCell
                    anchors.centerIn: parent
                    //anchors.verticalCenterOffset: UI.formFactor === "phone" ? Units.gu (-0.15) : 0
					anchors.verticalCenterOffset: Units.gu(-0.15)
                    text: modelData
                    font.family: UI.fontFamily
                    font.pixelSize: text.length > 2 ? FontUtils.sizeToPixels(UI.smallFontSize) : FontUtils.sizeToPixels(UI.fontSize)
                    font.bold: false //UI.fontBold
                    color: key.highlight ? UI.extendedHighLightColor : UI.extendedFontColor
                }

                PressArea {
                    id: popupKeyPressArea
                    anchors.fill: parent
                    compatibleWithPopover: true

                    onKeyReleased: {
                        if (maliit_input_method.useAudioFeedback)
                            audioFeedback.play();

                        popover.itemSelected(modelData);

                        if (UI.currentShiftState === "SHIFTED" && UI.currentSymbolState === "CHARACTERS")
                            UI.currentShiftState = "NORMAL"
                    }
                }

            }
        }
    }

    MultiTouchKeyArea {
        id: popoverTouchArea
        anchors.fill: rowOfKeys
        z: 3

        keyRootItem: rowOfKeys
    }

    function __repositionPopoverTo(item)
    {
        // item.parent is a row
        var row = item.parent;
        var point = keyPad.mapFromItem(row, item.x, item.y)

        anchorPoint.x = point.x
        anchorPoint.y = point.y
    }

    function __restoreAssignedKey()
    {
        currentlyAssignedKey.state = "NORMAL"
    }

    function closePopover()
    {
        extendedListModel = null;
        popover.enabled = false;
        popover.extendedListDismissed();
    }
}


