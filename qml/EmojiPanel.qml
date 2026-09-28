/*
 * This file is part of the LuneOS keyboard
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 */

import QtQuick 2.0
import keys 1.0
import LunaNext.Common 0.1

import "emoji.js" as EmojiData

/*!
 * \brief A grid of emoji, in the keys' place.
 *
 * Dressed in the keyboard's own materials - the same tiled background, the same
 * key art for the tabs, the same border above - so it reads as a face of the
 * keyboard rather than a window that happened to open over it.
 *
 * An emoji is inserted through event_handler exactly as a character key inserts
 * one, because that is all it is: text. Legacy's seven emoticons were ASCII with
 * pictures drawn over them, so every consumer had to be in on the trick and the
 * set could never grow. These are characters, and the only thing they need is a
 * font that covers them.
 */
Item {
    id: emojiPanel

    //! Which of EmojiData.groups is showing.
    property int currentGroup: 0

    /*!
     * How many emoji fit across, and therefore how big each is.
     *
     * Derived from the width rather than fixed, so the grid is flush on both
     * edges instead of leaving a ragged column, and so a 720-wide phone and a
     * 1024-wide tablet each get a sensible number rather than the phone getting
     * eight enormous ones.
     */
    readonly property int columns: Math.max(6, Math.floor(width / Units.gu(4.2)))
    readonly property real cellSize: width / columns

    Image {
        anchors.fill: parent
        source: "images/" + UI.formFactor + "/keyboard-bg.png"
        fillMode: Image.TileHorizontally
    }

    Column {
        anchors.fill: parent

        Image {
            source: "images/" + UI.formFactor + "/border_top.png"
            width: parent.width
        }

        /*!
         * The tabs, in the shape LuneOS gives a TabBar rather than in the
         * keyboard's key art: a #343434 strip with the selected tab in #4585b8,
         * the same blue a Switch carries when it is on. The key art was tried
         * first and read as nine black lozenges with a blue halo, which belongs
         * to no other screen on the device.
         */
        Rectangle {
            id: tabRow
            width: parent.width
            height: Units.gu(3.6)
            color: "#343434"

            Row {
                anchors.fill: parent

                Repeater {
                    model: EmojiData.groups.length

                    Rectangle {
                        width: Math.round(tabRow.width / EmojiData.groups.length)
                        height: tabRow.height
                        color: index === emojiPanel.currentGroup ? "#4585b8" : "#343434"

                        Text {
                            anchors.centerIn: parent
                            //! The group's own first emoji is its icon, so the
                            //! tabs need no artwork and follow the data.
                            text: EmojiData.groups[index].tab
                            font.pixelSize: Math.round(parent.height * 0.62)
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: emojiPanel.currentGroup = index
                        }
                    }
                }
            }
        }

        GridView {
            id: grid
            width: parent.width
            height: parent.height - tabRow.height
            clip: true
            cacheBuffer: height

            cellWidth: emojiPanel.cellSize
            cellHeight: emojiPanel.cellSize

            model: EmojiData.groups[emojiPanel.currentGroup].emoji

            //! Back to the top when the group changes; carrying one group's
            //! scroll position into the next lands the user in the middle of
            //! something they did not choose.
            onModelChanged: positionViewAtBeginning()

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Units.gu(0.2)
                    radius: Units.gu(0.4)
                    color: "#ffffff"
                    opacity: emojiArea.pressed ? 0.25 : 0
                }

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: Math.round(parent.height * 0.6)
                }

                MouseArea {
                    id: emojiArea
                    anchors.fill: parent

                    onClicked: {
                        //! The road a character key takes. No action, so it goes
                        //! in as plain text.
                        event_handler.onKeyPressed(modelData, "");
                        event_handler.onKeyReleased(modelData, "");
                    }
                }
            }
        }
    }
}
