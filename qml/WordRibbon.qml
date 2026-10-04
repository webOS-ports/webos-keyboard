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
import LunaNext.Common 0.1
import keys 1.0

Rectangle {

    id: wordRibbonCanvas
    objectName: "wordRibbenCanvas"
    state: "NORMAL"

    Rectangle {
        anchors.fill: parent
        // Dark under the Pre layouts, to sit with their charcoal deck.
        color: UI.preStyle ? "#2b3034" : "#f1f1f1"
    }

    /*!
     * \brief Whether the emoji panel is showing.
     *
     * Kept here rather than in the panel because the button that toggles it is
     * here, and because the candidate bar is the one part of the keyboard that
     * is on screen whatever else is - including on a device being typed on with
     * physical keys, where there are no on-screen keys at all to put it among.
     */
    property alias emojiShown: emojiButton.checked

    ListView {
        id: listView
        objectName: "wordListView"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: emojiButton.left

        model: maliit_wordribbon

        orientation: ListView.Horizontal
        delegate: wordCandidateDelegate

    }

    Component {
        id: wordCandidateDelegate
        Item {
            id: wordCandidateItem
            width: wordItem.width + Units.gu(2);
            height: wordRibbonCanvas.height
            property alias word_text: wordItem // For testing in Autopilot

            Item {
                anchors.fill: parent
                // Horizontal only: the delegate's own width is
                // wordItem.width + Units.gu(2), which is this margin either
                // side. Vertically the text is centred instead, so the bar does
                // not have to be tall enough to hold a margin above and below
                // it as well - which is what made it twice the height of its
                // own text.
                anchors.leftMargin: Units.gu(1);
                anchors.rightMargin: Units.gu(1);

                Text {
                    id: wordItem
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Units.gu(2);
                    font.family: "Prelude"
                    color: UI.preStyle ? "#d8dde0" : "#999999"
                    font.bold: false
                    text: word;
                }
            }

            MouseArea {
                anchors.fill: wordCandidateItem
                onPressed: {
                    wordRibbonCanvas.state = "SELECTED"
                    event_handler.onWordCandidatePressed(wordItem.text);
                }
                onReleased: {
                    wordRibbonCanvas.state = "NORMAL"
                    event_handler.onWordCandidateReleased(wordItem.text)
                }
            }
        }
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: wordRibbonCanvas
                color: "transparent"
            }
        },
        State {
            name: "SELECTED"
            PropertyChanges {
                target: wordRibbonCanvas
                color: UI.preStyle ? "#3a4146" : "#e4e4e4"
            }
        }
    ]

    /*!
     * \brief Opens the emoji panel.
     *
     * On the right, where it does not move as candidates come and go.
     */
    Item {
        id: emojiButton

        property bool checked: false

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height

        Rectangle {
            anchors.fill: parent
            color: emojiButton.checked ? (UI.preStyle ? "#3a4146" : "#d4d4d4") : "transparent"
        }

        Rectangle {
            // A hairline to part it from the candidates, so a long word does
            // not appear to run into the button.
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: UI.preStyle ? "#454c51" : "#d0d0d0"
        }

        Text {
            anchors.centerIn: parent
            // The character is the icon: the font that makes the panel worth
            // having is the same one that draws this.
            text: "\ud83d\ude42"
            font.pixelSize: parent.height * 0.6
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                emojiButton.checked = !emojiButton.checked;

                // With a physical keyboard the keys are not on screen, and the
                // panel goes where they would be - so ask for them back first,
                // or there is nowhere for it to appear.
                if (emojiButton.checked)
                    maliit_input_method.expandKeys();
            }
        }
    }
}
