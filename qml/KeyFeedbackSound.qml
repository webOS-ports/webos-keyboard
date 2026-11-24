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

import QtQuick 2.0
import LuneOS.Service 1.0

/*!
 * \brief The key-press click, played through audiod the way LunaSysMgr played
 *        the on-screen keyboard's tap (SoundPlayerPool::playFeedback).
 *
 * audiod's systemsounds/playFeedback plays a named sound from PulseAudio's
 * sample cache on audiod's own feedback stream: the sound is pre-uploaded, so
 * the click is heard as the key goes down, and - unlike a QML MediaPlayer or
 * SoundEffect - the keyboard holds no PulseAudio stream of its own.
 *
 * That last point is the reason this is not a SoundEffect. A SoundEffect keeps
 * one stream open for the component's lifetime to make the next play instant;
 * in the always-resident maliit server that stream never closes, which stops
 * the audio sink suspending on idle (module-suspend-on-idle) and keeps the
 * audio path powered. luna-next-cardshell hit exactly that and moved its
 * feedback sounds onto audiod (Utils/FeedbackSound.qml); this does the same
 * for the keyboard, and audiod owns and suspends the one feedback stream.
 *
 * A MediaPlayer, which the keyboard used before, did not hold a stream open
 * but opened a fresh one for every play(), so the click came 175-460 ms after
 * the key on a mako. audiod's cached playback is heard as the key goes down
 * without holding anything open.
 */
LunaService {
    id: keyFeedback

    // A com.webos.service.ime* name: the MaliitServer LS2 role only lets the
    // keyboard register com.webos.service.ime*/org.maliit, and those are the
    // names granted the audio.operation group that guards playFeedback
    // (client-permissions.d/com.webos.service.ime.perm.json). org.maliit is
    // already held by the plugin's C++ service, so take a distinct sub-name -
    // with a dot, not a dash, which has bitten LS2 names here before.
    name: "com.webos.service.ime.feedback"
    service: "luna://com.webos.service.audio"
    method: "systemsounds/playFeedback"

    // The system sound under /usr/share/systemsounds, without the extension.
    // "key" is the name audiod recognises as the keyboard tap (key.pcm);
    // "backspace", "space" and "return" exist too for a key that wants its own.
    property string soundName: "key"

    function play() {
        // call() is deferred until the service has registered, so an early tap
        // is not lost; nothing is held open between taps.
        // override:true keeps the keyboard's own "key press feedback" switch
        // (maliit_input_method.useAudioFeedback, checked at the call sites) the
        // sole control, as it was with the MediaPlayer. Without it audiod would
        // also gate "key" on Settings' system "Touch Sound" and the ringer.
        keyFeedback.call(JSON.stringify({ "name": soundName, "override": true }));
    }
}
