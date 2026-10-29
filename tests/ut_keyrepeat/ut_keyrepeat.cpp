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

// The editor's own hold-to-repeat, and the ways a held key can end without a
// release. A backspace whose release never came used to repeat for good,
// deleting from whatever field had focus - on mindphone, every digit typed
// into the dialer, which has no keyboard on screen to stop it.

#include "abstracttexteditor.h"
#include "logic/abstractlanguagefeatures.h"
#include "models/text.h"

#include <QtTest>

using namespace MaliitKeyboard;

namespace {

class NoFeatures : public AbstractLanguageFeatures
{
public:
    bool activateAutoCaps(const QString &) const override { return false; }
    QString appendixForReplacedPreedit(const QString &) const override { return QString(); }
};

class NoWords : public Logic::AbstractWordEngine
{
public:
    AbstractLanguageFeatures *languageFeature() override { return &features; }

private:
    WordCandidateList fetchCandidates(Model::Text *) override { return WordCandidateList(); }
    NoFeatures features;
};

//! Counts the backspaces the editor sends to the application.
class CountingEditor : public AbstractTextEditor
{
public:
    CountingEditor()
        : AbstractTextEditor(EditorOptions(), new Model::Text, new NoWords)
    {}

    int backspaces = 0;

private:
    void sendPreeditString(const QString &, Model::Text::PreeditFace,
                           const Replacement &) override {}
    void sendCommitString(const QString &) override {}
    void sendKeyEvent(const QKeyEvent &ev) override
    {
        if (ev.type() == QEvent::KeyPress && ev.key() == Qt::Key_Backspace)
            ++backspaces;
    }
    void invokeAction(const QString &, const QKeySequence &) override {}
};

Key backspaceKey()
{
    Key key;
    key.setAction(Key::ActionBackspace);
    return key;
}

} // namespace

class TestKeyRepeat : public QObject
{
    Q_OBJECT

    // EditorOptions: the first repeat after 350ms, then one every 120ms.
    static constexpr int pastFirstRepeat = 700;
    static constexpr int quietPeriod = 600;

private Q_SLOTS:
    void tapDeletesOne()
    {
        CountingEditor editor;
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, 1);
    }

    void holdRepeatsUntilReleased()
    {
        CountingEditor editor;
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        QVERIFY2(editor.backspaces >= 2, qPrintable(QString::number(editor.backspaces)));

        editor.onKeyReleased(backspaceKey());
        const int atRelease = editor.backspaces;
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, atRelease);
    }

    // What happened on the device: held, and then no release ever.
    void cancelStopsARunningRepeat()
    {
        CountingEditor editor;
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        QVERIFY(editor.backspaces >= 2);

        editor.cancelRepeat();
        const int atCancel = editor.backspaces;
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, atCancel);
    }

    void cancelBeforeTheFirstRepeat()
    {
        CountingEditor editor;
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(100);
        editor.cancelRepeat();
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, 0);
    }

    // A cancelled press deletes nothing, even if its release turns up late.
    void lateReleaseAfterCancelDeletesNothing()
    {
        CountingEditor editor;
        editor.onKeyPressed(backspaceKey());
        editor.cancelRepeat();
        editor.onKeyReleased(backspaceKey());
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, 0);
    }

    // Nothing held: harmless, and the next tap still works.
    void cancelWithNothingHeld()
    {
        CountingEditor editor;
        editor.cancelRepeat();
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QTest::qWait(quietPeriod);
        QCOMPARE(editor.backspaces, 1);
    }
};

QTEST_GUILESS_MAIN(TestKeyRepeat)
#include "ut_keyrepeat.moc"
