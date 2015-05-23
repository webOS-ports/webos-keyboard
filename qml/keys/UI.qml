/*
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
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

pragma Singleton

import QtQuick 2.0
import LunaNext.Common 0.1

import "key_constants.js" as DesignConstants

QtObject {
    // set from keyboard
    property bool isLandscape: false
    // internal helper
    property string formFactor: Settings.tabletUi ? "tablet" : "phone";
    onFormFactorChanged: console.log("formFactor = " + formFactor);

    // UI design values (taken from key_constants.js)
    property real keyMargins: DesignConstants.keyMargins;
    property real keyHeight: DesignConstants.keyHeight[formFactor + (isLandscape ? "Landscape" : "Portrait")];

    property string fontSize: DesignConstants.fontSize[formFactor];
    property string fontFamily: DesignConstants.fontFamily;
    property color fontColor: DesignConstants.fontColor[formFactor];
    property bool fontBold: DesignConstants.fontBold[formFactor];
    property bool fontBoldAction: DesignConstants.fontBoldAction;

    property string annotationFontSize: DesignConstants.annotationFontSize[formFactor];
    property real annotationMargins: DesignConstants.annotationMargins;
    property color annotationFontColor: DesignConstants.annotationFontColor[formFactor];

    property color magnifierFontColor: DesignConstants.magnifierFontColor[formFactor];
    property color extendedHighLightColor: DesignConstants.extendedHighLightColor[formFactor];
    property color extendedFontColor: DesignConstants.extendedFontColor[formFactor];
    property color greyColor: DesignConstants.greyColor[formFactor];

    /* magnifier */
    property real magnifierHorizontalPadding: DesignConstants.magnifierHorizontalPadding;
    property real magnifierVerticalPadding: DesignConstants.magnifierVerticalPadding;

    /* special keys */
    property real actionKeyPadding: DesignConstants.actionKeyPadding;
    property string symbolShiftKeyFontSize: DesignConstants.symbolShiftKeyFontSize[formFactor];
    property string smallFontSize: DesignConstants.smallFontSize[formFactor];

    /* extended keys */
    property real popoverTopMargin: DesignConstants.popoverTopMargin;
    property string popoverFontSize: DesignConstants.popoverFontSize[formFactor];

    property string imageWhiteKey: DesignConstants.imageWhiteKey[formFactor];
    property string imageWhiteKeyPressed: DesignConstants.imageWhiteKeyPressed[formFactor];
    property string imageBlackKey: DesignConstants.imageBlackKey[formFactor];
    property string imageBlackKeyPressed: DesignConstants.imageBlackKeyPressed[formFactor];
    property string imageGreyKey: DesignConstants.imageGreyKey[formFactor];
    property string imageGreyKeyPressed: DesignConstants.imageGreyKeyPressed[formFactor];
    property string imagePopover: DesignConstants.imagePopover[formFactor];
    property string imagePopupKey: DesignConstants.imagePopupKey[formFactor];
    property string imagePopupKeyPressed: DesignConstants.imagePopupKeyPressed[formFactor];

    property string imagePopupBgLeft: DesignConstants.imagePopupBgLeft[formFactor];
    property string imagePopupBgBetween: DesignConstants.imagePopupBgBetween[formFactor];
    property string imagePopupBgCaret: DesignConstants.imagePopupBgCaret[formFactor];
    property string imagePopupBgRight: DesignConstants.imagePopupBgRight[formFactor];

    property string imageShiftKey: DesignConstants.imageShiftKey[formFactor];
    property string imageShiftKeyPressed: DesignConstants.imageShiftKeyPressed[formFactor];
    property string imageShiftLockKey: DesignConstants.imageShiftLockKey[formFactor];
    property string imageShiftLockKeyPressed: DesignConstants.imageShiftLockKeyPressed[formFactor];
    property string imageSpaceKey: formFactor === "tablet" ? imageWhiteKey : imageBlackKey;
    property string imageSpaceKeyPressed: formFactor === "tablet" ? imageWhiteKeyPressed : imageBlackKeyPressed;

    property real top_margin: DesignConstants.top_margin;
    property real bottom_margin: DesignConstants.bottom_margin;
    property real wordribbonHeight: DesignConstants.wordribbonHeight;

    function getHeightRatio(screenHeight, isLandscape, sizeChoice) {
        if( formFactor === "tablet" ){
            var keyboardHeight = { "XS" : 0.31640625,		/* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                                    "S" : 0.379557292, 	/* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                                    "M" : 0.44270833, 		/* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                                    "L" : 0.51171875 }; 	/* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
            return keyboardHeight[sizeChoice];
        }
        else { //formFactor === "phone"

            if( isLandscape ) {
                return 0.3385417
            }
            else{
                return 0.368164
            }
        }
    }
}
