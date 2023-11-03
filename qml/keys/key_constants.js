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

var topRowKeyHeightRatio = [ { "name": "XS", "ratio" : 0.82 },	/* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "S", "ratio" : 0.74 }, 	/* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "M", "ratio" : 0.74 },         	/* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "L", "ratio" : 0.74 } ];	/* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/

var numKeyWidthRatio       =   [ { "name": "XS", "ratio" : 0.925 },	/* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "S", "ratio" : 0.925 }, 	/* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "M", "ratio" : 0.925 },         	/* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
                       { "name":  "L", "ratio" : 0.925 } ];	/* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/

var fontSize        =   {"tablet" : "22pt",
                         "phone"  : "16pt"};

var thumbFontSize   =   "18pt";
var thumbAnnotationFontSize   =   "10pt";

var fontFamily      =   "Prelude";

var fontColor       =   {"tablet" : "#141414",
                         "phone"  : "#FFFFFF"};

var fontBold        =   {"tablet" : false,
                         "phone"  : true};

var fontBoldAction  =   true

var annotationFontSize = {"tablet" : "14pt",
                         "phone"  : "14pt"};

var annotationMargins = 0.7; // gu

var annotationFontColor = {"tablet" : "#646464",
                           "phone"  : "#646464"};

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

var top_margin = 1.1;  // gu
var bottom_margin = 0; // gu

var wordribbonHeight = 5; //gu
