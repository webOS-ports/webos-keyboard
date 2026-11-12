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

#ifndef PRE_VOSK_H
#define PRE_VOSK_H

#include "predictation.h"

#include <functional>
#include <memory>
#include <thread>

namespace Pre {

//! Sound to listen to: 16 kHz, one channel, signed 16-bit little-endian.
class AudioSource
{
public:
    virtual ~AudioSource() = default;
    //! Starts; false, saying why in [problem] if it can, when it can't.
    virtual bool open(QString *problem) = 0;
    //! Up to [bytes] of sound into [buffer], waiting for it: how much; 0 at the end, less on an error.
    virtual int read(char *buffer, int bytes) = 0;
};

//! The microphone, through PulseAudio.
std::unique_ptr<AudioSource> microphone();

/*!
 * Dictation on the device, with Vosk (libvosk.so.0, loaded when first
 * needed, so the keyboard works without it) and a model per language in
 * [modelRoot]. Listening, loading and recognising happen on a thread of
 * their own. The model and its recogniser are kept between dictations - a
 * new recogniser builds its graph on the first words it hears, seconds on
 * a phone - and let go after IDLE_MS unused.
 */
class VoskDictation : public Dictation
{
    Q_OBJECT

public:
    using AudioFactory = std::function<std::unique_ptr<AudioSource>()>;

    explicit VoskDictation(const QString &modelRoot, AudioFactory audio = microphone,
                           const QString &library = QStringLiteral("vosk"), QObject *parent = nullptr);
    ~VoskDictation() override;

    bool available(const QString &language) const override;
    void start(const QString &language) override;
    void stop() override;
    void cancel() override;
    bool listening() const override { return m_listening; }

    //! After words, this much quiet (ms of sound) ends listening.
    static constexpr int QUIET_MS = 3000;
    //! This long (ms of sound) with nothing said ends it: "Didn't catch that".
    static constexpr int NOTHING_MS = 6000;
    //! The longest listening goes on (ms of sound).
    static constexpr int LONGEST_MS = 120000;
    //! The model is let go after this long (ms) unused.
    static constexpr int IDLE_MS = 180000;
    //! Sound read at a time, in bytes: 0.1 s.
    static constexpr int CHUNK = 3200;

    //! For the tests: how long the model is kept unused (ms), and whether it is loaded now.
    void setIdle(int ms);
    bool modelLoaded() const;

private:
    struct Shared;
    void run();
    void deliver(quint64 session, int what, const QString &text);

    const QString m_root;
    std::shared_ptr<Shared> m_shared;
    bool m_listening = false;
    quint64 m_session = 0;
    std::thread m_thread;
};

} // namespace Pre

#endif // PRE_VOSK_H
