// -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; c-file-offsets: ((innamespace . 0)); -*-
/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <QDir>
#include <QFile>
#include <QRegExp>

#include "coreutils.h"

#include "keyboardloader.h"

namespace MaliitKeyboard {

class KeyboardLoaderPrivate
{
public:

    QString active_id;
};

KeyboardLoader::KeyboardLoader(QObject *parent)
    : QObject(parent)
    , d_ptr(new KeyboardLoaderPrivate)
{}

KeyboardLoader::~KeyboardLoader()
{}

QStringList KeyboardLoader::ids() const
{
    QStringList ids;
    ids << d_ptr->active_id; // don't return an empty list
    return ids;
}

QString KeyboardLoader::activeId() const
{
    return d_ptr->active_id;
}

void KeyboardLoader::setActiveId(const QString &id)
{
    d_ptr->active_id = id;
}

QString KeyboardLoader::title(const QString &id) const
{
    Q_UNUSED(id);
    return QString();
}

Keyboard KeyboardLoader::keyboard() const
{
    return Keyboard();
}

Keyboard KeyboardLoader::nextKeyboard() const
{
    return Keyboard();
}

Keyboard KeyboardLoader::previousKeyboard() const
{
    return Keyboard();
}

Keyboard KeyboardLoader::shiftedKeyboard() const
{
    return Keyboard();
}

Keyboard KeyboardLoader::symbolsKeyboard(int page) const
{
    Q_UNUSED(page);
    return Keyboard();
}

Keyboard KeyboardLoader::deadKeyboard(const Key &dead) const
{
    Q_UNUSED(dead);
    return Keyboard();
}

Keyboard KeyboardLoader::shiftedDeadKeyboard(const Key &dead) const
{
    Q_UNUSED(dead);
    return Keyboard();
}

Keyboard KeyboardLoader::extendedKeyboard(const Key &key) const
{
    Q_UNUSED(key);
    return Keyboard();
}

Keyboard KeyboardLoader::numberKeyboard() const
{
    return Keyboard();
}

Keyboard KeyboardLoader::phoneNumberKeyboard() const
{
    return Keyboard();
}

} // namespace MaliitKeyboard
