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
	property bool thumbKeyboard: false

    width: UI.keyWidth
    height: parent.height

    /* to be set in keyboard layouts */
    property string label: ""
    property string shifted: ""
    property var extended; // list of extended keys
    property var extendedShifted; // list of extended keys in shifted state

    property string valueToSubmit: UI.currentShiftState === "NORMAL" ? keyLabel.text : annotationLabel.text

    property string action
    property bool noMagnifier: UI.formFactor==="tablet" ? true : false
    property bool skipAutoCaps: false

    /* design */
    property bool useHorizontalLayout: false;

    property string imgNormal: UI.imageGreyKey
    property string imgPressed: UI.imageGreyKeyPressed
    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property string fontSize: thumbKeyboard ? UI.thumbFontSize : UI.fontSize

    /// annotation shows a small label in the upper right corner
    // if the annotiation property is set, it will be used. If not, the first position in extended[] list or extendedShifted[] list will
    // be used, depending on the state. If no extended/extendedShifted arrays exist, no annotation is shown
    property string annotation: ""

    /*! indicates if te key is currently pressed/down*/
    property alias pressed: keyMouseArea.isPressed

    /* internal */
    property string __annotationLabelNormal
    property string __annotationLabelShifted

    /**
     * this property specifies if the key can submit its value or not (e.g. when the popover is shown, it does not commit its value)
     */

    property bool extendedKeysShown: UI.extendedKeysShown

    /*
     * label changes when keyboard is in shifted mode
     * extended keys change as well when shifting keyboard, typically lower-uppercase: ê vs Ê
     */

    property string oskState: UI.currentShiftState
    property var activeExtendedModel: (UI.currentShiftState === "NORMAL" || !extendedShifted) ? extended : extendedShifted

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
        border {
            left:   UI.formFactor==="tablet" ? 14 : 24
            top:    UI.formFactor==="tablet" ? 13 : 24
            right:  UI.formFactor==="tablet" ? 14 : 24
            bottom: UI.formFactor==="tablet" ? 17 : 24
        }
        anchors.centerIn: parent
        anchors.fill: key
        anchors.margins: thumbKeyboard ? Units.gu(-0.30) : UI.keyboardSizeChoice === "XS" ? Units.gu(-0.20) : Units.gu( UI.keyMargins );
        source: key.pressed ? key.imgPressed : key.imgNormal
    }

    /// label of the key
    //  the label is also the value submitted to the app

    Text {
        id: keyLabel
        text: label

        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.horizontalCenterOffset: thumbKeyboard ? UI.keyWidth / 14 : useHorizontalLayout ? UI.keyWidth / 6 : 0

        anchors.verticalCenter: parent.verticalCenter
		anchors.verticalCenterOffset: useHorizontalLayout || thumbKeyboard ? UI.keyHeight / -2 : UI.keyHeight / 0.6  //Units.gu(-0.25)
		//anchors.verticalCenterOffset: thumbKeyboard? Units.gu(0) : useHorizontalLayout ? UI.keyHeight / -2 : UI.keyHeight / 0.6 
		
        font.family: UI.fontFamily
        font.pixelSize: (UI.currentShiftState === "NORMAL") ? FontUtils.sizeToPixels(UI.annotationFontSize) : FontUtils.sizeToPixels(UI.xsFontSize)
        font.bold: UI.fontBold
        color: (UI.currentShiftState === "NORMAL") ? UI.fontColor : UI.annotationFontColor
        style: (UI.currentShiftState === "NORMAL") ? Text.Raised : Text.Normal
        styleColor: "white"
        smooth: true
    }

    /// shows an annotation
    // used e.g. for indicating the existence of extended keys

    Text {
        id: annotationLabel
        text: __annotationLabelNormal

        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.horizontalCenterOffset: thumbKeyboard ? UI.keyWidth / -14 : useHorizontalLayout ? UI.keyWidth / -6 : 0

        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: useHorizontalLayout || thumbKeyboard ? UI.keyHeight / -2 : UI.keyHeight / -0.4  //Units.gu(-0.25)
		


        font.family: UI.fontFamily
        font.pixelSize: thumbKeyboard ? FontUtils.sizeToPixels(UI.thumbAnnotationFontSize) : (UI.currentShiftState === "NORMAL") ? FontUtils.sizeToPixels(UI.annotationFontSize) : FontUtils.sizeToPixels(UI.xsFontSize)
        font.bold: false
        color: (UI.currentShiftState !== "NORMAL") ? UI.fontColor : UI.annotationFontColor
        style: (UI.currentShiftState !== "NORMAL") ? Text.Raised : Text.Normal
        styleColor: "white"
        smooth: true
		}
	
	Text {
        id: annotationLabel2
		text: "…" //__annotationLabelNormal

        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.horizontalCenterOffset: thumbKeyboard ? UI.keyWidth / 14 : UI.formFactor === "phone" && !UI.isLandscape ? UI.keyWidth / 4 : UI.keyWidth / 3.5

        /*anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: thumbKeyboard ? UI.keyHeight / 14 : useHorizontalLayout ? UI.keyHeight / 1.5 : UI.formFactor === "phone" && !UI.isLandscape ? UI.keyHeight / 0.25 : UI.keyHeight / 0.5 */
		
		anchors.bottom: parent.bottom
        //anchors.bottomMargin: thumbKeyboard ? UI.keyHeight / 14 : UI.formFactor === "tablet" ? UI.keyHeight / 0.8 : UI.keyHeight / 3  //Units.gu(1.00) : Units.gu(0.50)
		anchors.bottomMargin: thumbKeyboard ? UI.keyHeight / 1.5 : UI.formFactor === "tablet" ? UI.keyHeight / 0.8 : UI.keyHeight / 3  //Units.gu(1.00) : Units.gu(0.50)


        font.family: UI.fontFamily
        font.pixelSize: FontUtils.sizeToPixels(UI.annotationFontSize)
        font.bold: false
        style: Text.Raised
        styleColor: "white"
        color: UI.fontColor //: UI.annotationFontColor
		smooth: true
    }

    PressArea {
        id: keyMouseArea
        anchors.fill: key

        onKeyPressedAndHold: {
            if (activeExtendedModel !== undefined) {
                UI.showExtendedKeys(activeExtendedModel, key);
            }
        }

        onKeyReleased: {
            if (!extendedKeysShown) {
                if (maliit_input_method.useAudioFeedback)
                    audioFeedback.play();

                event_handler.onKeyReleased(valueToSubmit, action);
                if (!skipAutoCaps)
                    if (UI.currentShiftState === "SHIFTED" && UI.currentSymbolState === "CHARACTERS")
                        UI.currentShiftState = "NORMAL";
            }
        }
        onKeyPressed: {
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
        text: valueToSubmit
        shown: key.pressed && !noMagnifier && !extendedKeysShown
    }
}
