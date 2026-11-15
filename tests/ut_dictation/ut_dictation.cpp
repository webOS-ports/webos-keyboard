/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Dictation: the helpers that find a model and put the words in the text, and
// VoskDictation listening, with libvosk itself, to a recording instead of the
// microphone.
//
// The tests that recognise speech need libvosk.so.0 where the loader finds it
// (LD_LIBRARY_PATH) and VOSK_TEST_DIR naming a folder with test.wav (16 kHz,
// mono, 16-bit: Vosk's own python/example/test.wav) and models/en-US (a
// small English model). Without them those tests are skipped.

#include "predictation.h"
#include "prevosk.h"

#include <QElapsedTimer>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <atomic>
#include <chrono>
#include <cstring>
#include <thread>

using Pre::AudioSource;
using Pre::VoskDictation;

namespace {

/*!
 * A recording as the microphone: [pcm], then [silenceMs] of quiet, then the
 * end, or a failure ([fail]), or quiet for ever ([endless]). [paceUs] waits
 * that long per read, as a microphone would; [opened]/[reads] say what was asked of it.
 */
struct Script
{
    QByteArray pcm;
    //! When set, the recordings started from then on are quiet instead of [pcm].
    std::atomic<bool> hush { false };
    int silenceMs = 0;
    bool endless = false;
    bool fail = false;
    bool open = true;
    QString openProblem;
    int paceUs = 0;
    std::atomic<int> opened { 0 };
    std::atomic<int> reads { 0 };
};

class Recording : public AudioSource
{
public:
    explicit Recording(std::shared_ptr<Script> s) : m_s(std::move(s)), m_quietOnly(m_s->hush) {}

    bool open(QString *problem) override
    {
        ++m_s->opened;
        if (!m_s->open && problem) *problem = m_s->openProblem;
        return m_s->open;
    }

    int read(char *buffer, int bytes) override
    {
        ++m_s->reads;
        if (m_s->paceUs) std::this_thread::sleep_for(std::chrono::microseconds(m_s->paceUs));
        if (!m_quietOnly && m_at < m_s->pcm.size()) {
            const int n = int(std::min<qsizetype>(bytes, m_s->pcm.size() - m_at));
            std::memcpy(buffer, m_s->pcm.constData() + m_at, size_t(n));
            m_at += n;
            return n;
        }
        if (m_s->fail) return -1;
        const qint64 quiet = qint64(m_s->silenceMs) * 32;
        if (!m_s->endless && m_quiet >= quiet) return 0;
        const int n = m_s->endless ? bytes : int(std::min<qint64>(bytes, quiet - m_quiet));
        std::memset(buffer, 0, size_t(n));
        m_quiet += n;
        return n;
    }

private:
    std::shared_ptr<Script> m_s;
    qsizetype m_at = 0;
    qint64 m_quiet = 0;
    bool m_quietOnly = false;
};

VoskDictation::AudioFactory from(const std::shared_ptr<Script> &s)
{
    return [s] { return std::make_unique<Recording>(s); };
}

//! A model's folder that is one only to modelFor(): an am/final.mdl of [content].
void fakeModel(const QString &dir, const QByteArray &content = QByteArray("x"))
{
    QVERIFY(QDir().mkpath(dir + QStringLiteral("/am")));
    QFile f(dir + QStringLiteral("/am/final.mdl"));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(content);
}

//! Everything a dictation said, in order: "partial: ...", "final: ...", "stopped: ...".
struct Heard
{
    QStringList all;
    QStringList finals;
    int stops = 0;
    QString problem;

