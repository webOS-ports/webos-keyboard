/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2026 WebOS Ports
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

#ifndef MALIIT_KEYBOARD_SPELLCHECKER_H
#define MALIIT_KEYBOARD_SPELLCHECKER_H

#include <QtCore>

#include <luna-service2/lunaservice.h>
#include <glib.h>

class SpellCheckerPrivate;

class SpellChecker
{
    Q_DISABLE_COPY(SpellChecker)
    Q_DECLARE_PRIVATE(SpellChecker)
public:
    // The db8 kind org.webosports.app.settings' Text Assist page keeps its
    // user dictionary in - one record per word, in a "word" property. A
    // poll timer (see POLL_INTERVAL_SECONDS in the .cpp) is what replaces
    // the flat file this used to read: db8 stays the one place a word
    // gets added or removed, and this picks up both a live edit and a
    // post-restore db8 already populated before this ever ran, neither of
    // which the file this used to read (~/.config/maliit/userwords.txt)
    // could do without something else writing to it first. find's own
    // "subscribe":true does not push updates when the result set changes,
    // and "watch":true (see findWordsAndWatch() in the .cpp) turned out to
    // only ever fire once for the life of a subscription rather than
    // continuously - the poll timer is the actual freshness mechanism;
    // watch is kept only as a fast path for the first change.
    explicit SpellChecker(const QString &user_dictionary_kind = QLatin1String("org.webosports.app.settings.dictionary:1"));

    ~SpellChecker();

    bool enabled() const;
    bool setEnabled(bool on);

    bool spell(const QString &word);
    QStringList suggest(const QString &word,
                        int limit = -1);
    void ignoreWord(const QString &word);
    void addToUserWordlist(const QString &word);

    bool setLanguage(const QString& language);

    static QString dictPath();

private:
    void findWordsAndWatch();
    void refreshWords();

    static bool findCallback(LSHandle *handle, LSMessage *message, void *user_data);
    static bool putCallback(LSHandle *handle, LSMessage *message, void *user_data);
    static gboolean pollCallback(gpointer user_data);

    static const unsigned int POLL_INTERVAL_SECONDS = 30;

    const QScopedPointer<SpellCheckerPrivate> d_ptr;
};

#endif // MALIIT_KEYBOARD_SPELLCHECKER_H
