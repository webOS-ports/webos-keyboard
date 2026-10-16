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

#ifndef MALIIT_KEYBOARD_T9MULTITAP_H
#define MALIIT_KEYBOARD_T9MULTITAP_H

#include <QChar>
#include <QString>
#include <Qt>

namespace MaliitKeyboard {

/*! \brief Multi-tap on a telephone keypad: which character a press gives.
 *
 * A keypad's 2 gives a, then b, then c, then 2, each press within the window
 * replacing the character before it; a press of another key, or one after the
 * window, starts a new character. InputMethod owns the window (a timer) and
 * what is done with the character - shown in the preedit, or committed
 * straight away in a field that hides what is typed. This holds only the
 * cycle, so that its rules can be driven by a unit test without a Maliit
 * plugin behind it.
 */
class T9Multitap
{
public:
    //! What a press gives: the character, and whether it takes the place of
    //! the one before it (the same key again, within the window) or is new.
    //! A null character: the key is not on the keypad.
    struct Step {
        QChar character;
        bool replacesPrevious = false;
    };

    //! The characters a key cycles through, ending with the digit itself.
    //! Empty for a key that is not on the keypad.
    static QString cycleFor(Qt::Key key);

    //! A press of a key. sameCharacter: it continues the character being
    //! cycled (the same key, within the window) rather than starting one.
    Step press(Qt::Key key, bool sameCharacter);

    //! The character being cycled is done: the next press starts another.
    void end();

    //! Whether a character is being cycled, and on which key.
    bool cycling() const { return m_cycling; }
    Qt::Key key() const { return m_key; }
    int index() const { return m_index; }

private:
    bool m_cycling = false;
    Qt::Key m_key = Qt::Key_unknown;
    int m_index = 0;
};

/*! \brief Whether a field gets word prediction.
 *
 * What it asks for (all it is asked when it says nothing), and only in free
 * text: not in a field that hides what is typed - suggestions would put it
 * on the candidate bar - nor in a number, phone number, email address or URL.
 * One rule for both of the places that used to decide it between them: one
 * said yes for a number field and the other no, so that every update of one
 * switched prediction on and off again.
 */
inline bool predictionWanted(bool asked, bool askedValid, bool hiddenText, bool freeText)
{
    return (askedValid ? asked : true) && !hiddenText && freeText;
}

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_T9MULTITAP_H