    void listen(Pre::Dictation &d)
    {
        QObject::connect(&d, &Pre::Dictation::partial, &d, [this](const QString &t) { all << QStringLiteral("partial: ") + t; });
        QObject::connect(&d, &Pre::Dictation::final, &d, [this](const QString &t) {
            all << QStringLiteral("final: ") + t;
            finals << t;
        });
        QObject::connect(&d, &Pre::Dictation::stopped, &d, [this](const QString &p) {
            all << QStringLiteral("stopped: ") + p;
            ++stops;
            problem = p;
        });
    }
};

const QStringList SPOKEN = { QStringLiteral("one zero zero zero one"), QStringLiteral("nah no to i know"),
                             QStringLiteral("zero one eight zero three") };

} // namespace

class TestDictation : public QObject
{
    Q_OBJECT

    QString m_models;
    QByteArray m_speech;
    bool m_haveVosk = false;

    //! libvosk, the model and the recording are there.
    bool needVosk() const { return m_haveVosk; }

private Q_SLOTS:
    void initTestCase()
    {
        const QString dir = qEnvironmentVariable("VOSK_TEST_DIR");
        QFile wav(dir + QStringLiteral("/test.wav"));
        m_models = dir + QStringLiteral("/models");
        QLibrary vosk;
        vosk.setFileNameAndVersion(QStringLiteral("vosk"), 0);
        if (!dir.isEmpty() && wav.open(QIODevice::ReadOnly) && QFileInfo::exists(m_models + QStringLiteral("/en-US/am/final.mdl"))
            && vosk.load()) {
            const QByteArray all = wav.readAll();
            const int data = int(all.indexOf("data"));
            QVERIFY(data > 0);
            m_speech = all.mid(data + 8);
            m_haveVosk = true;
            vosk.unload();
        }
    }

    // --- Speech: the helpers -------------------------------------------------------------------

    /*
     * The microphone is named, never left to the default: on LuneOS
     * module-palm-policy puts an unnamed recording on the "record" virtual
     * source, silent until audiod binds it, and the mako refused it outright.
     * Its sources as the mako's PulseAudio lists them.
     */
    void recordingSource()
    {
        const QList<QPair<QString, QString>> mako = {
            { QStringLiteral("sink.primary_output.monitor"), QStringLiteral("monitor") },
            { QStringLiteral("source.droid"), QStringLiteral("sound") },
            { QStringLiteral("pcm_output.monitor"), QStringLiteral("monitor") },
            { QStringLiteral("record"), QStringLiteral("abstract") },
            { QStringLiteral("precord"), QStringLiteral("abstract") },
        };
        QCOMPARE(Pre::Speech::recordingSource(QStringLiteral("source.droid"), mako), QStringLiteral("source.droid"));
        // The default set to something that isn't a microphone, or not known yet: still the microphone.
        QCOMPARE(Pre::Speech::recordingSource(QStringLiteral("pcm_output.monitor"), mako), QStringLiteral("source.droid"));
        QCOMPARE(Pre::Speech::recordingSource(QString(), mako), QStringLiteral("source.droid"));
        // Another phone's name for it, and the default where there are two.
        const QList<QPair<QString, QString>> two = {
            { QStringLiteral("source.primary_input"), QStringLiteral("sound") },
            { QStringLiteral("source.usb_headset"), QStringLiteral("sound") },
        };
        QCOMPARE(Pre::Speech::recordingSource(QStringLiteral("source.usb_headset"), two), QStringLiteral("source.usb_headset"));
        QCOMPARE(Pre::Speech::recordingSource(QStringLiteral("record"), two), QStringLiteral("source.primary_input"));
        // Only virtual ones: none named.
        QVERIFY(Pre::Speech::recordingSource(QStringLiteral("record"), mako.mid(3)).isEmpty());
    }

    void modelFor_data()
    {
        QTest::addColumn<QString>("language");
        QTest::addColumn<QString>("model");
        QTest::newRow("the language's own") << "de" << "de";
        QTest::newRow("its first, by name") << "en" << "en-GB";
        QTest::newRow("a tag, as a tag") << "en_us" << "en-US";
        QTest::newRow("a region that has none") << "en-AU" << "en-GB";
        QTest::newRow("a folder that isn't a model") << "nl" << "";
        QTest::newRow("a region only") << "pt" << "pt-BR";
        QTest::newRow("none") << "fr" << "";
        QTest::newRow("no language") << "" << "";
        QTest::newRow("a file, not a folder") << "it" << "";
    }

