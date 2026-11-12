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

#include "prevosk.h"

#include <QLibrary>
#include <QMetaObject>
#include <QPointer>

#include <pulse/error.h>
#include <pulse/simple.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

// libvosk's handles, as its vosk_api.h has them: only ever pointed at.
struct VoskModel;
struct VoskRecognizer;

namespace Pre {

namespace {

enum What { Partial, Final, Stopped };

//! What is wanted of a dictation once it is listening.
enum Ending { Listen = 0, Finish = 1, Drop = 2 };

/*!
 * The few calls of libvosk's C API used, looked up when it is first needed:
 * the keyboard is built and runs without it.
 */
struct Vosk
{
    QLibrary library;
    VoskModel *(*modelNew)(const char *) = nullptr;
    void (*modelFree)(VoskModel *) = nullptr;
    VoskRecognizer *(*recognizerNew)(VoskModel *, float) = nullptr;
    void (*recognizerFree)(VoskRecognizer *) = nullptr;
    void (*recognizerReset)(VoskRecognizer *) = nullptr;
    int (*acceptWaveform)(VoskRecognizer *, const char *, int) = nullptr;
    const char *(*result)(VoskRecognizer *) = nullptr;
    const char *(*partialResult)(VoskRecognizer *) = nullptr;
    const char *(*finalResult)(VoskRecognizer *) = nullptr;
    void (*setLogLevel)(int) = nullptr;

    template<typename F> bool find(F &f, const char *name)
    {
        f = reinterpret_cast<F>(library.resolve(name));
        return f != nullptr;
    }

    bool load(const QString &name)
    {
        if (library.isLoaded()) return true;
        library.setFileNameAndVersion(name, 0);
        if (!library.load()) return false;
        if (find(modelNew, "vosk_model_new") && find(modelFree, "vosk_model_free")
            && find(recognizerNew, "vosk_recognizer_new") && find(recognizerFree, "vosk_recognizer_free")
            && find(recognizerReset, "vosk_recognizer_reset")
            && find(acceptWaveform, "vosk_recognizer_accept_waveform")
            && find(result, "vosk_recognizer_result") && find(partialResult, "vosk_recognizer_partial_result")
            && find(finalResult, "vosk_recognizer_final_result") && find(setLogLevel, "vosk_set_log_level")) {
            setLogLevel(-1);
            return true;
        }
        library.unload();
        return false;
    }
};

class PulseMicrophone : public AudioSource
{
public:
    ~PulseMicrophone() override
    {
        if (m_stream) pa_simple_free(m_stream);
    }

    bool open(QString *problem) override
    {
        const pa_sample_spec spec = { PA_SAMPLE_S16LE, 16000, 1 };
        // Small fragments, so the words show as they are said.
        pa_buffer_attr attr;
        attr.maxlength = static_cast<uint32_t>(-1);
        attr.tlength = static_cast<uint32_t>(-1);
        attr.prebuf = static_cast<uint32_t>(-1);
        attr.minreq = static_cast<uint32_t>(-1);
        attr.fragsize = VoskDictation::CHUNK;
        int error = 0;
        // The default server first; LuneOS's is the system one, whose socket
        // a client that isn't its user may not look for.
        for (const char *server : { static_cast<const char *>(nullptr), "unix:/run/pulse/native" }) {
            m_stream = pa_simple_new(server, "LuneOS keyboard", PA_STREAM_RECORD, nullptr, "Dictation",
                                     &spec, nullptr, &attr, &error);
            if (m_stream) return true;
        }
        if (problem) *problem = QStringLiteral("Can't use the microphone (%1)").arg(QString::fromUtf8(pa_strerror(error)));
        return false;
    }

