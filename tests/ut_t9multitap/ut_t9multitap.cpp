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

#include "t9multitap.h"

#include <QtTest>

using MaliitKeyboard::T9Multitap;
using MaliitKeyboard::multitapWanted;
using MaliitKeyboard::predictionWanted;

class TestT9Multitap : public QObject
{
    Q_OBJECT

private:
    // What a run of presses of one key, all within the window, ends on.
    static QString typed(T9Multitap &t9, Qt::Key key, int presses)
    {
        T9Multitap::Step step;
        for (int i = 0; i < presses; ++i)
            step = t9.press(key, i > 0 && t9.cycling() && t9.key() == key);
        return QString(step.character);
    }

private Q_SLOTS:
    void cycles_data()
    {
        QTest::addColumn<int>("key");
        QTest::addColumn<QString>("cycle");
        QTest::newRow("1") << int(Qt::Key_1) << ".,?!1";
        QTest::newRow("2") << int(Qt::Key_2) << "abc2";
        QTest::newRow("7") << int(Qt::Key_7) << "pqrs7";
        QTest::newRow("9") << int(Qt::Key_9) << "wxyz9";
        QTest::newRow("0") << int(Qt::Key_0) << " 0";
        QTest::newRow("star") << int(Qt::Key_Asterisk) << "";
        QTest::newRow("letter") << int(Qt::Key_A) << "";
    }

    void cycles()
    {
        QFETCH(int, key);
        QFETCH(QString, cycle);
        QCOMPARE(T9Multitap::cycleFor(Qt::Key(key)), cycle);
    }

    // The same key within the window walks the cycle, each step in place of
    // the one before, and comes round again after the digit.
    void sameKeyWalksTheCycle()
    {
        T9Multitap t9;
        const QString cycle = QStringLiteral("pqrs7");
        for (int i = 0; i < cycle.length() + 2; ++i) {
            const T9Multitap::Step step = t9.press(Qt::Key_7, i > 0);
            QCOMPARE(step.character, cycle.at(i % cycle.length()));
            QCOMPARE(step.replacesPrevious, i > 0);
        }
    }

    // "secret": every letter on its own key or after the window closed.
    void typesAWord()
    {
        T9Multitap t9;
        QString word;
        word += typed(t9, Qt::Key_7, 4); t9.end();
        word += typed(t9, Qt::Key_3, 2); t9.end();
        word += typed(t9, Qt::Key_2, 3); t9.end();
        word += typed(t9, Qt::Key_7, 3); t9.end();
        word += typed(t9, Qt::Key_3, 2); t9.end();
        word += typed(t9, Qt::Key_8, 1);
        QCOMPARE(word, QStringLiteral("secret"));
    }

    // Another key starts a new character, whatever the window says.
    void anotherKeyStartsAnew()
    {
        T9Multitap t9;
        t9.press(Qt::Key_4, false);
        const T9Multitap::Step step = t9.press(Qt::Key_3, true);
        QCOMPARE(step.character, QChar('d'));
        QVERIFY(!step.replacesPrevious);
        QCOMPARE(t9.key(), Qt::Key_3);
    }

    // The same key after the window closed starts a new character: "aa".
    void sameKeyAfterTheWindowStartsAnew()
    {
        T9Multitap t9;
        t9.press(Qt::Key_2, false);
        const T9Multitap::Step step = t9.press(Qt::Key_2, false);
        QCOMPARE(step.character, QChar('a'));
        QVERIFY(!step.replacesPrevious);
    }

    // Ended (the timer, another key, focus going), the next press of the same
    // key is a new character even if asked to continue: nothing to continue.
    void endedCycleCannotContinue()
    {
        T9Multitap t9;
        t9.press(Qt::Key_5, false);
        t9.press(Qt::Key_5, true);
        t9.end();
        QVERIFY(!t9.cycling());
        const T9Multitap::Step step = t9.press(Qt::Key_5, true);
        QCOMPARE(step.character, QChar('j'));
        QVERIFY(!step.replacesPrevious);
    }

    // A key off the keypad gives nothing and leaves the cycle as it was.
    void offKeypadGivesNothing()
    {
        T9Multitap t9;
        t9.press(Qt::Key_6, false);
        const T9Multitap::Step step = t9.press(Qt::Key_Backspace, true);
        QVERIFY(step.character.isNull());
        QCOMPARE(t9.key(), Qt::Key_6);
        QCOMPARE(t9.index(), 0);
    }

    void prediction_data()
    {
        QTest::addColumn<bool>("asked");
        QTest::addColumn<bool>("askedValid");
        QTest::addColumn<bool>("hidden");
        QTest::addColumn<bool>("freeText");
        QTest::addColumn<bool>("wanted");
        QTest::newRow("free text") << true << true << false << true << true;
        QTest::newRow("free text, asked for none") << false << true << false << true << false;
        QTest::newRow("free text, nothing said") << false << false << false << true << true;
        QTest::newRow("password") << true << true << true << true << false;
        // The case that flapped: a number field asking for prediction (or
        // saying nothing) must be told no here as updateWordEngine() says no.
        QTest::newRow("number field asking") << true << true << false << false << false;
        QTest::newRow("number field, nothing said") << false << false << false << false << false;
    }

    void prediction()
    {
        QFETCH(bool, asked);
        QFETCH(bool, askedValid);
        QFETCH(bool, hidden);
        QFETCH(bool, freeText);
        QFETCH(bool, wanted);
        QCOMPARE(predictionWanted(asked, askedValid, hidden, freeText), wanted);
    }

    void multitap_data()
    {
        QTest::addColumn<bool>("letters");
        QTest::addColumn<bool>("custom");
        QTest::addColumn<bool>("hidden");
        QTest::addColumn<bool>("wanted");
        QTest::newRow("free text or email") << true << false << false << true;
        QTest::newRow("password in an app") << true << false << true << true;
        // Chromium's password field: purpose "password", custom to Maliit.
        QTest::newRow("password on a web page") << false << true << true << true;
        QTest::newRow("date, time, terminal") << false << true << false << false;
        QTest::newRow("number or phone") << false << false << false << false;
        QTest::newRow("PIN") << false << false << true << false;
    }

    void multitap()
    {
        QFETCH(bool, letters);
        QFETCH(bool, custom);
        QFETCH(bool, hidden);
        QFETCH(bool, wanted);
        QCOMPARE(multitapWanted(letters, custom, hidden), wanted);
    }
};

QTEST_GUILESS_MAIN(TestT9Multitap)

#include "ut_t9multitap.moc"
