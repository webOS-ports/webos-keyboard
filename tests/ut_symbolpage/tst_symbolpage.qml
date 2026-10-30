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

/*
 * The symbols page and auto-capitalisation, driven through the real
 * Keyboard.qml with taps on the keys themselves.
 */

import QtQuick
import QtTest
import LunaNext.Common 0.1
import keys 1.0

Item {
    id: root
    width: 1024
    height: 768

    property Item keyboard: null
    property var typed: []

    // SettingsStub.testEnvs
    readonly property int phoneEnv: 0      // mako
    readonly property int tabletEnv: 5     // tenderloin

    Connections {
        target: maliit_event_handler
        function onKeyReleased() { root.typed.push(stubs.lastKey); }
    }

    function findItem(item, test) {
        if (!item || item.visible === false)
            return null;
        if (item.width > 0 && test(item))
            return item;
        for (var i = 0; i < item.children.length; ++i) {
            var found = findItem(item.children[i], test);
            if (found)
                return found;
        }
        return null;
    }

    TestCase {
        name: "SymbolPage"
        when: windowShown

        function load(env) {
            Settings.currentTestEnv = env;
            maliit_geometry.shown = true;
            var component = Qt.createComponent(keyboardQmlUrl);
            compare(component.status, Component.Ready, component.errorString());
            root.keyboard = component.createObject(root, { width: root.width, height: root.height });
            tryCompare(UI, "currentSymbolState", "CHARACTERS");
            // The panel slides in; the keys are where they will be tapped
            // once that has finished.
            waitForRendering(root.keyboard);
            wait(300);
            root.typed = [];
        }

        function cleanup() {
            if (root.keyboard)
                root.keyboard.destroy();
            root.keyboard = null;
            wait(0);
        }

        // Taps through the window rather than on the key: the symbol key
        // swaps the page on press, and the key it was is gone by the release.
        function tap(label) {
            var key = root.findItem(root.keyboard, function(item) {
                return item.label === label && item.pressArea !== undefined
                       || item.label === label && item.action !== undefined;
            });
            verify(key !== null, "no key '" + label + "' on the " + UI.currentSymbolState + " page");
            var p = key.mapToItem(root, key.width / 2, key.height / 2);
            mousePress(root, p.x, p.y);
            mouseRelease(root, p.x, p.y);
            wait(0);
        }

        function test_symbolsPageTypesDigits_data() {
            return [
                { tag: "phone", env: root.phoneEnv, symbolKey: "123" },
                { tag: "tablet", env: root.tabletEnv, symbolKey: "+=[]" },
            ];
        }
        function test_symbolsPageTypesDigits(data) {
            load(data.env);
            tap(data.symbolKey);
            compare(UI.currentSymbolState, "SYMBOLS");
            tap("1"); tap("2"); tap("3");
            compare(root.typed, ["1", "2", "3"]);
            tap("ABC");
            compare(UI.currentSymbolState, "CHARACTERS");
        }

        // Auto-capitalisation fired while the symbols page is up - a field
        // focused again before the panel finished going away, or a sentence
        // ended with "! " - goes back to the letters for the capital it is
        // for. It used to shift the symbols page instead, where nothing ever
        // releases shift: every key of the tablet's number row then typed its
        // shifted symbol, "123" coming out as "!@#", until shift was tapped.
        function test_autocapsOnSymbolsPage_data() {
            return [
                { tag: "phone", env: root.phoneEnv, symbolKey: "123" },
                { tag: "tablet", env: root.tabletEnv, symbolKey: "+=[]" },
            ];
        }
        function test_autocapsOnSymbolsPage(data) {
            load(data.env);
            tap(data.symbolKey);
            compare(UI.currentSymbolState, "SYMBOLS");

            maliit_input_method.activateAutocaps();

            compare(UI.currentSymbolState, "CHARACTERS");
            compare(UI.currentShiftState, "SHIFTED");
            tap("l");
            compare(UI.currentShiftState, "NORMAL");
            tap(data.symbolKey);
            tap("1"); tap("2"); tap("3");
            compare(root.typed, ["L", "1", "2", "3"]);
        }

        // What the Wi-Fi passphrase popup does: focus leaves the field and
        // comes back before the panel's hide animation has finished. The
        // HIDDEN state's reset to the letters never runs, so the symbols page
        // is still up when the refocused, empty field asks for a capital.
        function test_refocusWhileHiding() {
            load(root.tabletEnv);
            tap("+=[]");
            maliit_geometry.shown = false;
            wait(20);
            maliit_geometry.shown = true;
            wait(400);
            compare(UI.currentSymbolState, "SYMBOLS");

            maliit_input_method.activateAutocaps();
            tap("a");
            tap("1"); tap("2"); tap("3");
            compare(root.typed, ["A", "1", "2", "3"]);
        }
    }
}
