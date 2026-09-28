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

    //! Sized from the keys so the grid reads at the same rhythm as the keyboard
    //! it replaces, rather than at a size of its own choosing.
    readonly property real cellSize: Units.gu(5.5)

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

        //! The tabs, one per group, drawn as keys.
        Row {
            id: tabRow
            width: parent.width
            height: Units.gu(4.5)

            Repeater {
                model: EmojiData.groups.length

                Item {
                    width: tabRow.width / EmojiData.groups.length
                    height: tabRow.height

                    BorderImage {
                        anchors.fill: parent
                        anchors.margins: Units.gu(0.1)
                        // The path is built here rather than taken from
                        // UI.imageGreyKey, which is written relative to
                        // qml/keys/ and would climb one directory too far from
                        // this file.
                        source: "images/" + UI.formFactor + "/key_bg_grey"
                                + (index === emojiPanel.currentGroup ? "_active" : "")
                                + ".png"
                        border {
                            left:   UI.formFactor === "tablet" ? 11 : 23
                            top:    UI.formFactor === "tablet" ? 11 : 23
                            right:  UI.formFactor === "tablet" ? 11 : 23
                            bottom: UI.formFactor === "tablet" ? 11 : 23
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        //! The group's own representative emoji is the icon, so
                        //! the tabs need no artwork and follow the data.
                        text: EmojiData.groups[index].tab
                        font.pixelSize: parent.height * 0.55
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: emojiPanel.currentGroup = index
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
                    font.pixelSize: parent.height * 0.62
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