    int read(char *buffer, int bytes) override
    {
        int error = 0;
        if (!m_stream || pa_simple_read(m_stream, buffer, static_cast<size_t>(bytes), &error) < 0) return -1;
        return bytes;
    }

private:
    pa_simple *m_stream = nullptr;
};

} // namespace

std::unique_ptr<AudioSource> microphone()
{
    return std::make_unique<PulseMicrophone>();
}

/*!
 * What the keyboard's thread and the listening thread share, under [mutex]:
 * the dictation wanted next, and how the one going on should end.
 */
struct VoskDictation::Shared
{
    std::mutex mutex;
    std::condition_variable wake;
    bool quit = false;
    //! A dictation wanted: its number, model and language.
    bool wanted = false;
    quint64 session = 0;
    QString model;
    QString language;
    //! How the dictation [session] is to end (Ending), read as each piece of sound is.
    std::atomic<int> ending { Listen };
    std::atomic<quint64> ends { 0 };
    std::atomic<int> idleMs { IDLE_MS };
    std::atomic<bool> loaded { false };
    QString library;
    AudioFactory audio;
};

VoskDictation::VoskDictation(const QString &modelRoot, AudioFactory audio, const QString &library, QObject *parent)
    : Dictation(parent)
    , m_root(modelRoot)
    , m_shared(std::make_shared<Shared>())
{
    m_shared->library = library;
    m_shared->audio = std::move(audio);
    m_thread = std::thread([this] { run(); });
}

VoskDictation::~VoskDictation()
{
    {
        std::lock_guard<std::mutex> lock(m_shared->mutex);
        m_shared->quit = true;
        m_shared->wanted = false;
        m_shared->ending = Drop;
    }
    m_shared->wake.notify_all();
    m_thread.join();
}

bool VoskDictation::available(const QString &language) const
{
    // Looked for each time: a model can be installed while the keyboard runs.
    return !Speech::modelFor(m_root, language).isEmpty();
}

void VoskDictation::start(const QString &language)
{
    if (m_listening) return;
    const QString model = Speech::modelFor(m_root, language);
    ++m_session;
    m_listening = true;
    if (model.isEmpty()) {
        deliver(m_session, Stopped, QStringLiteral("No speech model for this language"));
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_shared->mutex);
        m_shared->wanted = true;
        m_shared->session = m_session;
        m_shared->model = model;
        m_shared->language = language;
        m_shared->ends = m_session;
        m_shared->ending = Listen;
    }
    m_shared->wake.notify_all();
}

void VoskDictation::stop()
{
    if (!m_listening) return;
    if (m_shared->ends == m_session) m_shared->ending = Finish;
}

void VoskDictation::cancel()
{
    if (!m_listening) return;
    {
        std::lock_guard<std::mutex> lock(m_shared->mutex);
        if (m_shared->wanted && m_shared->session == m_session) m_shared->wanted = false;
        if (m_shared->ends == m_session) m_shared->ending = Drop;
    }
    // Nothing more of this dictation is passed on, whatever the thread still sends.
    ++m_session;
    m_listening = false;
    Q_EMIT stopped(QString());
}

void VoskDictation::setIdle(int ms)
{
    m_shared->idleMs = ms;
    m_shared->wake.notify_all();
}

bool VoskDictation::modelLoaded() const
{
    return m_shared->loaded;
}

//! On the keyboard's thread: what the listening thread heard, unless it is about a dictation since dropped.
void VoskDictation::deliver(quint64 session, int what, const QString &text)
{
    QPointer<VoskDictation> self(this);
    // A copy of its own: the listening thread's strings stay its own.
    const QString own(text.constData(), text.size());
    QMetaObject::invokeMethod(this, [self, session, what, text = own] {
        if (!self || session != self->m_session || !self->m_listening) return;
        switch (what) {
        case Partial: Q_EMIT self->partial(text); break;
        case Final: Q_EMIT self->final(text); break;
        default:
            self->m_listening = false;
            Q_EMIT self->stopped(text);
            break;
        }
    }, Qt::QueuedConnection);
}

