/*
 * Copyright (C) 2013 Christophe Chapuis <chris.chapuis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

.pragma library

var isTestEnvironment = true;

/* mako
var displayWidth = 768;
var displayHeight = 1280;
var gridUnit = 18;
var dpi = 264;
var tabletUi = false;
/**/
/* A500 alike
var displayWidth = 1024;
var displayHeight = 768;
var gridUnit = 18;
var dpi = 148;
var tabletUi = true;
/**/
/*GNex alike
var displayWidth = 720;
var displayHeight = 1280;
var gridUnit = 18;
var dpi = 264;
var tabletUi = false;
/**/
/* N7 alike
var displayWidth = 800;
var displayHeight = 1280;
var gridUnit = 14;
var dpi = 216;
var tabletUi = true;
/**/
/* tenderloin alike */
var displayWidth = 1024;
var displayHeight = 768 ;
var dpi = 132;
var gridUnit = 10;
var tabletUi = true;
/**/

var displayFps = true;
var fontStatusBar = "Prelude"
var showReticle = false;

// not used
var lunaSystemResourcesPath = "./resourcesPath";
var compatDpi = 114;
var splashIconSize = 64;
var gestureAreaHeight = 64;
var positiveSpaceTopPadding = 0;
var positiveSpaceBottomPadding = 0;

var layoutScale = dpi/132;
