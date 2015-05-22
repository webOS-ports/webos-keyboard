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

var currentTestEnv = "tenderloin";
var testEnvs = {
        mako: {
            displayWidth:768,
            displayHeight:1280,
            gridUnit:18,
            dpi:264,
            tabletUi:false
        },
        a500: {
            displayWidth:1280,
            displayHeight:800,
            gridUnit:12,
            dpi:148,
            tabletUi:true
        },
        gnexus: {
            displayWidth:720,
            displayHeight:1280,
            gridUnit:18,
            dpi:264,
            tabletUi:false
        },
        grouper: {
            displayWidth:1280,
            displayHeight:800,
            gridUnit:14,
            dpi:216,
            tabletUi:true
        },
        tenderloin: {
            displayWidth:1024,
            displayHeight:768,
            gridUnit:10,
            dpi:132,
            tabletUi:true
        }
    }

var displayWidth = testEnvs[currentTestEnv].displayWidth;
var displayHeight = testEnvs[currentTestEnv].displayHeight;
var dpi = testEnvs[currentTestEnv].dpi;
var gridUnit = testEnvs[currentTestEnv].gridUnit;
var tabletUi = testEnvs[currentTestEnv].tabletUi;

function changeCurrentTestEnv(newTestEnv, Units, FontUtils) {
    currentTestEnv = newTestEnv;

    displayWidth = testEnvs[currentTestEnv].displayWidth;
    displayHeight = testEnvs[currentTestEnv].displayHeight;
    dpi = testEnvs[currentTestEnv].dpi;
    gridUnit = testEnvs[currentTestEnv].gridUnit;
    tabletUi = testEnvs[currentTestEnv].tabletUi;


    Units.displayWidth = testEnvs[currentTestEnv].displayWidth;
    Units.displayHeight = testEnvs[currentTestEnv].displayHeight;
    Units.dpi = testEnvs[currentTestEnv].dpi;
    Units.gridUnit = testEnvs[currentTestEnv].gridUnit;
    Units.tabletUi = testEnvs[currentTestEnv].tabletUi;

    FontUtils.displayWidth = testEnvs[currentTestEnv].displayWidth;
    FontUtils.displayHeight = testEnvs[currentTestEnv].displayHeight;
    FontUtils.dpi = testEnvs[currentTestEnv].dpi;
    FontUtils.gridUnit = testEnvs[currentTestEnv].gridUnit;
    FontUtils.tabletUi = testEnvs[currentTestEnv].tabletUi;

}

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
