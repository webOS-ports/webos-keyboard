/*
 * Copyright 2012 Canonical Ltd.
 * Copyright (C) 2015 Christophe Chapuis <chris.chapuis@gmail.com>
 * Copyright (C) 2015 Herman van Hazendonk <github.com@herrie.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

.pragma library

var keyMargins      =   0 //gu

var keyHeight       =   {"tabletLandscape" : "7",
                         "tabletPortrait"  : "7",
                         "phonePortrait"   : "6.1",
                         "phoneLandscape"  : "4.3"};   // gu

var keyHeightRatio = [ { "name": "XS", "ratio" : 0.7147058877 },	/* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "S", "ratio" : 0.857352948 }, 	/* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "M", "ratio" : 1.0 },         	/* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "L", "ratio" : 1.15588236 } ];	/* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/

/* Golden reference: TabletKeyboard sets row 0 from key-gray-short.png (110/2 = 55px)
   and rows 1-4 from key-white.png (140/2 = 70px). The ratio is a property of the
   artwork, so it is the same for every keyboard size: 55 / 70 = 0.785714. */
var topRowKeyHeightRatio = [ { "name": "XS", "ratio" : 0.785714286 },
                       { "name":  "S", "ratio" : 0.785714286 },
                       { "name":  "M", "ratio" : 0.785714286 },
                       { "name":  "L", "ratio" : 0.785714286 } ];