    /*
     * A keyboard language finds the model named for it, else one of its
     * language; a folder without an acoustic model, or a file, is not one.
     */
    void modelFor()
    {
        QFETCH(QString, language);
        QFETCH(QString, model);
        QTemporaryDir root;
        fakeModel(root.filePath(QStringLiteral("de")));
        fakeModel(root.filePath(QStringLiteral("en-US")));
        fakeModel(root.filePath(QStringLiteral("en-GB")));
        fakeModel(root.filePath(QStringLiteral("pt-BR")));
        QVERIFY(QDir().mkpath(root.filePath(QStringLiteral("nl/conf"))));
        QFile it(root.filePath(QStringLiteral("it")));
        QVERIFY(it.open(QIODevice::WriteOnly));
        it.close();
        QCOMPARE(Pre::Speech::modelFor(root.path(), language),
                 model.isEmpty() ? QString() : root.filePath(model));
    }

    void modelForNoFolder()
    {
        QCOMPARE(Pre::Speech::modelFor(QStringLiteral("/nonexistent/vosk"), QStringLiteral("en")), QString());
    }

    void resultText_data()
    {
        QTest::addColumn<QByteArray>("json");
        QTest::addColumn<QString>("key");
        QTest::addColumn<QString>("text");
        QTest::newRow("text") << QByteArray("{\n  \"text\" : \"hello there\"\n}") << "text" << "hello there";
        QTest::newRow("partial") << QByteArray("{\"partial\" : \"hel\"}") << "partial" << "hel";
        QTest::newRow("empty") << QByteArray("{\"text\" : \"\"}") << "text" << "";
        QTest::newRow("spaces") << QByteArray("{\"text\" : \"  a   b \"}") << "text" << "a b";
        QTest::newRow("another key") << QByteArray("{\"partial\" : \"x\"}") << "text" << "";
        QTest::newRow("not a string") << QByteArray("{\"text\" : 3}") << "text" << "";
        QTest::newRow("an array") << QByteArray("[\"text\"]") << "text" << "";
        QTest::newRow("not JSON") << QByteArray("text: hello") << "text" << "";
        QTest::newRow("nothing") << QByteArray() << "text" << "";
        QTest::newRow("UTF-8") << QByteArray("{\"text\" : \"gr\xc3\xbc\xc3\x9f dich\"}") << "text" << QStringLiteral("grüß dich");
    }

    void resultText()
    {
        QFETCH(QByteArray, json);
        QFETCH(QString, key);
        QFETCH(QString, text);
        QCOMPARE(Pre::Speech::resultText(json, key.toLatin1().constData()), text);
    }

    void join_data()
    {
        QTest::addColumn<QString>("spoken");
        QTest::addColumn<QString>("before");
        QTest::addColumn<bool>("capital");
        QTest::addColumn<QString>("language");
        QTest::addColumn<QString>("text");
        QTest::newRow("first in the field") << "hello there" << "" << true << "en" << "Hello there";
        QTest::newRow("after a word") << "hello" << "say" << false << "en" << " hello";
        QTest::newRow("after a space") << "hello" << "say " << false << "en" << "hello";
        QTest::newRow("after a new line") << "hello" << "\n" << true << "en" << "Hello";
        QTest::newRow("after a period") << "next" << "done." << true << "en" << " Next";
        QTest::newRow("after a bracket") << "aside" << "(" << false << "en" << "aside";
        QTest::newRow("after a quote") << "quoted" << "“" << false << "en" << "quoted";
        QTest::newRow("after ¿") << "qué" << "¿" << false << "es" << "qué";
        QTest::newRow("English I") << "i think i know" << "" << false << "en" << "I think I know";
        QTest::newRow("English I'm") << "i'm sure i'll go" << "" << false << "en" << "I'm sure I'll go";
        QTest::newRow("English I’m") << "i’m here" << "" << false << "en" << "I’m here";
        QTest::newRow("English: not in words") << "in it is hi" << "" << false << "en" << "in it is hi";
        QTest::newRow("English: a letter i") << "the letter i" << "" << false << "en" << "the letter I";
        QTest::newRow("English, by tag") << "i do" << "" << false << "en-GB" << "I do";
        QTest::newRow("not English") << "i giorni" << "" << false << "it" << "i giorni";
        QTest::newRow("Dutch's ij") << "ijs" << "" << true << "nl" << "Ijs";
        QTest::newRow("spaces") << "  a   b  " << "x" << false << "en" << " a b";
        QTest::newRow("nothing") << "  " << "x" << true << "en" << "";
        QTest::newRow("accented capital") << "élan" << "" << true << "fr" << "Élan";
    }

