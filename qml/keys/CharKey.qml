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
    property string formFactor: Settings.tabletUi ? "tablet" : "phone"
    property string imgNormal: UI.imageWhiteKey[formFactor]
    property string imgPressed: UI.imageWhiteKeyPressed[formFactor]
    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property int fontSize: UI.fontSize;

    /// annotation shows a small label in the upper right corner
    // if the annotiation property is set, it will be used. If not, the first position in extended[] list or extendedShifted[] list will
    // be used, depending on the state. If no extended/extendedShifted arrays exist, no annotation is shown
//    property string annotation: "…"
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
     		if (extended) {
				if(imgNormal === UI.imageGreyKey[formFactor]) {
					__annotationLabelNormal = extended[0]
					__annotationLabelShifted = label
				}
				else{
                    __annotationLabelNormal = "..."
				}
			}
            if (extendedShifted) {
				if(imgNormal === UI.imageGreyKey[formFactor]) {
					__annotationLabelShifted = extendedShifted[0]
				}
				else{
					__annotationLabelShifted = "..."
				}
			}
        }
    }

    BorderImage {
        id: buttonImage
        border { left: 27; top: 29; right: 27; bottom: 29 }
        anchors.centerIn: parent
        anchors.fill: key
        anchors.margins: units.dp( UI.keyMargins );
        source: key.pressed ? key.imgPressed : key.imgNormal
    }

    /// label of the key
    //  the label is also the value subitted to the app

    Text {
        id: keyLabel
        text: (imgNormal === UI.imageGreyKey[formFactor]) ? label : (panel.activeKeypadState === "NORMAL") ? label : shifted;
        anchors.horizontalCenter: buttonImage.horizontalCenter
		anchors.verticalCenter: buttonImage.verticalCenter 
		anchors.verticalCenterOffset: imgNormal === UI.imageGreyKey[formFactor] ? units.gu(0.5) : units.gu(-0.25)
        font.family: UI.fontFamily
        font.pixelSize: ((imgNormal === UI.imageGreyKey[formFactor])&&(panel.activeKeypadState !== "NORMAL") ) ? UI.annotationFontSize : fontSize
        font.bold: UI.fontBold
        color: ((imgNormal === UI.imageGreyKey[formFactor])&&(panel.activeKeypadState !== "NORMAL")) ? UI.annotationFontColor : UI.fontColor
		smooth: true
		visible: action === ""
    }

    /// shows an annotation
    // used e.g. for indicating the existence of extended keys

    Text {
        id: annotationLabel
        text: (imgNormal === UI.imageGreyKey[formFactor]) ? shifted : (panel.activeKeypadState !== "NORMAL") ? __annotationLabelShifted : __annotationLabelNormal

		anchors.horizontalCenter: buttonImage.horizontalCenter
		anchors.horizontalCenterOffset: imgNormal === UI.imageGreyKey[formFactor] ? 0 : units.gu(2.75)
   		anchors.verticalCenter: buttonImage.verticalCenter 
		anchors.verticalCenterOffset: imgNormal === UI.imageGreyKey[formFactor] ? units.gu(-1.75) : units.gu(1.75)

	    anchors.margins: units.gu( UI.annotationMargins )

        font.pixelSize: ((imgNormal === UI.imageGreyKey[formFactor])&&(panel.activeKeypadState !== "NORMAL")) ? fontSize : UI.annotationFontSize 
        font.bold: false
        color: ((imgNormal === UI.imageGreyKey[formFactor]) && (panel.activeKeypadState !== "NORMAL")) ? UI.fontColor : UI.annotationFontColor
		smooth: true
        visible: !noMagnifier //imgNormal === UI.imageGreyKey[formFactor] ? true : false
    }

    PressArea {
        id: keyMouseArea
        anchors.fill: key

        onPressAndHold: {
            if (activeExtendedModel != undefined && !noMagnifier) {
                extendedKeysSelector.enabled = true
                extendedKeysSelector.extendedKeysModel = activeExtendedModel
                extendedKeysSelector.currentlyAssignedKey = key
				annotationLabel.visible = true
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
