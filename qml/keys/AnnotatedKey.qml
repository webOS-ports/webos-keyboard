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
import QtMultimedia 5.0

import LunaNext.Common 0.1

import "key_constants.js" as UI

Item {
    id: key

    property int padding: 0

    width: panel.keyWidth
    height: panel.keyHeight

    /* to be set in keyboard layouts */
    property string label: ""
    property string shifted: ""
    property var extended; // list of extended keys
    property var extendedShifted; // list of extended keys in shifted state

    property alias valueToSubmit: keyLabel.text

    property string action
    property bool noMagnifier: false
    property bool skipAutoCaps: false

    /* design */
    property bool useHorizontalLayout: false;

    property string formFactor: Settings.tabletUi ? "tablet" : "phone"
    property string imgNormal: UI.imageGreyKey[formFactor]
    property string imgPressed: UI.imageGreyKeyPressed[formFactor]
    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property string fontSize: UI.fontSize

    /// annotation shows a small label in the upper right corner
    // if the annotiation property is set, it will be used. If not, the first position in extended[] list or extendedShifted[] list will
    // be used, depending on the state. If no extended/extendedShifted arrays exist, no annotation is shown
    property string annotation: ""

    /*! indicates if te key is currently pressed/down*/
    property alias pressed: keyMouseArea.pressed

    /* internal */
    property string __annotationLabelNormal
    property string __annotationLabelShifted

    /**
     * this property specifies if the key can submit its value or not (e.g. when the popover is shown, it does not commit its value)
     */

    property bool extendedKeysShown: extendedKeysSelector.enabled

    /*
     * label changes when keyboard is in shifted mode
     * extended keys change as well when shifting keyboard, typically lower-uppercase: ê vs Ê
     */

    property string oskState: panel.activeKeypadState
    property var activeExtendedModel: (panel.activeKeypadState === "NORMAL") ? extended : extendedShifted

    Component.onCompleted: {
        if (annotation) {
            __annotationLabelNormal = annotation
            __annotationLabelShifted = annotation
        } else {
            if (extended)
                __annotationLabelNormal = extended[1]
            if (extendedShifted)
                __annotationLabelShifted = extendedShifted[0]
        }
    }

    BorderImage {
        id: buttonImage
        border { left: 27; top: 29; right: 27; bottom: 29 }
        anchors.centerIn: parent
        anchors.fill: key
        anchors.margins: units.gu( UI.keyMargins / 3);
        source: key.pressed ? key.imgPressed : key.imgNormal
    }

    /// label of the key
    //  the label is also the value subitted to the app

    Text {
        id: keyLabel
        //text: (panel.activeKeypadState === "NORMAL") ? label : shifted;
        text: label
        anchors.right: useHorizontalLayout ? parent.right : undefined
        anchors.rightMargin:  useHorizontalLayout ? units.gu(2.0) : 0

        anchors.horizontalCenter: useHorizontalLayout ? undefined : buttonImage.horizontalCenter

        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: useHorizontalLayout ? units.gu(-0.25) : units.gu(0.5)

        font.family: UI.fontFamily
        font.pixelSize: (panel.activeKeypadState === "NORMAL") ? FontUtils.sizeToPixels(UI.fontSize) : FontUtils.sizeToPixels(UI.annotationFontSize)
        font.bold: UI.fontBold
        color: (panel.activeKeypadState === "NORMAL") ? UI.fontColor[formFactor] : UI.annotationFontColor[formFactor]
        style: (panel.activeKeypadState === "NORMAL") ? Text.Raised : Text.Normal
        styleColor: "white"
        smooth: true
    }

    /// shows an annotation
    // used e.g. for indicating the existence of extended keys

    Text {
        id: annotationLabel
        //text: (panel.activeKeypadState != "NORMAL") ? __annotationLabelShifted : __annotationLabelNormal
        text: __annotationLabelNormal

        anchors.left: useHorizontalLayout ? parent.left : undefined
        anchors.leftMargin: useHorizontalLayout ? units.gu(2.0) : 0

        anchors.horizontalCenter: useHorizontalLayout ? undefined : buttonImage.horizontalCenter
        anchors.horizontalCenterOffset: useHorizontalLayout ? units.gu(2.75) : 0

        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: useHorizontalLayout ? units.gu(-0.25) : units.gu(-1.75)

        font.pixelSize: (panel.activeKeypadState === "NORMAL") ? FontUtils.sizeToPixels(UI.annotationFontSize) : FontUtils.sizeToPixels(UI.fontSize)
        font.bold: false
        color: (panel.activeKeypadState !== "NORMAL") ? UI.fontColor[formFactor] : UI.annotationFontColor[formFactor]
        style: (panel.activeKeypadState !== "NORMAL") ? Text.Raised : Text.Normal
        styleColor: "white"
        smooth: true
    }
	
	Text {
        id: annotationLabel2
        //text: (panel.activeKeypadState != "NORMAL") ? __annotationLabelShifted : __annotationLabelNormal
		text: "..." //__annotationLabelNormal

        anchors.right: parent.right
        anchors.rightMargin: useHorizontalLayout ? units.gu(1.0) : units.gu(2.0)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: useHorizontalLayout ? units.gu(1.4) : units.gu(2.0)

        font.pixelSize: FontUtils.sizeToPixels(UI.annotationFontSize)
        font.bold: false
        style: Text.Raised
        styleColor: "white"
        color: UI.fontColor[formFactor] //: UI.annotationFontColor
		smooth: true
    }

    PressArea {
        id: keyMouseArea
        anchors.fill: key

        onPressAndHold: {
            if (activeExtendedModel != undefined) {
                extendedKeysSelector.enabled = true
                extendedKeysSelector.extendedKeysModel = activeExtendedModel
                extendedKeysSelector.currentlyAssignedKey = key
            }
        }

        onReleased: {
            if (!extendedKeysShown) {
                if (maliit_input_method.useAudioFeedback)
                    audioFeedback.play();

                event_handler.onKeyReleased(valueToSubmit, action);
                if (!skipAutoCaps)
                    if (panel.activeKeypadState === "SHIFTED" && panel.state === "CHARACTERS")
                        panel.activeKeypadState = "NORMAL"
            }
        }
        onPressed: {
            event_handler.onKeyPressed(valueToSubmit, action);
        }
    }

    Connections {
        target: swipeArea.drag
        onActiveChanged: {
            if (swipeArea.drag.active)
                keyMouseArea.cancelPress();
        }
    }

    Magnifier {
        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.bottom: buttonImage.top
        width: key.width + units.gu(UI.magnifierHorizontalPadding)
        height: key.height + units.gu(UI.magnifierVerticalPadding)
        text: keyLabel.text
        shown: key.pressed && !noMagnifier && !extendedKeysShown
    }
}