    /*
     * A phrase goes into the text trimmed, after a space where it follows a
     * word, capitalised where a sentence starts; English gets its "I".
     */
    void join()
    {
        QFETCH(QString, spoken);
        QFETCH(QString, before);
        QFETCH(bool, capital);
        QFETCH(QString, language);
        QFETCH(QString, text);
        QCOMPARE(Pre::Speech::join(spoken, before, capital, language), text);
    }

    // --- VoskDictation without Vosk -------------------------------------------------------------

    // No model for the language: it says so, without asking for the microphone.
    void noModel()
    {
        auto s = std::make_shared<Script>();
        QTemporaryDir root;
        VoskDictation d(root.path(), from(s));
        Heard h;
        h.listen(d);
        QVERIFY(!d.available(QStringLiteral("en")));
        d.start(QStringLiteral("en"));
        QVERIFY(d.listening());
        QTRY_COMPARE(h.stops, 1);
        QCOMPARE(h.problem, QStringLiteral("No speech model for this language"));
        QVERIFY(!d.listening());
        QCOMPARE(s->opened.load(), 0);
    }

    // A model installed while it runs is found.
    void aModelInstalledLater()
    {
        QTemporaryDir root;
        VoskDictation d(root.path(), from(std::make_shared<Script>()));
        QVERIFY(!d.available(QStringLiteral("en")));
        fakeModel(root.filePath(QStringLiteral("en-US")));
        QVERIFY(d.available(QStringLiteral("en")));
    }

    // No libvosk: the keyboard runs, and dictation says it isn't installed.
    void notInstalled()
    {
        auto s = std::make_shared<Script>();
        QTemporaryDir root;
        fakeModel(root.filePath(QStringLiteral("en-US")));
        VoskDictation d(root.path(), from(s), QStringLiteral("vosk-not-installed"));
        Heard h;
        h.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_COMPARE(h.stops, 1);
        QCOMPARE(h.problem, QStringLiteral("Speech recognition isn't installed"));
        QCOMPARE(h.all.size(), 1);
        QCOMPARE(s->opened.load(), 0);
        QVERIFY(!d.listening());
        // And again, the same.
        d.start(QStringLiteral("en"));
        QTRY_COMPARE(h.stops, 2);
        QCOMPARE(h.problem, QStringLiteral("Speech recognition isn't installed"));
    }

    // Stopping or cancelling when not listening does nothing.
    void stopWhenNotListening()
    {
        QTemporaryDir root;
        VoskDictation d(root.path(), from(std::make_shared<Script>()));
        Heard h;
        h.listen(d);
        d.stop();
        d.cancel();
        QTest::qWait(50);
        QCOMPARE(h.all, QStringList());
    }

    // --- VoskDictation with Vosk ----------------------------------------------------------------

