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

#ifndef MALIIT_KEYBOARD_KEYPAIRING_H
#define MALIIT_KEYBOARD_KEYPAIRING_H

#include <QSet>

namespace MaliitKeyboard {

/*!
 * \brief Which physical keys went down while the keyboard had them.
 *
 * A key only reaches the input method while a field has its focus. Pressed with
 * none, it goes to the application; if the application puts the focus in a
 * field there and then (Messaging types to compose), the key's release comes to
 * the input method, which never saw it go down. A physical keyboard that
 * repeats types on the release, so that release typed the letter a second
 * time: "QQuick" after a send. A release is the keyboard's only if its press was.
 *
 * Keys are told apart by scancode; a kernel repeating a held key sends its
 * press again, which changes nothing here.
 */
class KeyPairing
{
public:
    //! A key went down, here.
    void press(quint32 scanCode)
    {
        if (scanCode)
            m_down.insert(scanCode);
    }

    /*!
     * \brief A key came up: whether its press was seen here.
     *
     * False means the press went to the application, and the release goes nowhere.
     * A key with no scancode (made up by software) cannot be paired and is let through.
     */
    bool release(quint32 scanCode)
    {
        if (!scanCode)
            return true;
        return m_down.remove(scanCode);
    }

private:
    QSet<quint32> m_down;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_KEYPAIRING_H
