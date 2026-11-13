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

#ifndef MALIIT_KEYBOARD_PERIODHOLD_H
#define MALIIT_KEYBOARD_PERIODHOLD_H

#include <QObject>
#include <QSet>
#include <QTimer>

namespace MaliitKeyboard {

/*!
 * \brief A physical keyboard's period, held, dictates.
 *
 * As the Pre Keyboard's on-screen period does: tapped it is a period, held for
 * HOLD_MS it listens. Whatever key types the period - its own on a USB keyboard,
 * Alt and a letter on a Titan or a KEY2 - so it works on any physical keyboard.
 *
 * Which means a period cannot go in when its key goes down: until it comes back
 * up, or is held long enough, there is no telling which it was. So its press is
 * kept back, and comes out whole (press and release together) when the key is
 * let go in time, or when another key goes down first, so a quick "." then
 * space keeps its order. Held, nothing of it is typed: held() is emitted and
 * the rest of that keystroke, the kernel's repeats among them, is swallowed.
 *
 * Keys are told apart by scancode, which is all a repeating kernel sends twice.
 */
class PeriodHold : public QObject
{
    Q_OBJECT

public:
    //! What the caller does with the key it was given.
    enum Verdict {
        //! On as it would have gone.
        Pass,
        //! Nothing of it reaches the editor.
        Swallow,
        //! A period kept back goes in whole now, and this key no further.
        TypePeriod,
        //! A period kept back goes in whole now, then this key as usual.
        TypePeriodFirst
    };

    //! As long as the on-screen period is held to listen (HOLD_FOR_VOICE, src/pre/prekeyboardview.cpp).
    static constexpr int HOLD_MS = 450;

    explicit PeriodHold(QObject *parent = nullptr);

    //! For the tests.
    void setHoldMs(int ms) { m_timer.setInterval(ms); }

    /*!
     * A key went down. [period]: it types a period. [canDictate]: holding it
     * would listen here - a field for prose with a model for its language, and
     * not listening already. A period that could not is a period at once.
     */
    Verdict press(quint32 scanCode, bool period, bool canDictate);
    Verdict release(quint32 scanCode);

    //! The rest of this key's keystroke, up to its release, reaches nothing.
    void swallow(quint32 scanCode) { if (scanCode != 0) m_swallowed.insert(scanCode); }

    //! Another field, or the keyboard gone: nothing is kept back and nothing swallowed.
    void reset();

    //! A period is kept back, not yet held long enough.
    bool pending() const { return m_period != 0 && !m_held; }

Q_SIGNALS:
    //! The period was held long enough: listen.
    void held();

private:
    QTimer m_timer;
    //! The period's key while it is down, and whether it has been held long enough.
    quint32 m_period = 0;
    bool m_held = false;
    //! Keys down whose keystrokes reach nothing more, up to their release.
    QSet<quint32> m_swallowed;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_PERIODHOLD_H