//! The listening thread: waits for a dictation, listens, and lets the model go when it has been unused a while.
void VoskDictation::run()
{
    Shared &s = *m_shared;
    Vosk vosk;
    VoskModel *model = nullptr;
    VoskRecognizer *recognizer = nullptr;
    QString loadedModel;
    auto unload = [&] {
        if (recognizer) vosk.recognizerFree(recognizer);
        if (model) vosk.modelFree(model);
        recognizer = nullptr;
        model = nullptr;
        loadedModel.clear();
        s.loaded = false;
    };

    for (;;) {
        quint64 session = 0;
        QString wantModel;
        {
            std::unique_lock<std::mutex> lock(s.mutex);
            auto ready = [&] { return s.quit || s.wanted; };
            if (model) {
                if (!s.wake.wait_for(lock, std::chrono::milliseconds(s.idleMs.load()), ready)) {
                    unload();
                    continue;
                }
            } else {
                s.wake.wait(lock, ready);
            }
            if (s.quit) break;
            s.wanted = false;
            session = s.session;
            wantModel = s.model;
        }

        auto ending = [&] { return s.ends == session ? s.ending.load() : int(Drop); };

        if (!vosk.load(s.library)) {
            deliver(session, Stopped, QStringLiteral("Speech recognition isn't installed"));
            continue;
        }
        if (loadedModel != wantModel) {
            unload();
            model = vosk.modelNew(wantModel.toLocal8Bit().constData());
            recognizer = model ? vosk.recognizerNew(model, 16000.0f) : nullptr;
            if (!recognizer) {
                unload();
                deliver(session, Stopped, QStringLiteral("The speech model can't be read"));
                continue;
            }
            loadedModel = wantModel;
            s.loaded = true;
        } else {
            vosk.recognizerReset(recognizer);
        }

        std::unique_ptr<AudioSource> audio = s.audio ? s.audio() : nullptr;
        QString problem;
        if (!audio || !audio->open(&problem)) {
            deliver(session, Stopped, problem.isEmpty() ? QStringLiteral("Can't use the microphone") : problem);
            continue;
        }

        char buffer[CHUNK];
        qint64 bytes = 0;
        qint64 wordsAt = 0;
        bool heard = false;
        QString shown;
        int end = Listen;
        while ((end = ending()) == Listen) {
            const int n = audio->read(buffer, CHUNK);
            if (n <= 0) {
                if (n < 0) problem = QStringLiteral("The microphone stopped");
                end = Finish;
                break;
            }
            bytes += n;
            const qint64 ms = bytes / 32;
            if (vosk.acceptWaveform(recognizer, buffer, n)) {
                const QString text = Speech::resultText(QByteArray(vosk.result(recognizer)), "text");
                if (!text.isEmpty()) {
                    deliver(session, Final, text);
                    heard = true;
                    wordsAt = ms;
                }
                shown.clear();
            } else {
                const QString text = Speech::resultText(QByteArray(vosk.partialResult(recognizer)), "partial");
                if (text != shown) {
                    shown = text;
                    deliver(session, Partial, text);
                    if (!text.isEmpty()) {
                        heard = true;
                        wordsAt = ms;
                    }
                }
            }
            if (heard ? ms - wordsAt > QUIET_MS : ms > NOTHING_MS) {
                if (!heard) problem = QStringLiteral("Didn't catch that");
                end = Finish;
                break;
            }
            if (ms > LONGEST_MS) {
                end = Finish;
                break;
            }
        }
        audio.reset();
        if (end == Drop || ending() == Drop) {
            // Finished and thrown away: vosk_recognizer_reset() keeps the
            // sound not yet decoded, which the next dictation would begin with.
            vosk.finalResult(recognizer);
            continue;
        }
        const QString last = Speech::resultText(QByteArray(vosk.finalResult(recognizer)), "text");
        if (!last.isEmpty()) deliver(session, Final, last);
        deliver(session, Stopped, problem);
    }
    unload();
}

} // namespace Pre
