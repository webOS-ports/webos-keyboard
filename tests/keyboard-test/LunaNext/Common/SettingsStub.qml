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

pragma Singleton

import QtQuick 2.0
import "StorageStub.js" as Storage

QtObject {
    property bool isTestEnvironment: true;

    property int currentTestEnv: 4
    property ListModel testEnvs: ListModel {
        ListElement {
            name: "mako"

            displayWidth:768
            displayHeight:1280
            gridUnit:18
            dpi:264
            tabletUi:false
        }
        ListElement {
            name: "a500"
            displayWidth:1280
            displayHeight:800
            gridUnit:12
            dpi:148
            tabletUi:true
        }
        ListElement {
            name: "gnexus"
            displayWidth:720
            displayHeight:1280
            gridUnit:18
            dpi:264
            tabletUi:false
        }
        ListElement {
            name: "grouper"
            displayWidth:1280
            displayHeight:800
            gridUnit:14
            dpi:216
            tabletUi:true
        }
        ListElement {
            name: "tenderloin"
            displayWidth:1024
            displayHeight:768
            gridUnit:10
            dpi:132
            tabletUi:true
        }
    }

    property string currentTestEnvName: testEnvs.get(currentTestEnv).name;

    property int displayWidth: testEnvs.get(currentTestEnv).displayWidth;
    property int displayHeight: testEnvs.get(currentTestEnv).displayHeight;
    property int dpi: testEnvs.get(currentTestEnv).dpi;
    property int gridUnit: testEnvs.get(currentTestEnv).gridUnit;
    property int tabletUi: testEnvs.get(currentTestEnv).tabletUi;

    // keep these properties in sync, to that the Units and FontUtils stubs return correct values
    onGridUnitChanged: Storage.gridUnit = gridUnit;
    onLayoutScaleChanged: Storage.layoutScale = layoutScale;

    property bool displayFps: true;
    property string fontStatusBar: "Prelude";
    property bool showReticle: false;

    // not used
    property string lunaSystemResourcesPath: "./resourcesPath";
    property int splashIconSize: 64;
    property int gestureAreaHeight: 64;
    property int positiveSpaceTopPadding: 0;
    property int positiveSpaceBottomPadding: 0;
    property int layoutScale: dpi/132;
}
