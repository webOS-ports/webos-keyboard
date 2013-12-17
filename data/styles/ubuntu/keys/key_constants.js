/*
 * Copyright 2012 Canonical Ltd.
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

var keyMargins  =   1.8;    // dp
var fontSize    =   2;    // gu
var fontFamily  =   "Prelude";
var fontColor   =   "#464646"
var fontBold    =   true

var annotationFontSize = 1.4; // gu
var annotationMargins = 0.7; // gu
var annotationFontColor = "#575757"

var greyColor	=	"#C9C9C9"

/* magnifier */
var magnifierHorizontalPadding = 1; // gu, stretch the magnifier a little bit in x-axis
var magnifierVerticalPadding = 1; // gu, stretch the magnifier a little bit in y-axis

/* special keys */
var actionKeyPadding = 2;  // gu, action keys are a bit bigger
var symbolShiftKeyFontSize = 2; // gu
var smallFontSize = 1.5; // gu, for keys that show more than one char, e.g. '.com'

/* extended keys */
var popoverCellPadding = 2.2; // gu
var popoverTopMargin   = 10; // dp
var popoverEdgeMargin = 2.2; // gu
var popoverSquat      = 3; // gu, when no wordribbon, avoid click-through above input trap

var imageWhiteKey        = "../images/key_bg_white.png"
var imageWhiteKeyPressed = "../images/key_bg_white_active.png"

var imageBlackKey        = "../images/key_bg_black"
var imageBlackKeyPressed = "../images/key_bg_black_active"

var imageGreyKey        = "../images/key_bg_grey.png"
var imageGreyKeyPressed = "../images/key_bg_grey_active.png"


var top_margin = 1.35;  // gu
var bottom_margin = 1.00; // gu


var urlLayoutLeftSpacerSize = 5; // gu
var emailLayoutUrlKeyPadding = 4; // gu

/* language menu */
var languageMenuListViewPadding = 2.22; // gu
var languageMenuCorner = 2.5; // gu


var wordribbonHeight = 50;

var phoneKeyboardHeightPortrait = 0.365;
var phoneKeyboardHeightLandscape = 0.50;

var tabletKeyboardHeightPortrait = 0.28;
var tabletKeyboardHeightLandscape = 0.38;
