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

import "key_constants.js" as UI

Item {

    MouseArea {
        width: fullScreenItem.width
        height: fullScreenItem.height

        anchors.centerIn: parent

        onClicked: canvas.languageMenuShown = false
    }

    BorderImage {
        id: name
//        anchors.bottom: languageMenuButton.top
        source: UI.imagePopover[formFactor]
		height: units.gu(9) 
		width: menuList.width + units.gu(UI.languageMenuListViewPadding) * 0.5

        property int __corner: units.gu(UI.languageMenuCorner)

        border.left: __corner; border.top: __corner;
        border.right: __corner; border.bottom: __corner;
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

        model: maliit_input_method.enabledLanguages

        delegate: Item {
            //height: languageTextItem.contentHeight
			//width: languageTextItem.contentWidth
			height: popupKeyImage.height //languageTextItem.contentHeight
            width: popupKeyImage.width //languageTextItem.contentWidth
            Image
            {
                anchors.top: parent.top
                id: popupKeyImage
                source: UI.imagePopupKey[formFactor]
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
                    maliit_input_method.activeLanguage = modelData
                    canvas.languageMenuShown = false;
                }
            }
         }
    }

    function languageIdToName(languageId)
    {
        if (languageId == "ar")         return i18n.tr("Arabic");
        if (languageId == "cs")         return i18n.tr("Czech");
        if (languageId == "da")         return i18n.tr("Danish");
        if (languageId == "de")         return i18n.tr("German");
        if (languageId == "en")         return i18n.tr("English");
        if (languageId == "es")         return i18n.tr("Spanish");
        if (languageId == "fi")         return i18n.tr("Finnish");
        if (languageId == "fr")         return i18n.tr("French");
        if (languageId == "he")         return i18n.tr("Hebrew");
        if (languageId == "hu")         return i18n.tr("Hungarian");
        if (languageId == "it")         return i18n.tr("Italian");
        if (languageId == "nl")         return i18n.tr("Dutch");
        if (languageId == "pl")         return i18n.tr("Polish");
        if (languageId == "pt")         return i18n.tr("Portuguese");
        if (languageId == "ru")         return i18n.tr("Russian");
        if (languageId == "sv")         return i18n.tr("Swedish");
        if (languageId == "zh")         return i18n.tr("Chinese - Pinyin");
        if (languageId == "wo")         return i18n.tr("WebOS");

        // fallback
        return i18n.tr("language " + languageId);
    }

}
