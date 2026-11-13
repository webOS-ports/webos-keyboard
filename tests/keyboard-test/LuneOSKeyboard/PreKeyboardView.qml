/*
 * This file is part of the LuneOS keyboard
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 */

import QtQuick 2.0

/*
 * A stand-in for the Pre Keyboard's view (src/pre/prekeyboardview.h, which
 * the plugin registers as LuneOSKeyboard.PreKeyboardView): the properties
 * Keyboard.qml reads and sets, and nothing drawn. The view is C++ and tested
 * in ut_prekeyboard.
 */
Item {
    property QtObject controller: null
    property real dp: 1
    property bool landscape: false
    readonly property real keysHeight: 0
    readonly property real headroom: 0
    readonly property rect popupRect: Qt.rect(0, 0, 0, 0)

    //! A key went down and wants a click.
    signal keyClick()
}
