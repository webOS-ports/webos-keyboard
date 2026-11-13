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

#include "periodhold.h"

namespace MaliitKeyboard {

PeriodHold::PeriodHold(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(HOLD_MS);
    // Stopped wherever the period is let go of, so it fires only for one still down.
    connect(&m_timer, &QTimer::timeout, this, [this] {
        m_held = true;
        Q_EMIT held();
    });
}

PeriodHold::Verdict PeriodHold::press(quint32 scanCode, bool period, bool canDictate)
{
    // A key already down, the kernel repeating it: the period kept back or held,
    // or a key whose keystroke is being swallowed.
    if (scanCode != 0 && (scanCode == m_period || m_swallowed.contains(scanCode)))
        return Swallow;

    if (m_period != 0) {
        // Held long enough, it is listening; the key that ends that is the
        // caller's business. Its own release is still to be swallowed.
        if (m_held)
            return Pass;
        // Another key before the period came back up: the period was a tap,
        // and in it goes now. Its release, still to come, has nothing to add.
        m_swallowed.insert(m_period);
        m_period = 0;
        m_timer.stop();
        return TypePeriodFirst;
    }

    if (period && canDictate && scanCode != 0) {
        m_period = scanCode;
        m_held = false;
        m_timer.start();
        return Swallow;
    }
    return Pass;
}

PeriodHold::Verdict PeriodHold::release(quint32 scanCode)
{
    if (scanCode == 0) return Pass;
    if (m_swallowed.remove(scanCode)) return Swallow;
    if (scanCode != m_period) return Pass;
    m_period = 0;
    m_timer.stop();
    if (m_held) {
        m_held = false;
        return Swallow;
    }
    return TypePeriod;
}

void PeriodHold::reset()
{
    m_timer.stop();
    m_period = 0;
    m_held = false;
    m_swallowed.clear();
}

} // namespace MaliitKeyboard
