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

#include <QThread>
#include <QTimer>
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

//! Counts the times the word being typed is looked up, and corrects it to "hello".
class CountingWords : public Logic::AbstractWordEngine
{
public:
    CountingWords() { setEnabled(true); }
    AbstractLanguageFeatures *languageFeature() override { return &features; }
    int lookups = 0;

private:
    WordCandidateList fetchCandidates(Model::Text *text) override
    {
        ++lookups;
        text->setPrimaryCandidate(QStringLiteral("hello"));
        return WordCandidateList() << WordCandidate(WordCandidate::SourcePrediction, QStringLiteral("hello"));
    }
    NoFeatures features;
};

//! Counts the backspaces the editor sends to the application.
class CountingEditor : public AbstractTextEditor
{
public:
    explicit CountingEditor(Logic::AbstractWordEngine *words = new NoWords,
                            const EditorOptions &options = EditorOptions())
        : AbstractTextEditor(options, new Model::Text, words)
    {}

    int backspaces = 0;
    //! What each word repeat deleted before the cursor, in one edit.
    QList<int> deletes;
    int preeditsSent = 0;
    //! How long each word deletion keeps the editor busy, and when each came.
    int busyMs = 0;
    QElapsedTimer clock;
    QList<qint64> deletedAt;
    //! The application: answers each deletion this long after (-1: never).
    int replyMs = 0;
    QList<qint64> repliedAt;
    //! Deletions sent while one before was still unanswered.
    int overtaken = 0;
    bool unanswered = false;

private:
    void sendPreeditString(const QString &, Model::Text::PreeditFace,
                           const Replacement &) override { ++preeditsSent; }
    void sendCommitString(const QString &) override {}
    void deleteBeforeCursor(int n) override
    {
        deletes << n;
        deletedAt << (clock.isValid() ? clock.elapsed() : 0);
        if (unanswered)
            ++overtaken;
        unanswered = true;
        if (busyMs > 0)
            QThread::msleep(busyMs);
        if (replyMs >= 0) {
            QTimer::singleShot(replyMs, this, [this] {
                unanswered = false;
                repliedAt << (clock.isValid() ? clock.elapsed() : 0);
                surroundingTextUpdated();
            });
        }
    }
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

//! Whole words from the first word repeat on: 350ms in, then one every 50ms.
EditorOptions quickWords()
{
    EditorOptions options;
    options.backspace_word_delay = 0;
    options.backspace_word_interval = 50;
    return options;
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

    // A tap in a word being typed: the word is looked up again, as it always was.
    void tapLooksTheWordUp()
    {
        auto *words = new CountingWords;
        CountingEditor editor(words);
        editor.text()->setPreedit(QStringLiteral("helo"));
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(editor.text()->preedit(), QStringLiteral("hel"));
        QCOMPARE(words->lookups, 1);
    }

    // Held: every repeat would look the word up again, and on a phone the
    // deleting fell behind the finger. The word is looked up once, when it lets go.
    void holdLooksTheWordUpWhenLetGo()
    {
        auto *words = new CountingWords;
        CountingEditor editor(words);
        editor.text()->setPreedit(QStringLiteral("abcdefghijklmnop"));
        editor.text()->setPrimaryCandidate(QStringLiteral("hello"));
        int shown = 0;
        WordCandidateList last;
        connect(&editor, &AbstractTextEditor::wordCandidatesChanged, this,
                [&](const WordCandidateList &c) { ++shown; last = c; });
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        const int deleted = 16 - editor.text()->preedit().size();
        QVERIFY2(deleted >= 2, qPrintable(QString::number(deleted)));
        QCOMPARE(words->lookups, 0);
        // Its correction from before must not replace what is left of it.
        QVERIFY(editor.text()->primaryCandidate().isEmpty());
        // The candidates went: they were for a word no longer there.
        QVERIFY(shown > 0);
        QVERIFY(last.isEmpty());

        editor.onKeyReleased(backspaceKey());
        QCOMPARE(words->lookups, 1);
        QCOMPARE(editor.text()->primaryCandidate(), QStringLiteral("hello"));

        // And the next tap is looked up as ever.
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(words->lookups, 2);
    }

    // Cancelled mid-repeat (the field went away): the next key is looked up as ever.
    void cancelledHoldLooksUpAgainAfter()
    {
        auto *words = new CountingWords;
        CountingEditor editor(words);
        editor.text()->setPreedit(QStringLiteral("abcdefghijklmnop"));
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        editor.cancelRepeat();
        QCOMPARE(words->lookups, 0);
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(words->lookups, 1);
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

    // A word and the spaces after it, punctuation with the word, and no more
    // than 64 characters of a run with no space.
    void wordBeforeCursorLength_data()
    {
        QTest::addColumn<QString>("left");
        QTest::addColumn<int>("length");
        QTest::newRow("empty") << QString() << 0;
        QTest::newRow("word") << QStringLiteral("the lazy dog") << 3;
        QTest::newRow("word and space") << QStringLiteral("the lazy dog ") << 4;
        QTest::newRow("spaces") << QStringLiteral("the lazy dog   ") << 6;
        QTest::newRow("period") << QStringLiteral("a dog.") << 4;
        QTest::newRow("comma and space") << QStringLiteral("Hello, ") << 7;
        QTest::newRow("line") << QStringLiteral("one\ntwo\n") << 4;
        QTest::newRow("only spaces") << QStringLiteral("   ") << 3;
        QTest::newRow("long") << QString(100, QLatin1Char('x')) << 64;
    }
    void wordBeforeCursorLength()
    {
        QFETCH(QString, left);
        QFETCH(int, length);
        QCOMPARE(AbstractTextEditor::wordBeforeCursorLength(left), length);
    }

    // Held past the word delay: each repeat takes a word and its space in one
    // edit, going by what is left, not the text as it was.
    void wordRepeatTakesAWordAnEdit()
    {
        CountingEditor editor(new NoWords, quickWords());
        editor.text()->setSurrounding(QStringLiteral("the lazy dog and"));
        editor.text()->setSurroundingOffset(16);
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(editor.deletes.size() >= 3, 2000);
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(editor.deletes.mid(0, 3), (QList<int>{3, 4, 5}));
        QCOMPARE(editor.text()->surroundingLeft(), QStringLiteral("the "));
        QCOMPARE(editor.backspaces, 0);
    }

    // The word being typed goes in one, as an empty preedit.
    void wordRepeatDropsTheWordBeingTyped()
    {
        CountingEditor editor(new NoWords, quickWords());
        editor.text()->setSurrounding(QStringLiteral("the "));
        editor.text()->setSurroundingOffset(4);
        editor.text()->setPreedit(QStringLiteral("laz"));
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(editor.text()->preedit().isEmpty(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(!editor.deletes.isEmpty(), 2000);
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(editor.deletes.first(), 4);
        QCOMPARE(editor.backspaces, 0);
    }

    // The field says nothing of its text: a backspace a repeat, as before.
    void wordRepeatWithoutSurroundingSendsBackspaces()
    {
        CountingEditor editor(new NoWords, quickWords());
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(editor.backspaces >= 2, 2000);
        editor.onKeyReleased(backspaceKey());
        QVERIFY(editor.deletes.isEmpty());
    }

    // Each word an interval after the application took the last one in:
    // however long it takes (here 30 ms busy, or a reply 120 ms after), the
    // gaps are the same every time and nothing goes before the last is in.
    void wordRepeatGoesEvenly_data()
    {
        QTest::addColumn<int>("busyMs");
        QTest::addColumn<int>("replyMs");
        QTest::newRow("quick") << 0 << 0;
        QTest::newRow("busy") << 30 << 0;
        QTest::newRow("slow application") << 0 << 120;
    }
    void wordRepeatGoesEvenly()
    {
        QFETCH(int, busyMs);
        QFETCH(int, replyMs);
        CountingEditor editor(new NoWords, quickWords());
        editor.text()->setSurrounding(QStringLiteral("a b c d e f g h i j k l"));
        editor.text()->setSurroundingOffset(23);
        editor.busyMs = busyMs;
        editor.replyMs = replyMs;
        editor.clock.start();
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(editor.deletedAt.size() >= 7, 4000);
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(editor.overtaken, 0);
        const auto &at = editor.deletedAt;
        qint64 least = 1 << 30, most = 0;
        for (int i = 2; i < 7; ++i) {
            least = qMin(least, at[i] - at[i - 1]);
            most = qMax(most, at[i] - at[i - 1]);
        }
        const qint64 expected = busyMs + replyMs + 50;
        QVERIFY2(least >= expected - 5 && most <= expected + 25,
                 qPrintable(QStringLiteral("gaps %1..%2 ms, expected %3").arg(least).arg(most).arg(expected)));
    }

    // A reply after the key is let go starts nothing.
    void replyAfterReleaseDeletesNothing()
    {
        CountingEditor editor(new NoWords, quickWords());
        editor.text()->setSurrounding(QStringLiteral("a b c d e f"));
        editor.text()->setSurroundingOffset(11);
        editor.replyMs = 100;
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(!editor.deletes.isEmpty(), 2000);
        editor.onKeyReleased(backspaceKey());
        const int atRelease = editor.deletes.size();
        QTest::qWait(400);
        QCOMPARE(editor.deletes.size(), atRelease);
    }

    // An application that never says: a deletion every kReplyWaitMs (600 ms).
    void wordRepeatGoesOnWithoutAnAnswer()
    {
        CountingEditor editor(new NoWords, quickWords());
        editor.text()->setSurrounding(QStringLiteral("a b c d e f"));
        editor.text()->setSurroundingOffset(11);
        editor.replyMs = -1;
        editor.clock.start();
        editor.onKeyPressed(backspaceKey());
        QTRY_VERIFY_WITH_TIMEOUT(editor.deletedAt.size() >= 3, 3000);
        editor.onKeyReleased(backspaceKey());
        const qint64 gap = editor.deletedAt[2] - editor.deletedAt[1];
        QVERIFY2(gap >= 590 && gap < 700, qPrintable(QString::number(gap)));
    }

    // Repeating, then let go or cancelled: said once each way.
    void deletingChangedOnRepeatAndRelease()
    {
        CountingEditor editor;
        QList<bool> changes;
        connect(&editor, &AbstractTextEditor::deletingChanged, this,
                [&](bool deleting) { changes << deleting; });
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        QVERIFY(editor.isDeleting());
        editor.onKeyReleased(backspaceKey());
        QVERIFY(!editor.isDeleting());
        editor.onKeyPressed(backspaceKey());
        QTest::qWait(pastFirstRepeat);
        editor.cancelRepeat();
        QCOMPARE(changes, (QList<bool>{true, false, true, false}));

        // A tap is not a repeat.
        editor.onKeyPressed(backspaceKey());
        editor.onKeyReleased(backspaceKey());
        QCOMPARE(changes.size(), 4);
    }
};

QTEST_GUILESS_MAIN(TestKeyRepeat)
#include "ut_keyrepeat.moc"