    // What is said comes phrase by phrase, the words so far before each; the end of the recording ends it.
    void transcribes()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->pcm = m_speech;
        VoskDictation d(m_models, from(s));
        Heard h;
        h.listen(d);
        QVERIFY(d.available(QStringLiteral("en")));
        QVERIFY(!d.modelLoaded());
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QCOMPARE(h.finals, SPOKEN);
        QCOMPARE(h.problem, QString());
        QVERIFY(!d.listening());
        QVERIFY(d.modelLoaded());
        QCOMPARE(h.all.last(), QStringLiteral("stopped: "));
        // Some words came before each phrase.
        const int firstFinal = int(h.all.indexOf(QStringLiteral("final: ") + SPOKEN[0]));
        QVERIFY(firstFinal > 0);
        QVERIFY(h.all.mid(0, firstFinal).contains(QStringLiteral("partial: one zero zero zero one")));
        QCOMPARE(s->opened.load(), 1);

        // Again, with the model kept: the same words.
        h.finals.clear();
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 2, 60000);
        QCOMPARE(h.finals, SPOKEN);
    }

    // Quiet after the words ends listening, without a problem; quiet for long with nothing said says so.
    void quietEnds()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->pcm = m_speech;
        s->endless = true;
        VoskDictation d(m_models, from(s));
        Heard h;
        h.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QCOMPARE(h.finals, SPOKEN);
        QCOMPARE(h.problem, QString());
        // It listened to about QUIET_MS of quiet after the last words.
        const qint64 heardMs = qint64(s->reads.load()) * VoskDictation::CHUNK / 32;
        const qint64 speechMs = m_speech.size() / 32;
        QVERIFY2(heardMs > speechMs && heardMs < speechMs + VoskDictation::QUIET_MS + 1500,
                 qPrintable(QStringLiteral("%1 ms heard, %2 ms of speech").arg(heardMs).arg(speechMs)));

        auto quiet = std::make_shared<Script>();
        quiet->endless = true;
        VoskDictation q(m_models, from(quiet));
        Heard hq;
        hq.listen(q);
        q.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(hq.stops == 1, 60000);
        QCOMPARE(hq.problem, QStringLiteral("Didn't catch that"));
        QCOMPARE(hq.finals, QStringList());
        const qint64 quietMs = qint64(quiet->reads.load()) * VoskDictation::CHUNK / 32;
        QVERIFY(quietMs > VoskDictation::NOTHING_MS && quietMs < VoskDictation::NOTHING_MS + 500);
    }

    // stop() ends it with what was being said; cancel() at once, with nothing more after.
    void stopAndCancel()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->pcm = m_speech;
        s->endless = true;
        s->paceUs = 2000;
        VoskDictation d(m_models, from(s));
        Heard h;
        h.listen(d);
        bool stopped = false;
        connect(&d, &Pre::Dictation::partial, &d, [&](const QString &t) {
            if (!stopped && t == QLatin1String("one zero")) { stopped = true; d.stop(); }
        });
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QVERIFY(stopped);
        QCOMPARE(h.problem, QString());
        QCOMPARE(h.finals.size(), 1);
        QVERIFY2(h.finals[0].startsWith(QLatin1String("one zero")), qPrintable(h.finals[0]));

        // Cancelled: stopped at once, and nothing after, though the thread was mid-phrase.
        Heard c;
        c.listen(d);
        bool cancelled = false;
        connect(&d, &Pre::Dictation::partial, &d, [&](const QString &t) {
            if (!cancelled && t == QLatin1String("one zero")) {
                cancelled = true;
                d.cancel();
                QVERIFY(!d.listening());
                QCOMPARE(c.stops, 1);
            }
        });
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(cancelled, 60000);
        QTest::qWait(500);
        QCOMPARE(c.stops, 1);
        QCOMPARE(c.finals, QStringList());
        QCOMPARE(c.all.last(), QStringLiteral("stopped: "));

        // A new dictation straight after hears from the start.
        Heard n;
        n.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(n.stops == 1, 60000);
        QVERIFY2(n.finals.value(0).startsWith(QLatin1String("one zero")), qPrintable(n.finals.join(QLatin1Char('|'))));
    }

    /*
     * Cancelled and started again at once, while what was heard before is
     * still on its way: none of it reaches the new dictation.
     */
    void nothingOfACancelledDictationAfter()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->pcm = m_speech;
        s->endless = true;
        VoskDictation d(m_models, from(s));
        Heard h;
        h.listen(d);
        bool restarted = false;
        int at = -1;
        connect(&d, &Pre::Dictation::partial, &d, [&](const QString &t) {
            if (restarted || !t.startsWith(QLatin1String("one"))) return;
            // Busy here while the listening thread goes on and sends more.
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            restarted = true;
            s->hush = true;
            d.cancel();
            at = int(h.all.size());
            d.start(QStringLiteral("en"));
        });
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 2, 60000);
        QVERIFY(restarted);
        // The second heard only quiet.
        QCOMPARE(h.all.mid(at), (QStringList{ QStringLiteral("stopped: Didn't catch that") }));
    }

    // The microphone failing: the words before it still come, then why.
    void microphoneProblems()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->open = false;
        s->openProblem = QStringLiteral("Can't use the microphone (Connection refused)");
        VoskDictation d(m_models, from(s));
        Heard h;
        h.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QCOMPARE(h.problem, s->openProblem);

        s->open = true;
        s->pcm = m_speech.left(m_speech.size() / 2);
        s->fail = true;
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 2, 60000);
        QCOMPARE(h.problem, QStringLiteral("The microphone stopped"));
        QCOMPARE(h.finals.value(0), SPOKEN[0]);

        VoskDictation none(m_models, [] { return std::unique_ptr<AudioSource>(); });
        Heard hn;
        hn.listen(none);
        none.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(hn.stops == 1, 60000);
        QCOMPARE(hn.problem, QStringLiteral("Can't use the microphone"));
    }

    // A folder that looks like a model but isn't one.
    void unreadableModel()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        QTemporaryDir root;
        fakeModel(root.filePath(QStringLiteral("en-US")), QByteArray("not a model"));
        VoskDictation d(root.path(), from(std::make_shared<Script>()));
        Heard h;
        h.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QCOMPARE(h.problem, QStringLiteral("The speech model can't be read"));
        QVERIFY(!d.modelLoaded());
    }

    // The model is let go when unused a while, and loaded again when wanted.
    void idleModelIsLetGo()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->pcm = m_speech.left(32 * 2500);
        VoskDictation d(m_models, from(s));
        d.setIdle(300);
        Heard h;
        h.listen(d);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 1, 60000);
        QTRY_VERIFY_WITH_TIMEOUT(!d.modelLoaded(), 5000);
        d.start(QStringLiteral("en"));
        QTRY_VERIFY_WITH_TIMEOUT(h.stops == 2, 60000);
        QCOMPARE(h.finals.value(0), SPOKEN[0]);
        QCOMPARE(h.finals.value(1), SPOKEN[0]);
    }

    // Going away while listening: it stops listening and lets the model go, at once.
    void destroyedWhileListening()
    {
        if (!needVosk()) QSKIP("Set VOSK_TEST_DIR, and LD_LIBRARY_PATH for libvosk.so.0");
        auto s = std::make_shared<Script>();
        s->endless = true;
        s->paceUs = 20000;
        QElapsedTimer t;
        {
            VoskDictation d(m_models, from(s));
            d.start(QStringLiteral("en"));
            QTRY_VERIFY_WITH_TIMEOUT(s->reads.load() > 2, 60000);
            t.start();
        }
        QVERIFY2(t.elapsed() < 1000, qPrintable(QString::number(t.elapsed())));
        // Nothing queued from it is delivered after.
        QTest::qWait(100);
    }
};

QTEST_GUILESS_MAIN(TestDictation)
#include "ut_dictation.moc"
