import QtQuick 2.3
import QtQuick.Controls 1.1
import "../../../../qml"

Rectangle {
    width: 768
    height: 1280

    QtObject {
        id: maliit_geometry
        property real canvasHeight: 300
        property rect visibleRect: Qt.rect(0,0,700,300);
        property int orientation: 0
        property bool shown: true
    }
    QtObject {
        id: maliit_event_handler

        function onKeyPressed(valueToSubmit, action) { console.log("onKeyPressed : " + valueToSubmit); }
        function onKeyReleased(valueToSubmit, action) { console.log("onKeyReleased : " + valueToSubmit); }
        function onWordCandidatePressed(word) { console.log("onWordCandidatePressed : " + word); }
        function onWordCandidateReleased(word) { console.log("onWordCandidateReleased : " + word); }
    }
    QtObject {
        id: maliit_word_engine
        property bool enabled: true
    }
    ListModel {
        id: maliit_wordribbon
        ListElement { word: "first" }
        ListElement { word: "second"}
    }
    QtObject {
        id: maliit_input_method

        signal activateAutocaps();

        property int contentType: 0 // 0 ->  text, 1 -> number, 2 -> telephone, 3 -> email, 4 -> url
        property bool testEnvironment: true
        property string activeLanguage: "en"
        property variant enabledLanguages: [ "en", "de", "nl", "fr", "sv", "xx" ]
    }

    Rectangle {
        anchors.fill: parent
        anchors.bottomMargin: maliit_geometry.visibleRect.height

        border {
            color: "black"
            width: 10
        }
        color: "darkgray"
        clip: true

        Flickable {
            anchors.fill: parent
            anchors.margins: 50

            flickableDirection: Flickable.VerticalFlick

            Column {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "contentType = " + maliit_input_method.contentType
                    onClicked: maliit_input_method.contentType = (maliit_input_method.contentType + 1)%5;
                }
                CheckBox {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "word engine : " + (maliit_word_engine.enabled ? "enabled" : "disabled")
                    checked: maliit_word_engine.enabled
                    onClicked: maliit_word_engine.enabled = !maliit_word_engine.enabled;
                }
            }
        }
    }

    Keyboard {
        anchors.fill: parent
    }
}

