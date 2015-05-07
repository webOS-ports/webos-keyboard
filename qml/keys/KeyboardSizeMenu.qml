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

import "key_constants.js" as UI

Item {

    MouseArea {
        width: fullScreenItem.width
        height: fullScreenItem.height

        anchors.centerIn: parent

        onClicked: canvas.keyboardSizeMenuShown = false
    }

    BorderImage {
        id: name
        //anchors.bottom: languageMenuButton.top
        source: "../images/keyboard_popover.png"
		height: units.gu(9) 
		width: menuList.width + units.gu(UI.languageMenuListViewPadding) * 0.5

        property int __corner: units.gu(UI.languageMenuCorner)

        border.left: __corner; border.top: __corner;
        border.right: __corner; border.bottom: __corner;
    }

	ListModel {
        id: myKBSizeModel
        ListElement { name: "XS" }
		ListElement { name: "S" }
		ListElement { name: "M" }
		ListElement { name: "L" }
    }
	
    ListView {
        id: menuList
        anchors.centerIn: name
        width: parent.width - units.gu(UI.languageMenuListViewPadding)
        height: units.gu(9) - units.gu(UI.languageMenuListViewPadding)
        //        width: delegate.count * 50
//        height: parent.height - units.gu(UI.languageMenuListViewPadding)
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        interactive: true
        clip: true
        orientation: ListView.Horizontal

        model: myKBSizeModel

        delegate: Item {
            //height: languageTextItem.contentHeight
			//width: languageTextItem.contentWidth
			height: popupKeyImage.height //languageTextItem.contentHeight
            width: popupKeyImage.width //languageTextItem.contentWidth
            Image
            {
                anchors.top: parent.top
                id: popupKeyImage
                source: "../images/popup_key_inactive.png"
                width: 60
                height: 60
				
                Text {
                    id: languageTextItem
                    anchors.centerIn: parent
					//anchors.fill: parent
                    text: modelData//languageIdToName(modelData)
					font.family: "Prelude"
                    font.bold: true
                }
            }


            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //maliit_input_method.activeLanguage = modelData
                    //TODO Actual resize of keys + keyboard
                    canvas.keyboardSizeMenuShown = false;
                }
            }
         }
    }


}
