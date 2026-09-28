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

#include "panelstate.h"

#include <QtTest>

using namespace MaliitKeyboard;

namespace {

//! A field has focus, a physical keyboard is attached, suggestions are on. The
//! state a Q25 is in while its user types, and the one every regression here has
//! been reachable from.
PanelState typingOnHardware()
{
    PanelState s;
    s.setWordEngine(true);
    s.setHardware(true);
    s.setFocused(true);
    return s;
}

} // namespace

class ut_panelstate : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNothingWantedWithoutFocus();
    void testOnScreenKeyboardShowsForAField();
    void testHardwareKeyboardLeavesTheCandidateBar();
    void testHardwareKeyboardWithNoSuggestionsShowsNothing();
    void testDragKeepsTheCandidateBar();
    void testDragTakesThePanelWhenThereIsNoBar();
    void testExpandBringsTheKeysBack();
    void testFocusClearsADismissal();
    void testFocusClearsACollapse();
    void testSwitchingInputSourceClearsADismissal();
    void testSameInputSourceIsNotAFreshIntent();
    void testDismissalSurvivesLosingFocus();
    void testWordEngineArrivingRevealsTheBar();
};

void ut_panelstate::testNothingWantedWithoutFocus()
{
    PanelState s;
    QVERIFY(!s.panelWanted());

    s.setWordEngine(true);
    QVERIFY2(!s.panelWanted(), "suggestions with no field to type in are not a panel");
}

void ut_panelstate::testOnScreenKeyboardShowsForAField()
{
    PanelState s;
    s.setFocused(true);
    QVERIFY(s.panelWanted());
    QVERIFY(!s.keysHidden());
}

void ut_panelstate::testHardwareKeyboardLeavesTheCandidateBar()
{
    const PanelState s = typingOnHardware();

    QVERIFY2(s.keysHidden(), "keys are not drawn for a keyboard that has keys");
    QVERIFY2(s.panelWanted(), "the candidate bar is worth keeping on a hardware keyboard");
}

void ut_panelstate::testHardwareKeyboardWithNoSuggestionsShowsNothing()
{
    PanelState s;
    s.setHardware(true);
    s.setFocused(true);

    QVERIFY(s.keysHidden());
    QVERIFY2(!s.panelWanted(), "no keys and no bar is no panel");
}

void ut_panelstate::testDragKeepsTheCandidateBar()
{
    PanelState s;
    s.setWordEngine(true);
    s.setFocused(true);

    s.collapseKeys();

    QVERIFY(s.keysCollapsed());
    QVERIFY2(!s.dismissed(), "the whole panel was not asked for");
    QVERIFY(s.keysHidden());
    QVERIFY2(s.panelWanted(), "the bar stays when the keys are dragged away");
}

void ut_panelstate::testDragTakesThePanelWhenThereIsNoBar()
{
    PanelState s;
    s.setFocused(true);

    s.collapseKeys();

    QVERIFY2(s.dismissed(), "with no suggestions there is nothing to be left with");
    QVERIFY(!s.panelWanted());
}

void ut_panelstate::testExpandBringsTheKeysBack()
{
    PanelState s;
    s.setWordEngine(true);
    s.setFocused(true);
    s.collapseKeys();

    s.expandKeys();

    QVERIFY(!s.keysCollapsed());
    QVERIFY(!s.keysHidden());
    QVERIFY(s.panelWanted());
}

void ut_panelstate::testFocusClearsADismissal()
{
    PanelState s;
    s.setFocused(true);
    s.dismiss();
    QVERIFY(!s.panelWanted());

    s.setFocused(false);
    s.setFocused(true);

    QVERIFY2(s.panelWanted(), "tapping another field is a fresh request for input");
}

void ut_panelstate::testFocusClearsACollapse()
{
    PanelState s;
    s.setWordEngine(true);
    s.setFocused(true);
    s.collapseKeys();

    s.setFocused(false);
    s.setFocused(true);

    QVERIFY(!s.keysCollapsed());
    QVERIFY(!s.keysHidden());
}

void ut_panelstate::testSwitchingInputSourceClearsADismissal()
{
    // The bug this is here for: drag the keyboard away, then use the shell's
    // toggle, and nothing happened at all - the dismissal still said the panel
    // was not wanted.
    PanelState s = typingOnHardware();
    s.dismiss();
    QVERIFY(!s.panelWanted());

    s.setHardware(false);

    QVERIFY(!s.dismissed());
    QVERIFY(!s.keysHidden());
    QVERIFY(s.panelWanted());
}

void ut_panelstate::testSameInputSourceIsNotAFreshIntent()
{
    PanelState s = typingOnHardware();
    s.collapseKeys();

    s.setHardware(true); // the framework re-asserting what is already true

    QVERIFY2(s.keysCollapsed(), "a state that did not change is not a new request");
}

void ut_panelstate::testDismissalSurvivesLosingFocus()
{
    // Losing focus must not quietly clear the flags: the panel is gone anyway
    // while nothing is focused, and clearing them here would make the state
    // depend on whether focus happened to bounce.
    PanelState s;
    s.setWordEngine(true);
    s.setFocused(true);
    s.dismiss();

    s.setFocused(false);

    QVERIFY(s.dismissed());
}

void ut_panelstate::testWordEngineArrivingRevealsTheBar()
{
    // update() settles predictionEnabled per field, and it arrives after the
    // input source is known. A hardware keyboard with the engine off shows
    // nothing; the same field with it on shows the bar.
    PanelState s;
    s.setHardware(true);
    s.setFocused(true);
    QVERIFY(!s.panelWanted());

    s.setWordEngine(true);

    QVERIFY(s.panelWanted());
}

QTEST_GUILESS_MAIN(ut_panelstate)

#include "ut_panelstate.moc"
