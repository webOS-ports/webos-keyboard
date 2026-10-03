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

    /* Horizontal metric. KeyRow assigns keyUnit from its own weight sum; outside a
       KeyRow the key falls back to the global UI.keyWidth unit. */
    property real weight: 1
    property real keyUnit: UI.keyWidth
    property bool unitFromRow: false
    property alias pressArea: keyPressArea

    width: keyUnit * weight
    height: parent.height

    /* to be set in keyboard layouts */
    property string label: ""
    property string shifted: ""
    property var extended; // list of extended keys
    property var extendedShifted; // list of extended keys in shifted state

    property alias valueToSubmit: keyLabel.text

    property string action

    /// The character the Pre prints on the key beside its letter, typed while
    /// the orange key (AltKey) is active. Digits are printed red, as on the Pre.
    property string alt: ""
    /// Where it is printed: "left" or "right" of the letter, raised, or
    /// "inline" beside it at nearly full size (the Pre's "@0" key).
    property string altPosition: "right"
    readonly property bool __altActive: alt !== "" && UI.currentAltState !== "NORMAL"
    readonly property string __submitValue: __altActive ? alt : valueToSubmit
    property bool skipAutoCaps: false
    property bool alignTextRight: false

    /* design */
    property string imgNormal: UI.imageWhiteKey
    property string imgPressed: UI.imageWhiteKeyPressed
    // fontSize can be overwritten when using the component, e.g. SymbolShiftKey uses smaller fontSize
    property string fontSize: thumbKeyboard ? UI.thumbFontSize : UI.fontSize

    //We only want the maginifier for phone, so set the noMagnifier to true for tablets
    property bool noMagnifier: UI.formFactor==="tablet" ? true : false

    /// annotation shows a small label in the upper right corner
    // if the annotiation property is set, it will be used. If not, the first position in extended[] list or extendedShifted[] list will
    // be used, depending on the state. If no extended/extendedShifted arrays exist, no annotation is shown

    property string annotation: ""

    /*! indicates if te key is currently pressed/down*/
    property alias pressed: keyPressArea.isPressed

    /* internal */
    property string __annotationLabelNormal
    property string __annotationLabelShifted

    property alias charkeyPressArea: keyPressArea;

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
             if (extended) {
                if(imgNormal === UI.imageGreyKey && !action === "url") {
                    __annotationLabelNormal = extended[0]
                    __annotationLabelShifted = label
                }
                else{
                    __annotationLabelNormal = "…"
                }
            }
            if (extendedShifted) {
                if(imgNormal === UI.imageGreyKey && !action === "url") {
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
            left:   UI.formFactor==="tablet" ? 11 : 23
            top:    UI.formFactor==="tablet" ? 11 : 23
            right:  UI.formFactor==="tablet" ? 11 : 23
            bottom: UI.formFactor==="tablet" ? 11 : 23
        }
        anchors.centerIn: parent
        anchors.fill: key
        anchors.margins: thumbKeyboard ? Units.gu(-0.30) : UI.keyboardSizeChoice === "XS" ? Units.gu(-0.20) : Units.gu( UI.keyMargins );
        source: key.pressed ? key.imgPressed : key.imgNormal
    }

    /// label of the key
    //  the label is also the value subitted to the app

    Text {
        id: keyLabel
        text: (UI.currentShiftState === "NORMAL") ? label : shifted;
        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.horizontalCenterOffset: alt === "" ? 0
                                       : altPosition === "inline" ? -buttonImage.width * 0.12
                                       : altPosition === "left" ? buttonImage.width * 0.07
                                       : -buttonImage.width * 0.07
        anchors.verticalCenter: buttonImage.verticalCenter 
        anchors.verticalCenterOffset: UI.singleGlyphOffset + (alt !== "" && altPosition !== "inline" ? buttonImage.height * 0.06 : 0)
        font.family: UI.fontFamily
        font.pixelSize: thumbKeyboard ? FontUtils.sizeToPixels(fontSize)
                                      : UI.glyphFontPx(text, false)
        font.bold: UI.fontBold
        // Letters are shown as capitals, as printed on the Pre's keys; what is
        // typed still follows the shift state (text is unchanged).
        font.capitalization: UI.formFactor === "phone" && !thumbKeyboard ? Font.AllUppercase : Font.MixedCase
        color: UI.fontColor
        style: UI.glyphStyle(UI.fontColor, UI.fontStyleColor)
        styleColor: UI.fontStyleColor
        smooth: true
        visible: action === "" || action === "url"
        // With the orange key active the printed alternates are what will be
        // typed, so the letters step back.
        opacity: UI.currentAltState !== "NORMAL" && alt !== "" ? 0.35 : 1
    }

    Text {
        id: altLabel
        visible: alt !== "" && keyLabel.visible
        text: alt
        font.family: UI.fontFamily
        font.bold: true
        font.pixelSize: keyLabel.font.pixelSize * (altPosition === "inline" ? 0.9 : 0.72)
        color: /^[0-9]$/.test(alt) ? "#F2402E" : UI.fontColor
        style: Text.Normal
        smooth: true

        anchors.left: altPosition === "left" ? undefined : keyLabel.right
        anchors.leftMargin: altPosition === "inline" ? keyLabel.font.pixelSize * 0.08 : -keyLabel.font.pixelSize * 0.04
        anchors.right: altPosition === "left" ? keyLabel.left : undefined
        anchors.rightMargin: -keyLabel.font.pixelSize * 0.04
        anchors.verticalCenter: altPosition === "inline" ? keyLabel.verticalCenter : undefined
        anchors.bottom: altPosition === "inline" ? undefined : keyLabel.verticalCenter
        anchors.bottomMargin: -keyLabel.font.pixelSize * 0.08
    }

    /// shows an annotation
    // used e.g. for indicating the existence of extended keys

    Text {
        id: annotationLabel
        text: (UI.currentShiftState !== "NORMAL") ? __annotationLabelShifted : __annotationLabelNormal

        anchors.right: thumbKeyboard ? undefined : parent.right
        anchors.rightMargin: UI.elipsisMargin
        anchors.horizontalCenter: thumbKeyboard ? parent.horizontalCenter : undefined
        anchors.horizontalCenterOffset: thumbKeyboard ? UI.keyWidth / 14 : 0

        anchors.bottom: parent.bottom
        anchors.bottomMargin: thumbKeyboard ? UI.keyHeight / 1.5 : UI.elipsisMargin

        font.family: UI.fontFamily
        font.pixelSize: thumbKeyboard ? FontUtils.sizeToPixels(UI.thumbAnnotationFontSize) : UI.elipsisFontPx
        font.bold: false
        style: UI.glyphStyle(UI.annotationFontColor, UI.annotationStyleColor)
        styleColor: UI.annotationStyleColor
        color: UI.annotationFontColor
        smooth: true
        // The Pre prints no hint for the long-press accents; only the tablet
        // keeps its "..." marker.
        visible: UI.formFactor === "tablet"
                 && activeExtendedModel !== undefined
    }

    PressArea {
        id: keyPressArea
        anchors.fill: key
        onlyExclusive: action !== "" && action !== "url" && action !== "space"

        onKeyPressedAndHold: {
            if (activeExtendedModel != undefined) {
                UI.showExtendedKeys(activeExtendedModel, key);
                annotationLabel.visible = true
            }
        }

        onKeyReleased: {
            if (!extendedKeysShown) {
                if (maliit_input_method.useAudioFeedback)
                    audioFeedback.play();

                var usedAlt = __altActive;
                event_handler.onKeyReleased(__submitValue, action);
                if (!skipAutoCaps && !usedAlt)
                    if (UI.currentShiftState === "SHIFTED" && UI.currentSymbolState === "CHARACTERS")
                        UI.shiftedKeySent();
                // A single tap of the orange key covers the next key only.
                if (UI.currentAltState === "ALT")
                    UI.currentAltState = "NORMAL";
            }
            else if (activeExtendedModel != undefined) {
                UI.showExtendedKeys(activeExtendedModel, key);
            }
            else {
                UI.hideCurrentPopover();
            }

        }
        onKeyPressed: {
            event_handler.onKeyPressed(__submitValue, action);
        }
    }

    Connections {
        target: swipeArea.drag
        function onActiveChanged() {
            if (swipeArea.drag.active)
                keyPressArea.cancelPress();
        }
    }

    Magnifier {
        anchors.horizontalCenter: buttonImage.horizontalCenter
        anchors.bottom: buttonImage.top
        width: key.width + Units.gu(UI.magnifierHorizontalPadding)
        height: key.height + Units.gu(UI.magnifierVerticalPadding)
        text: __altActive ? alt : keyLabel.text
        shown: key.pressed && !noMagnifier && !extendedKeysShown
    }
}
