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

Item {
    id: wordRibbonCanvas
    objectName: "wordRibbenCanvas"
    state: "NORMAL"

    //property TextInput textEntry
    visible: true

    Row {
        id: overlayBackground
        visible: true
        //anchors.top
        //anchors.fill:parent
        //y: parent.height / 12
        x: overlayBackground.width / 2

        Image {
            id: overlayBackgroundLeft
            source: "images/"+UI.formFactor+"/ate-left.png"
            height: Units.gu(6)
            width: Units.gu(1.7)
        }
        Image {
            id: overlayBackgroundLeftMiddle
            source: "images/"+UI.formFactor+"/ate-middle.png"
            //width: overlayBackgroundArrowDown.width/2
            width: wordCandidateDelegate.width //- overlayBackgroundArrowDown.width/2
            height: Units.gu(6)
            //anchors.left: overlayBackgroundLeft.right
        }
        Image {
            id: overlayBackgroundArrowDown
            source: "images/"+UI.formFactor+"/ate-arrow-down.png"
            anchors.bottom: overlayBackgroundLeft.bottom
            height: Units.gu(6.7)
            width: Units.gu(3.3)
            //anchors.left: overlayBackgroundLeftMiddle.right
        }
        Image {
            id: overlayBackgroundRightMiddle
            source: "images/"+UI.formFactor+"/ate-middle.png"
            //width: overlayBackgroundArrowDown.width/2
            width: wordCandidateDelegate.width //- overlayBackgroundArrowDown.width/2
            height: Units.gu(6)
            //anchors.left: overlayBackgroundArrowDown.right
        }
        Image {
            id: overlayBackgroundRight
            source: "images/"+UI.formFactor+"/ate-right.png"
            height: Units.gu(6)
            width: Units.gu(1.7)
            //anchors.left: overlayBackgroundRightMiddle.right
        }
    }

    ListView {
        id: listView
        objectName: "wordListView"
        anchors.fill: parent;

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
                anchors.margins: Units.gu(1);

                Text {
                    id: wordItem
                    font.pixelSize: Units.gu(2);
                    font.family: "Prelude"
                    color: "white"
                    //color: "#999999"
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
                //color: "transparent"
            }
        },
        State {
            name: "SELECTED"
            PropertyChanges {
                target: wordRibbonCanvas
                //color: "#e4e4e4"
            }
        }
    ]
}


/*Item {

    id: wordRibbonCanvas
    objectName: "wordRibbenCanvas"
    state: "NORMAL"

    Rectangle {
        width: Units.gu(10)
        anchors.fill: parent
        //color: "#f1f1f1"
        color: "#82bef1"
        border.width: 1
        border.color: "black"
        radius: 5
    }

    ListView {
        id: listView
        objectName: "wordListView"
        anchors.fill: parent;

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
                anchors.margins: Units.gu(1);

                Text {
                    id: wordItem
                    font.pixelSize: Units.gu(2);
                    font.family: "Prelude"
                    color: "white"
                    //color: "#999999"
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
                color: "#e4e4e4"
            }
        }
    ]

}*/

