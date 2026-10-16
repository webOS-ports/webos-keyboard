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

#include "t9multitap.h"

namespace MaliitKeyboard {

QString T9Multitap::cycleFor(Qt::Key key)
{
    switch (key) {
    case Qt::Key_1: return QStringLiteral(".,?!1");
    case Qt::Key_2: return QStringLiteral("abc2");
    case Qt::Key_3: return QStringLiteral("def3");
    case Qt::Key_4: return QStringLiteral("ghi4");
    case Qt::Key_5: return QStringLiteral("jkl5");
    case Qt::Key_6: return QStringLiteral("mno6");
    case Qt::Key_7: return QStringLiteral("pqrs7");
    case Qt::Key_8: return QStringLiteral("tuv8");
    case Qt::Key_9: return QStringLiteral("wxyz9");
    case Qt::Key_0: return QStringLiteral(" 0");
    default:        return QString();
    }
}

T9Multitap::Step T9Multitap::press(Qt::Key key, bool sameCharacter)
{
    const QString cycle = cycleFor(key);
    if (cycle.isEmpty())
        return Step();

    Step step;
    if (sameCharacter && m_cycling && key == m_key) {
        m_index = (m_index + 1) % int(cycle.length());
        step.replacesPrevious = true;
    } else {
        m_cycling = true;
        m_key = key;
        m_index = 0;
    }
    step.character = cycle.at(m_index);
    return step;
}

void T9Multitap::end()
{
    m_cycling = false;
    m_key = Qt::Key_unknown;
    m_index = 0;
}

} // namespace MaliitKeyboard
