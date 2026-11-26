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

#include "keypairing.h"

#include <QtTest>

using MaliitKeyboard::KeyPairing;

class ut_keypairing : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    //! A tap: down and up here.
    void pairedRelease()
    {
        KeyPairing k;
        k.press(24);
        QVERIFY(k.release(24));
        // Once: a second release has no press left.
        QVERIFY(!k.release(24));
    }

    //! The press went to the application (no field had focus); its release is not ours.
    void releaseWithoutPress()
    {
        KeyPairing k;
        QVERIFY(!k.release(24));
        // Shift held across it, pressed here, still pairs.
        k.press(50);
        QVERIFY(!k.release(17));
        QVERIFY(k.release(50));
    }

    //! A kernel repeating a held key sends its press again; one release ends it.
    void kernelRepeat()
    {
        KeyPairing k;
        k.press(22);
        k.press(22);
        k.press(22);
        QVERIFY(k.release(22));
        QVERIFY(!k.release(22));
    }

    //! A key with no scancode (made up by software) cannot be paired, so it is let through.
    void noScanCode()
    {
        KeyPairing k;
        QVERIFY(k.release(0));
        k.press(0);
        QVERIFY(k.release(0));
    }
};

QTEST_GUILESS_MAIN(ut_keypairing)
#include "ut_keypairing.moc"
