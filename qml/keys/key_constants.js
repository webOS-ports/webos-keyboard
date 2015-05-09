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

var keyMargins  	=   0//1 //1.8;    // gu
var fontSize    	=   22;    // gu
var fontFamily  	=   "Prelude";
var fontColor  		=   "#141414"
var fontBold    	=   false
var fontBoldAction  =   true

var annotationFontSize = 14; // gu
var annotationMargins = 0.7; // gu
var annotationFontColor = "#646464"

var greyColor	=	"#D2D2D2"

/* magnifier */
var magnifierHorizontalPadding = 1; // gu, stretch the magnifier a little bit in x-axis
var magnifierVerticalPadding = 1; // gu, stretch the magnifier a little bit in y-axis

/* special keys */
var actionKeyPadding = 2;  // gu, action keys are a bit bigger
var symbolShiftKeyFontSize = 22; // gu
var smallFontSize = 14; // gu, for keys that show more than one char, e.g. '.com'

/* extended keys */
var popoverCellPadding = 2.2; // gu
var popoverTopMargin   = 10; // dp
var popoverEdgeMargin = 2.2; // gu
var popoverSquat      = 3; // gu, when no wordribbon, avoid click-through above input trap

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

var imagePopover 	     = { "tablet" : "../images/tablet/keyboard_popover.png",
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

var imagePopupBgLeft2    = { "tablet" : "../images/tablet/popup-bg-2-left.png",
                             "phone"  : "../images/phone/popup-bg-2-left.png"  };

var imagePopupBgBetween2 = { "tablet" : "../images/tablet/popup-bg-2-between.png",
                             "phone"  : "../images/phone/popup-bg-2-between.png"  };

var imagePopupBgCaret2   = { "tablet" : "../images/tablet/popup-bg-2-caret.png",
                             "phone"  : "../images/phone/popup-bg-2-caret.png"  };

var imagePopupBgRight2   = { "tablet" : "../images/tablet/popup-bg-2-right.png",
                             "phone"  : "../images/phone/popup-bg-2-right.png"  };

var imageShiftKey        = { "tablet" : "../images/tablet/key_bg_shift_on.png",
                             "phone"  : "../images/phone/key_bg_shift_on.png"  };

var imageShiftKeyPressed = { "tablet" : "../images/tablet/key_bg_shift_on_active.png",
                             "phone"  : "../images/phone/key_bg_shift_on_active.png"  };

var imageShiftLockKey    = { "tablet" : "../images/tablet/key_bg_shift.png",
                             "phone"  : "../images/phone/key_bg_shift.png"  };

var imageShiftLockKeyPressed    = { "tablet" : "../images/tablet/key_bg_shift_active.png",
                                    "phone"  : "../images/phone/key_bg_shift_active.png"  };
							 
var top_margin = 1.1; //.35;  // gu
var bottom_margin = 0; // gu


var urlLayoutLeftSpacerSize = 5; // gu
var emailLayoutUrlKeyPadding = 4; // gu

/* language menu */
var languageMenuListViewPadding = 2.22; // gu
var languageMenuCorner = 2.5; // gu


var wordribbonHeight = 50;

//var phoneKeyboardHeightPortrait = 0.365;
//var phoneKeyboardHeightLandscape = 0.50;

var phoneKeyboardHeightLandscape = 0.51171875; /* 240 / 1024 based on Touchpads resolution, might need adjusting for widescreen tablets*/	


/* some legacy code that might be of use to us in the future for VKB in landscape on phones:

			height = width * 200 / 320;	// arbitrary proportions...
			if (size.height() < 480)
				height = 160;	// shrink height for Pixie & Broadway, and some horizontal layout
			if (height > 300)
				height = 300;

*/

var tabletKeyboardHeightPortrait = 0.28;
var tabletKeyboardHeightLandscapeXS = 0.31640625; /* 243 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
var tabletKeyboardHeightLandscapeS = 0.379557292; /* (340+243 / 2)  / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
var tabletKeyboardHeightLandscapeM = 0.44270833; /* 340 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/
var tabletKeyboardHeightLandscapeL = 0.51171875; /* 393 / 768 based on Touchpads resolution, might need adjusting for widescreen tablets*/

