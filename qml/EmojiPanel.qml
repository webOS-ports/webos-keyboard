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
 * \brief A grid of emoji, in place of the keys.
 *
 * An emoji is inserted the same way a character key inserts one, through
 * event_handler, because that is all it is: text. Legacy's seven emoticons were
 * ASCII with pictures drawn over them, which meant every consumer had to be in
 * on the trick and the set could never grow. These are ordinary characters, so
 * nothing downstream needs to know anything - as long as something can draw
 * them, which is what ttf-noto-emoji-color in the image is for.
 *
 * Reached from the candidate bar rather than a Sym key: the bar is on screen
 * whenever a field has focus, including on a device typing on physical keys,
 * whereas a Sym key is not something every keyboard lets out. The Zinwa Q25's
 * driver resolves Alt and Sym itself and emits only the resolved character, so
 * there is no Sym keypress there to hang this on - measured by reading its event
 * node while the key was pressed: nothing at all came out.
 */
Item {
    id: emojiPanel

    //! Roughly a key's worth, so the grid reads like the keyboard it replaces.
    readonly property real cellSize: Units.gu(5)

    Rectangle {
        anchors.fill: parent
        color: UI.backgroundColor !== undefined ? UI.backgroundColor : "#111111"
    }

    Column {
        anchors.fill: parent

        //! The categories, as a strip of one representative emoji each.
        Row {
            id: categoryRow
            width: parent.width
            height: Units.gu(4)

            Repeater {
                model: EmojiData.categories.length

                Item {
                    width: categoryRow.width / EmojiData.categories.length
                    height: categoryRow.height

                    Rectangle {
                        anchors.fill: parent
                        color: index === emojiPanel.currentCategory ? "#3a3a3a" : "transparent"
                    }

                    Text {
                        anchors.centerIn: parent
                        // The first emoji of the category stands for it, so the
                        // strip needs no icons of its own and grows with the data.
                        text: EmojiData.categories[index].emoji.charAt(0)
                              + EmojiData.categories[index].emoji.charAt(1)
                        font.pixelSize: Units.gu(2.5)
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: emojiPanel.currentCategory = index
                    }
                }
            }
        }

        GridView {
            id: grid
            width: parent.width
            height: parent.height - categoryRow.height
            clip: true

            cellWidth: emojiPanel.cellSize
            cellHeight: emojiPanel.cellSize

            // Surrogate pairs: every one of these is above U+FFFF, so a QML
            // string holds each as two code units and the model has to step in
            // twos rather than ones.
            model: EmojiData.categories[emojiPanel.currentCategory].emoji.length / 2

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                property string emoji:
                    EmojiData.categories[emojiPanel.currentCategory].emoji
                        .substr(index * 2, 2)

                Text {
                    anchors.centerIn: parent
                    text: parent.emoji
                    font.pixelSize: Units.gu(3)
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // The same road a character key takes. No action, so it
                        // is inserted as plain text.
                        event_handler.onKeyPressed(parent.emoji, "");
                        event_handler.onKeyReleased(parent.emoji, "");
                    }
                }
            }
        }
    }

    property int currentCategory: 0
}
