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

#ifndef MALIIT_KEYBOARD_PANELSTATE_H
#define MALIIT_KEYBOARD_PANELSTATE_H

namespace MaliitKeyboard {

/*! \brief Whether the panel is wanted, and what of it.
 *
 * Four inputs decide it and they are all invisible from outside, which is why
 * this is a class of its own rather than four members of InputMethod. Every bug
 * in this area has been a transition rather than a state - a dismissal surviving
 * a focus change, a drag leaving the candidate bar behind, a toggle that had to
 * be used twice - and a transition is only testable if it can be driven without a
 * Maliit plugin, a QQuickView and a compositor behind it. So the rules live here,
 * where a unit test can drive them directly, and InputMethod holds one of these
 * and does as it says.
 *
 * The two user flags are deliberately separate. Dragging the keys away when there
 * are suggestions to keep leaves the candidate bar, which is not the same thing as
 * taking the panel away altogether - and a zero-height panel comes back full
 * height and blank (KeyboardView.qml), so "nothing left" has to mean "gone".
 */
class PanelState
{
public:
    //! \brief A field wants input, or has stopped wanting it.
    //!
    //! Focus arriving is a fresh intent and clears both user flags: a dismissal
    //! belonged to the field that had focus a moment ago.
    void setFocused(bool focused);

    //! \brief The framework switched between a physical and an on-screen keyboard.
    //!
    //! Also a fresh intent, for the same reason: asking for the other input source
    //! is not a state an earlier dismissal should survive. Without this, dragging
    //! the keyboard away and then using the shell's toggle did nothing at all.
    void setHardware(bool hardware);

    //! \brief Whether the word engine has anything to offer.
    void setWordEngine(bool enabled);

    /*! \brief Whether the focused field will accept an on-screen keyboard.
     *
     * Asked for by the field, by way of the content hint. A field with
     * a keypad of its own -- a dialer, a PIN pad -- wants everything else the
     * input method does, the content type and the key redirection that gives a
     * phone QWERTY its digits, and nothing drawn over the keypad it already
     * has.
     *
     * Not a fresh intent, unlike focus and the input source: it is a fact about
     * the field rather than something the user did, so it clears no flags.
     */
    void setOnScreenKeyboardAllowed(bool allowed);

    //! \brief The user dragged the keys away.
    //!
    //! Keeps the candidate bar where there is one to keep, and takes the whole
    //! panel where there is not.
    void collapseKeys();

    //! \brief The user asked for the keys back.
    void expandKeys();

    //! \brief The panel window was closed under us.
    void dismiss();

    bool focused() const { return m_focused; }
    bool hardware() const { return m_hardware; }
    bool wordEngine() const { return m_wordEngine; }
    bool keysCollapsed() const { return m_keysCollapsed; }
    bool dismissed() const { return m_dismissed; }
    bool onScreenKeyboardAllowed() const { return m_onScreenKeyboardAllowed; }

    /*! \brief Whether the keys are off screen.
     *
     * True with a physical keyboard whether or not anything was dragged: the keys
     * are not drawn for a keyboard that already has keys.
     */
    bool keysHidden() const
    {
        return m_hardware or m_keysCollapsed or not m_onScreenKeyboardAllowed;
    }

    /*! \brief Whether any panel at all should be on screen.
     *
     * With the keys hidden the panel is just the candidate bar, which is only
     * worth putting up if there is one.
     */
    bool panelWanted() const;

private:
    bool m_focused = false;
    bool m_hardware = false;
    bool m_wordEngine = false;
    //! The user dragged the keys off; the bar stays.
    bool m_keysCollapsed = false;
    //! The user took the whole panel away.
    bool m_dismissed = false;
    //! True until a field says otherwise, so a field that says nothing behaves
    //! exactly as every field always has.
    bool m_onScreenKeyboardAllowed = true;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_PANELSTATE_H
