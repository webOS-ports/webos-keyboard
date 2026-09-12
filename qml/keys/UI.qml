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
    property string keyboardSizeChoice: maliit_input_method.keyboardSize
    property real keyWidth: 5 // arbitrary value, will be overwritten when KeyPad computes the ideal key width
    property bool extendedKeysShown: false
    property bool isShiftKeyPressed: false
    property string currentShiftState: "NORMAL"  // can be "NORMAL", "SHIFTED" or "CAPSLOCK"
    property string currentSymbolState: "CHARACTER" // can be "CHARACTER" or "SYMBOL"
    property string currentAlternativeLayout: maliit_input_method.keyboardLayout === "LuneOS" ? "" : maliit_input_method.keyboardLayout

    // keyboard-wide signals
    signal showExtendedKeys(variant keysExtendedModel, Item keyItem);
    signal showKeyboardSizeMenu(Item keyItem);
    signal showLanguagesMenu(Item keyItem);
    signal showAlternativeLayoutsMenu(Item keyItem);
    signal hideCurrentPopover();

    signal shiftedKeySent();

    // internal helper
    property string formFactor: Settings.tabletUi ? "tablet" : "phone";

    // UI design values (taken from key_constants.js)
    property real keyMargins: DesignConstants.keyMargins;
    property real keyHeight: DesignConstants.keyHeightRatio[keyboardSizeChoices.indexOf(keyboardSizeChoice)].ratio * DesignConstants.keyHeight[formFactor + (isLandscape ? "Landscape" : "Portrait")];
    
    property real numKeyWidthRatio: DesignConstants.numKeyWidthRatio[keyboardSizeChoices.indexOf(keyboardSizeChoice)].ratio;
    property real topRowKeyHeightRatio: DesignConstants.topRowKeyHeightRatio[keyboardSizeChoices.indexOf(keyboardSizeChoice)].ratio;
        
    property variant keyboardSizeChoices: DesignConstants.keyHeightRatio.map(function(obj) {return obj.name});

    /* Key height in pixels. keyHeight itself is in grid units; mixing the two was
       the reason the glyph offsets below drifted. */
    property real keyHeightPx: Units.gu(keyHeight);

    /* Tablet: start from the fixed cap, then clamp to half the key height so a short
       key still fits its glyph, exactly as drawKeyCap does.
       Phone: scale the reference glyph by how our key height compares to the
       reference's, since the phone keyboard has no size setting to clamp against. */
    function __capped(gu) {
        return Math.min(Units.gu(gu), Math.floor((keyHeightPx + 1) / 2));
    }
    function __scaled(px) {
        return keyHeightPx * px / DesignConstants.phoneRefKeyHeight;
    }
    readonly property bool __tablet: formFactor === "tablet"

    property real charFontPx: __tablet ? __capped(DesignConstants.tabletCharFontCap)
                                       : __scaled(DesignConstants.phoneCharFontPx);
    property real dualFontPx: __tablet ? __capped(DesignConstants.tabletDualFontCap)
                                       : __scaled(DesignConstants.phoneDualFontPx);
    property real labelFontPx: __tablet ? Math.min(dualFontPx, Units.gu(DesignConstants.tabletLabelFontCap))
                                        : __scaled(DesignConstants.phoneLabelFontPx);
    property real elipsisFontPx: __tablet ? Units.gu(DesignConstants.tabletElipsisFontCap)
                                          : __scaled(DesignConstants.phoneElipsisFontPx);
    property real boostFontPx: Units.gu(DesignConstants.boostFontSize);
    property string boostedGlyphs: DesignConstants.boostedGlyphs;

    /* Where the two glyphs sit on a dual-label key. The reference trims 4px off the
       bottom of the key, splits what is left into thirds, puts the alt glyph in the
       top third and the primary in the bottom one - +9px and -14px from the centre
       of a 70px key. The horizontal variant used on the number row splits the key
       into halves instead. */
    /* drawKeyCap renders each glyph into an explicit box rather than centring it on
       a point, which is what keeps a comma off the "..." hint below it: box height is
       a third of the key once 4px is trimmed off the bottom, the alt box starts 10px
       down from the top, and the primary box ends 10px above the trimmed bottom. */
    property real dualBoxHeight: (keyHeightPx - (__tablet ? Units.gu(0.4) : __scaled(4))) / 3;
    property real dualAltTop: __tablet ? Units.gu(1.0) : __scaled(10);
    property real dualPrimaryBottom: __tablet ? Units.gu(1.4) : __scaled(14);
    property real singleGlyphOffset: __tablet ? -Units.gu(0.2) : -__scaled(2);
    property real dualPrimaryFactor: 0.206;
    property real dualAltFactor: -0.217;

    /* The "..." hint sits 9px in from the right and bottom edges. */
    property real elipsisMargin: __tablet ? Units.gu(0.9) : __scaled(9);

    property real popupFontPx: Units.gu(DesignConstants.popupFontCap);
    property real popupFontPxLong: Units.gu(DesignConstants.popupFontCapLong);
    property int popupLongAt: DesignConstants.popupLongAt;
    property real previewFontPx: __scaled(DesignConstants.phonePreviewFontPx);

    function popupGlyphFontPx(text) {
        return (text && text.length >= popupLongAt) ? popupFontPxLong : popupFontPx;
    }

    function glyphFontPx(text, isDual) {
        var size = isDual ? dualFontPx : charFontPx;
        if (text && text.length > 1)
            return labelFontPx;
        if (text && text.length === 1 && boostedGlyphs.indexOf(text) >= 0)
            return size + boostFontPx;
        return size;
    }

    property string fontSize: DesignConstants.fontSize[formFactor];
    property string thumbFontSize: DesignConstants.thumbFontSize;
    property string thumbAnnotationFontSize: DesignConstants.thumbAnnotationFontSize;
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
    property string smallFontSize: DesignConstants.smallFontSize[formFactor];
    property string xsFontSize: DesignConstants.xsFontSize[formFactor];

    /* extended keys */
    property real popoverTopMargin: DesignConstants.popoverTopMargin;
    property string popoverFontSize: DesignConstants.popoverFontSize[formFactor];
    property string popoverFontSizeLong: DesignConstants.popoverFontSizeLong[formFactor];

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
}