var numKeyWidthRatio       =   [ { "name": "XS", "ratio" : 0.925 },	/* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "S", "ratio" : 0.925 }, 	/* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "M", "ratio" : 0.925 },         	/* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "L", "ratio" : 0.925 } ];	/* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/

/* Glyph sizes come from {Tablet,Phone}Keyboard::drawKeyCap, and the two form
   factors have to be translated differently.

   The tablet sizes are fixed pixels that do not move when the user changes the
   keyboard size - drawKeyCap only shrinks them once half the key height drops below
   the cap. So they are grid-unit constants, which at the TouchPad's gridUnit of 10
   come out as exactly the reference 26 / 24 / 22 / 14 px, and still scale with
   density on other hardware.

   The phone reference has no size setting at all: its key height is fixed per
   orientation, 90px in portrait, with a 24px glyph. A grid-unit constant would
   therefore be wrong on a dense phone - it would give a 43px glyph on a 110px key
   where the reference wants 29. What carries over is the ratio, so the phone values
   are fractions of the key height.

     charFont    a key showing one glyph              26px tablet, 24/90 phone
     dualFont    a key showing a glyph and its alt    24px tablet, 24/90 phone
     labelFont   a multi-character label ("Tab")      22px tablet, 22/90 phone
     elipsisFont the "..." extended-keys hint         14px tablet, 14/90 phone */
var tabletCharFontCap    =   2.6;   // gu
var tabletDualFontCap    =   2.4;   // gu
var tabletLabelFontCap   =   2.2;   // gu
var tabletElipsisFontCap =   1.4;   // gu

var phoneRefKeyHeight    =   90.25; // px, PhoneKeymap portrait row height
var phoneCharFontPx      =   24;
var phoneDualFontPx      =   24;
var phoneLabelFontPx     =   22;
var phoneElipsisFontPx   =   14;
var phonePreviewFontPx   =   32;    // the magnified key preview, drawn bold

/* boostSize(): the reference adds 2px to '. , ; : \' "' so they do not look lost
   next to a letter. */
var boostedGlyphs   =   ".,;:'\"";
var boostFontSize   =   0.2;                              // gu

/* The extended-keys popup: cPopupFontSize is 22px, dropping to 22 - 8 once the
   label runs to six characters or more. The popup artwork is the same size on both
   form factors, so these stay grid-unit constants. */
var popupFontCap      =   2.2;                            // gu
var popupFontCapLong  =   1.4;                            // gu
var popupLongAt       =   6;                              // characters

var fontSize        =   {"tablet" : "22pt",
                         "phone"  : "16pt"};

var thumbFontSize   =   "18pt";
var thumbAnnotationFontSize   =   "10pt";

var fontFamily      =   "Prelude";

var fontColor       =   {"tablet" : "#141414",   /* cActiveColor, tablet */
                         "phone"  : "#D2D2D2"};  /* cActiveColor, phone  */

/* drawKeyCap only sets bold for multi-character function-key labels and for the
   magnified preview, never for a plain letter. */
var fontBold        =   {"tablet" : false,
                         "phone"  : false};

var fontBoldAction  =   true

var annotationFontSize = {"tablet" : "14pt",
                         "phone"  : "14pt"};

var annotationMargins = 0.7; // gu

var annotationFontColor = {"tablet" : "#646464",   /* cDisabledColor, tablet */
                           "phone"  : "#808080"};  /* cDisabledColor, phone  */

var magnifierFontColor = {"tablet" : "#141414",
                           "phone"  : "#141414"};

var extendedHighLightColor = {"tablet" : "#4B97DE",
                           "phone"  : "#4B97DE"};

var extendedFontColor = {"tablet" : "#141414",
                         "phone"  : "#141414"};

var greyColor    =    {"tablet" : "#D2D2D2",
                     "phone"  : "#D2D2D2"};

/* magnifier */
var magnifierHorizontalPadding = 0; // gu, stretch the magnifier a little bit in x-axis
var magnifierVerticalPadding = 0; // gu, stretch the magnifier a little bit in y-axis

/* special keys */
var actionKeyPadding = 2;  // gu, action keys are a bit bigger

var smallFontSize = {"tablet" : "14pt",
                     "phone"  : "14pt"}; 

var xsFontSize = {"tablet" : "14pt",
                  "phone"  : "10pt"};

/* extended keys */
var popoverTopMargin   = 1; // gu

var popoverFontSize   = {"tablet" : "32pt",
                         "phone"  : "32pt"};    // gu

var popoverFontSizeLong   = {"tablet" : "32pt",
                             "phone"  : "24pt"};    // gu

var imageWhiteKey        = { "tablet" : "../images/tablet/key_bg_white.png",
                             "phone"  : "../images/phone/key_bg_white.png"  };

var imageWhiteKeyPressed = { "tablet" : "../images/tablet/key_bg_white_active.png",
                             "phone"  : "../images/phone/key_bg_white_active.png"  };

var imageBlackKey        = { "tablet" : "../images/tablet/key_bg_black.png",
                             "phone"  : "../images/phone/key_bg_black.png"  };

var imageBlackKeyPressed = { "tablet" : "../images/tablet/key_bg_black_active.png",
                             "phone"  : "../images/phone/key_bg_black_active.png"  };

var imageGreyKey         = { "tablet" : "../images/tablet/key_bg_grey.png",
                             "phone"  : "../images/phone/key_bg_grey.png"  };

var imageGreyKeyPressed  = { "tablet" : "../images/tablet/key_bg_grey_active.png",
                             "phone"  : "../images/phone/key_bg_grey_active.png"  };

var imagePopover          = { "tablet" : "../images/tablet/keyboard_popover.png",
                             "phone"  : "../images/phone/keyboard_popover.png"  };

var imagePopupKey        = { "tablet" : "../images/tablet/popup_key_inactive.png",
                             "phone"  : "../images/phone/popup_key_inactive.png"  };

var imagePopupKeyPressed = { "tablet" : "../images/tablet/popup_key_active.png",
                             "phone"  : "../images/phone/popup_key_active.png"  };

var imagePopupBgLeft     = { "tablet" : "../images/tablet/popup-bg-left.png",
                             "phone"  : "../images/phone/popup-bg-left.png"  };

var imagePopupBgBetween  = { "tablet" : "../images/tablet/popup-bg-between.png",
                             "phone"  : "../images/phone/popup-bg-between.png"  };

var imagePopupBgCaret    = { "tablet" : "../images/tablet/popup-bg-caret.png",
                             "phone"  : "../images/phone/popup-bg-caret.png"  };

var imagePopupBgRight    = { "tablet" : "../images/tablet/popup-bg-right.png",
                             "phone"  : "../images/phone/popup-bg-right.png"  };

var imageShiftKey        = { "tablet" : "../images/tablet/key_bg_shift_on.png",
                             "phone"  : "../images/phone/key_bg_shift_on.png"  };

var imageShiftKeyPressed = { "tablet" : "../images/tablet/key_bg_shift_on_active.png",
                             "phone"  : "../images/phone/key_bg_shift_on_active.png"  };

var imageShiftLockKey    = { "tablet" : "../images/tablet/key_bg_shift.png",
                             "phone"  : "../images/phone/key_bg_shift.png"  };

var imageShiftLockKeyPressed    = { "tablet" : "../images/tablet/key_bg_shift_active.png",
                                    "phone"  : "../images/phone/key_bg_shift_active.png"  };

var top_margin = 0.5;  // gu - golden keyboardTopPading is 4/5/5/6px for XS/S/M/L
var bottom_margin = 0; // gu

var wordribbonHeight = 5.5; //gu - golden candidate bar is a fixed 55px (key-gray-short.png / 2)
