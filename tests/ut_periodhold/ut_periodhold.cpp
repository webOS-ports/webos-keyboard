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

// A physical keyboard's period: tapped a period, held it dictates (PeriodHold).

#include "periodhold.h"

#include <QSignalSpy>
#include <QtTest>

using namespace MaliitKeyboard;

namespace {

// Scancodes as the framework hands them on: evdev's plus 8.
const quint32 DOT = 52 + 8;     // KEY_DOT
const quint32 V = 47 + 8;       // the Titan's period, at Alt
const quint32 SPACE = 57 + 8;
const quint32 BACKSPACE = 14 + 8;

const int HOLD = 60;

} // namespace

class ut_periodhold : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void aTapIsAPeriod();
    void heldItListens();
    void theKernelsRepeatsAreNotPeriods();
    void anotherKeyBeforeItsReleaseKeepsTheOrder();
    void anotherKeyWhileListening();
    void wherePeriodsCannotDictate();
    void otherKeys();
    void aSwallowedKeystroke();
    void anotherField();
    void noScanCode();
    void heldAsLongAsTheScreensPeriod();
};

void ut_periodhold::aTapIsAPeriod()
{
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QVERIFY(h.pending());
    QCOMPARE(h.release(DOT), PeriodHold::TypePeriod);
    QVERIFY(!h.pending());

    QTest::qWait(HOLD * 2);
    QCOMPARE(held.count(), 0);
}

void ut_periodhold::heldItListens()
{
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(V, true, true), PeriodHold::Swallow);
    QTRY_COMPARE(held.count(), 1);
    QVERIFY(!h.pending());
    // Nothing of it is typed: not on its release either.
    QCOMPARE(h.release(V), PeriodHold::Swallow);
    QTest::qWait(HOLD * 2);
    QCOMPARE(held.count(), 1);

    // And the next tap is a period again.
    QCOMPARE(h.press(V, true, true), PeriodHold::Swallow);
    QCOMPARE(h.release(V), PeriodHold::TypePeriod);
}

void ut_periodhold::theKernelsRepeatsAreNotPeriods()
{
    // A keyboard that repeats (the Q25's) sends press after press while held.
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QTRY_COMPARE(held.count(), 1);
    QCOMPARE(h.press(DOT, true, false), PeriodHold::Swallow);
    QCOMPARE(h.press(DOT, true, false), PeriodHold::Swallow);
    QCOMPARE(h.release(DOT), PeriodHold::Swallow);
    QCOMPARE(held.count(), 1);
}

void ut_periodhold::anotherKeyBeforeItsReleaseKeepsTheOrder()
{
    // "." then space, typed fast enough that space goes down first.
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QCOMPARE(h.press(SPACE, false, false), PeriodHold::TypePeriodFirst);
    // The period went in with the space; its release adds nothing.
    QCOMPARE(h.release(DOT), PeriodHold::Swallow);
    QCOMPARE(h.release(SPACE), PeriodHold::Pass);

    QTest::qWait(HOLD * 2);
    QCOMPARE(held.count(), 0);
    // Nothing is left over to swallow.
    QCOMPARE(h.press(DOT, true, false), PeriodHold::Pass);
    QCOMPARE(h.release(DOT), PeriodHold::Pass);
}

void ut_periodhold::anotherKeyWhileListening()
{
    // Held, it listens; the key that stops that is for the caller.
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QTRY_COMPARE(held.count(), 1);
    QCOMPARE(h.press(SPACE, false, false), PeriodHold::Pass);
    QCOMPARE(h.release(SPACE), PeriodHold::Pass);
    QCOMPARE(h.release(DOT), PeriodHold::Swallow);
}

void ut_periodhold::wherePeriodsCannotDictate()
{
    // No model, a number field, a password: a period at once, as ever.
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, false), PeriodHold::Pass);
    QVERIFY(!h.pending());
    QTest::qWait(HOLD * 2);
    QCOMPARE(h.release(DOT), PeriodHold::Pass);
    QCOMPARE(held.count(), 0);
}

void ut_periodhold::otherKeys()
{
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    // Not a period, wherever dictation could start.
    QCOMPARE(h.press(SPACE, false, true), PeriodHold::Pass);
    QVERIFY(!h.pending());
    QTest::qWait(HOLD * 2);
    QCOMPARE(h.release(SPACE), PeriodHold::Pass);
    QCOMPARE(held.count(), 0);
}

void ut_periodhold::aSwallowedKeystroke()
{
    // Backspace while listening takes back what was being said, and nothing else.
    PeriodHold h;
    h.swallow(BACKSPACE);
    QCOMPARE(h.press(BACKSPACE, false, false), PeriodHold::Swallow);   // the kernel repeating it
    QCOMPARE(h.press(SPACE, false, false), PeriodHold::Pass);
    QCOMPARE(h.release(SPACE), PeriodHold::Pass);
    QCOMPARE(h.release(BACKSPACE), PeriodHold::Swallow);
    // Only up to its release.
    QCOMPARE(h.press(BACKSPACE, false, false), PeriodHold::Pass);
    QCOMPARE(h.release(BACKSPACE), PeriodHold::Pass);
}

void ut_periodhold::anotherField()
{
    PeriodHold h;
    h.setHoldMs(HOLD);
    QSignalSpy held(&h, &PeriodHold::held);

    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    h.swallow(BACKSPACE);
    h.reset();
    QVERIFY(!h.pending());
    QTest::qWait(HOLD * 2);
    QCOMPARE(held.count(), 0);
    QCOMPARE(h.release(DOT), PeriodHold::Pass);
    QCOMPARE(h.press(BACKSPACE, false, false), PeriodHold::Pass);
}

void ut_periodhold::noScanCode()
{
    // A key with no scancode cannot be told apart from the next; it is never kept back.
    PeriodHold h;
    h.setHoldMs(HOLD);
    QCOMPARE(h.press(0, true, true), PeriodHold::Pass);
    QVERIFY(!h.pending());
    h.swallow(0);
    QCOMPARE(h.press(0, false, false), PeriodHold::Pass);
    QCOMPARE(h.release(0), PeriodHold::Pass);
}

void ut_periodhold::heldAsLongAsTheScreensPeriod()
{
    PeriodHold h;
    QSignalSpy held(&h, &PeriodHold::held);
    QElapsedTimer t;
    t.start();
    QCOMPARE(h.press(DOT, true, true), PeriodHold::Swallow);
    QTRY_COMPARE_WITH_TIMEOUT(held.count(), 1, PeriodHold::HOLD_MS * 4);
    QVERIFY(t.elapsed() >= PeriodHold::HOLD_MS - 5);
    QCOMPARE(PeriodHold::HOLD_MS, 450);
}

QTEST_GUILESS_MAIN(ut_periodhold)
#include "ut_periodhold.moc"
