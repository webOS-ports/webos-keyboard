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

#include "panelstate.h"

namespace MaliitKeyboard {

bool panelWindowKeepsGeometry(const QRect &window, const QRect &screen)
{
    if (window == screen)
        return true;

    // On its side: the screen's long side across, at most its short side down.
    const bool portraitScreen = screen.height() > screen.width();
    return portraitScreen and window.topLeft() == screen.topLeft()
        and window.width() == screen.height()
        and window.height() > 0 and window.height() <= screen.width();
}

void PanelState::setFocused(bool focused)
{
    m_focused = focused;

    if (focused)
        m_keysCollapsed = m_dismissed = false;
}

void PanelState::setHardware(bool hardware)
{
    if (m_hardware == hardware)
        return;

    m_hardware = hardware;
    m_keysCollapsed = m_dismissed = false;
}

void PanelState::setWordEngine(bool enabled)
{
    m_wordEngine = enabled;
}

void PanelState::collapseKeys()
{
    if (m_wordEngine) {
        // The suggestions are still wanted; only the keys go.
        m_keysCollapsed = true;
    } else {
        // Nothing would be left, and a zero-height panel comes back full height
        // and blank, so take it away properly.
        m_dismissed = true;
    }
}

void PanelState::expandKeys()
{
    m_keysCollapsed = m_dismissed = false;
}

void PanelState::dismiss()
{
    m_dismissed = true;
}

void PanelState::setOnScreenKeyboardAllowed(bool allowed)
{
    m_onScreenKeyboardAllowed = allowed;
}

bool PanelState::panelWanted() const
{
    return m_focused && !m_dismissed && (!keysHidden() || m_wordEngine);
}

} // namespace MaliitKeyboard
