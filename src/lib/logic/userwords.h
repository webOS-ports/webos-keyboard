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

#ifndef MALIIT_KEYBOARD_USERWORDS_H
#define MALIIT_KEYBOARD_USERWORDS_H

#include <QString>
#include <QStringList>

namespace MaliitKeyboard {
namespace Logic {

/*
 * The owner's saved words that are [word] in any case, as they were saved:
 * what deleting [word] takes out. A deleted word is blocked without regard to
 * case (AbstractWordEngine::isBlocked), so holding "webos" in the bar has to
 * take out the "webOS" Text Assist saved, or it stays in the list there.
 */
inline QStringList savedSpellings(const QStringList &saved, const QString &word)
{
    QStringList out;
    if (word.isEmpty())
        return out;
    for (const QString &s : saved)
        if (s.compare(word, Qt::CaseInsensitive) == 0 && !out.contains(s))
            out << s;
    return out;
}

} // namespace Logic
} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_USERWORDS_H
