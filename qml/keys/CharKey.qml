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

import keys 1.0
import LunaNext.Common 0.1

Item {
    id: key

    property int padding: 0

    width: panel.keyWidth
    height: parent.height

    /* to be set in keyboard layouts */
    property string label: ""
    property string shifted: ""
    property var extended; // list of extended keys
    property var extendedShifted; // list of extended keys in shifted state

    property alias valueToSubmit: keyLabel.text

    property string action
    property bool skipAutoCaps: false

    /* design */
    property string imgNormal: UI.imageWhiteKey
    property string imgPressed: UI.imageWhiteKeyPressed
    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property string fontSize: UI.fontSize
	
	//We only want the maginifier for phone, so set the noMagnifier to true for tablets
    property bool noMagnifier: UI.formFactor==="tablet" ? true : false
	
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
     		if (extended) {
                if(imgNormal === UI.imageGreyKey) {
					__annotationLabelNormal = extended[0]
					__annotationLabelShifted = label
				}
				else{
                    __annotationLabelNormal = "…"
				}
			}
            if (extendedShifted) {
                if(imgNormal === UI.imageGreyKey) {
					__annotationLabelShifted = extendedShifted[0]
				}
				else{
					__annotationLabelShifted = "…"
				}
			}
        }
    }

    BorderImage {
        id: buttonImage
        border {
            left:   UI.formFactor==="tablet" ? 14 : 24
            top:    UI.formFactor==="tablet" ? 13 : 24
            right:  UI.formFactor==="tablet" ? 14: 24
            bottom: UI.formFactor==="tablet" ? 17 : 24
        }
        anchors.centerIn: parent
        anchors.fill: key
        anchors.margins: Units.gu( UI.keyMargins );
        source: key.pressed ? key.imgPressed : key.imgNormal
    }

    /// label of the key
    //  the label is also the value subitted to the app

    Text {
        id: keyLabel
        text: (panel.activeKeypadState === "NORMAL") ? label : shifted;
        anchors.horizontalCenter: buttonImage.horizontalCenter
		anchors.verticalCenter: buttonImage.verticalCenter 
        anchors.verticalCenterOffset: Units.gu(-0.25)
        font.family: UI.fontFamily
        font.pixelSize: FontUtils.sizeToPixels(fontSize)
        font.bold: UI.fontBold
        color: UI.fontColor
		smooth: true
		visible: action === ""
    }

    /// shows an annotation
    // used e.g. for indicating the existence of extended keys

    Text {
        id: annotationLabel
        text: (panel.activeKeypadState !== "NORMAL") ? __annotationLabelShifted : __annotationLabelNormal

        anchors.right: parent.right
        anchors.rightMargin: Units.gu(1.00)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Units.gu(1.00)

        font.pixelSize: FontUtils.sizeToPixels(UI.annotationFontSize)
        font.bold: false
        style: Text.Raised
        styleColor: "white"
        color: UI.annotationFontColor
		smooth: true
        visible: UI.formFactor === "tablet" || !noMagnifier
    }

    PressArea {
        id: keyMouseArea
        anchors.fill: key

        onPressAndHold: {
            if (activeExtendedModel != undefined) {
                extendedKeysSelector.enabled = true
                extendedKeysSelector.extendedListModel = activeExtendedModel
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
        width: key.width + Units.gu(UI.magnifierHorizontalPadding)
        height: key.height + Units.gu(UI.magnifierVerticalPadding)
        text: keyLabel.text
        shown: key.pressed && !noMagnifier && !extendedKeysShown
    }
}
