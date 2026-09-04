/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012 Openismus GmbH
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

#include "spellchecker.h"

#ifdef HAVE_HUNSPELL
#include "hunspell/hunspell.hxx"
#else
class Hunspell
{
public:
    Hunspell(const char *, const char *, const char * = NULL) : encoding("UTF-8") {}
    int add_dic (const char *, const char * = NULL) { return 0; }
    char *get_dic_encoding() { return encoding.data(); }
    int spell(const char *, int * = NULL, char ** = NULL) { return 1; }
    int suggest(char *** lst, const char *) { if (lst) { *lst = NULL; } return 0; }
    void free_list(char ***, int) {}
    int add(const char *) { return 0; }
private:
    // Using QByteArray here instead of just returning "UTF-8" in get_dic_encoding
    // to avoid a following warning:
    // warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
    QByteArray encoding;
};
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#if QT_VERSION >= 0x060000
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif
#include <QStringList>
#include <QDebug>
#include <QDir>

//! \class SpellChecker
//! Checks spelling and suggest words. Currently Spellchecker is
//! implemented by using Hunspell.

struct SpellCheckerPrivate
{
    Hunspell *hunspell; //!< The spellchecker backend, Hunspell.
    QTextCodec *codec; //!< Which codec to use.
    QSet<QString> ignored_words; //!< The words to ignore.
    QString user_dictionary_kind;
    QString aff_file;
    QString dic_file;

    // The db8-backed replacement for what used to be a flat file read once
    // per setEnabled(true)/setLanguage() call: a standing subscription (see
    // SpellChecker::findCallback) keeps this current, live, for as long as
    // the process runs, and applyUserWords() pushes it into whichever
    // Hunspell instance is active right now.
    LSHandle *serviceHandle;
    QStringList userWords;

    SpellCheckerPrivate(const QString &user_dictionary_kind);
    ~SpellCheckerPrivate();
    void applyUserWords();
    void clear();
};


SpellCheckerPrivate::SpellCheckerPrivate(const QString &user_dictionary_kind)
    : hunspell(0)
    , codec(0)
    , ignored_words()
    , user_dictionary_kind(user_dictionary_kind)
    , aff_file()
    , dic_file()
    , serviceHandle(0)
    , userWords()
{
}

SpellCheckerPrivate::~SpellCheckerPrivate()
{
    clear();

    if (serviceHandle) {
        LSError error;
        LSErrorInit(&error);
        if (!LSUnregister(serviceHandle, &error)) {
            qWarning("LSUnregister failed: %s", error.message);
            LSErrorFree(&error);
        }
    }
}

//! \brief SpellCheckerPrivate::applyUserWords adds the cached db8 word list
//! to whichever Hunspell instance is currently active.
void SpellCheckerPrivate::applyUserWords()
{
    if (not hunspell)
        return;

    for (const QString &word : userWords)
        hunspell->add(codec->fromUnicode(word).toStdString());
}

//! \brief SpellCheckerPrivate::clear cleans up all memory and does reset
//! everything for a new language
void SpellCheckerPrivate::clear()
{
    delete(hunspell);
    hunspell = 0;
    aff_file.clear();
    dic_file.clear();
}

SpellChecker::~SpellChecker()
{}

//! \brief SpellChecker::enabled returns if the spechchecking is active
//! \return
bool SpellChecker::enabled() const
{
    Q_D(const SpellChecker);
    return (d->hunspell != 0);
}

//! \brief SpellChecker::setEnabled
//! \param on
//! \return true if setting it enabled/disabled went ok
bool SpellChecker::setEnabled(bool on)
{
    Q_D(SpellChecker);

    if (enabled() == on)
        return true;

    delete(d->hunspell);
    d->hunspell = 0;

    if (not on) {
        return true;
    }

    if (d->aff_file.isEmpty() || d->dic_file.isEmpty()) {
        qWarning() << "no dictionary to turn on spellchecking";
        return false;
    }

    d->hunspell = new Hunspell(d->aff_file.toUtf8().constData(),
                               d->dic_file.toUtf8().constData());

    d->codec = QTextCodec::codecForName(d->hunspell->get_dic_encoding());
    if (not d->codec) {
        qWarning () << Q_FUNC_INFO << ":Could not find codec for" << d->hunspell->get_dic_encoding() << "- turning off spellchecking";
        d->clear();
        return false;
    }

    d->applyUserWords();
    return true;
}

//! \param user_dictionary_kind The db8 kind the user dictionary lives in.
SpellChecker::SpellChecker(const QString &user_dictionary_kind)
    : d_ptr(new SpellCheckerPrivate(user_dictionary_kind))
{
    Q_D(SpellChecker);

    LSError error;
    LSErrorInit(&error);

    // Same registration + subscription shape as KeyboardSettings elsewhere
    // in this plugin (see keyboardsettings.cpp) - a distinct identity of
    // its own, since a process registering the same LS2 name twice fails
    // the second call.
    if (!LSRegister("org.webosports.keyboard.dictionary", &d->serviceHandle, &error)) {
        qWarning("Failed to register service handle: %s", error.message);
        LSErrorFree(&error);
        return;
    }

    if (!LSGmainAttach(d->serviceHandle, g_main_loop_new(g_main_context_default(), TRUE), &error)) {
        qWarning("Failed to attach to glib mainloop: %s", error.message);
        LSErrorFree(&error);
        return;
    }

    QByteArray payload = "{\"subscribe\":true,\"query\":{\"from\":\"" +
                         d->user_dictionary_kind.toUtf8() + "\",\"orderBy\":\"word\"}}";

    if (!LSCall(d->serviceHandle, "luna://com.palm.db/find", payload.constData(),
              SpellChecker::findCallback, this, NULL, &error)) {
        qWarning("Setting up subscription for user dictionary failed: %s", error.message);
        LSErrorFree(&error);
    }
}

bool SpellChecker::findCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    Q_UNUSED(handle);

    if (!message)
        return true;

    const char *payload = LSMessageGetPayload(message);
    if (!payload)
        return true;

    QJsonDocument document = QJsonDocument::fromJson(QByteArray(payload));
    QJsonObject root = document.object();
    if (!root.contains("results") || !root.value("results").isArray())
        return true;

    QJsonArray results = root.value("results").toArray();
    QStringList words;
    for (const QJsonValue &entry : results) {
        QJsonObject obj = entry.toObject();
        if (obj.contains("word") && obj.value("word").isString())
            words.append(obj.value("word").toString());
    }

    SpellChecker *self = static_cast<SpellChecker*>(user_data);
    self->d_func()->userWords = words;
    self->d_func()->applyUserWords();

    return true;
}

bool SpellChecker::putCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    Q_UNUSED(handle);
    Q_UNUSED(user_data);

    if (!message)
        return true;

    const char *payload = LSMessageGetPayload(message);
    if (payload) {
        QJsonObject root = QJsonDocument::fromJson(QByteArray(payload)).object();
        if (!root.value("returnValue").toBool())
            qWarning() << "com.palm.db/put failed:" << payload;
    }

    return true;
}


//! \brief Checks whether given word is spelled correctly.
//!
//! Ignored words are treated as having correct spelling. \sa ignoreWord.
//! \param word word to check for spelling.
//! \return \c true if the word has correct spelling (or is ignored),
//!         otherwise \c false.
bool SpellChecker::spell(const QString &word)
{
    Q_D(SpellChecker);

    if (not enabled() or d->ignored_words.contains(word)) {
        return true;
    }

    return d->hunspell->spell(d->codec->fromUnicode(word).toStdString());
}


//! \brief Gives suggestions for a given word.
//! \param word Base for suggestions.
//! \param limit Suggestion count limit (-1 for no limits).
//! \return a list of suggestions.
QStringList SpellChecker::suggest(const QString &word,
                                  int limit)
{
    Q_D(SpellChecker);

    if (not enabled()) {
        return QStringList();
    }

    char** suggestions = NULL;
    const int suggestions_count = d->hunspell->suggest(&suggestions, d->codec->fromUnicode(word).toStdString().c_str());

    // Less than zero means some error.
    if (suggestions_count < 0) {
        qWarning() << __PRETTY_FUNCTION__ << ": Failed to get suggestions for" << word << ".";
        return QStringList();
    }

    QStringList result;
    const int final_limit((limit < 0) ? suggestions_count : qMin(limit, suggestions_count));

    for (int index(0); index < final_limit; ++index) {
        result << d->codec->toUnicode(suggestions[index]);
    }
    d->hunspell->free_list(&suggestions, suggestions_count);
    return result;
}


//! \brief Marks a given word as ignored.
//! \param word The word to ignore - it will not be checked for spelling.
void SpellChecker::ignoreWord(const QString &word)
{
    Q_D(SpellChecker);

    if (not enabled()) {
        return;
    }

    d->ignored_words.insert(word);
}

//! \brief Adds a given word to user dictionary.
//! \param word The word to be added to user dictionary - it will be used for
//!             spellchecking and suggesting.
void SpellChecker::addToUserWordlist(const QString &word)
{
    Q_D(SpellChecker);

    if (not enabled()) {
        return;
    }

    // db8 is the one place this list lives now - org.webosports.app.settings'
    // Text Assist page reads and writes the very same kind, so a word added
    // from either place shows up in both. Applied to the live Hunspell
    // instance immediately rather than waiting on the round trip; the
    // subscription in the constructor will reconfirm it a moment later.
    if (!d->userWords.contains(word)) {
        d->userWords.append(word);
        if (d->hunspell)
            d->hunspell->add(d->codec->fromUnicode(word).toStdString());
    }

    LSError error;
    LSErrorInit(&error);
    QByteArray payload = "{\"objects\":[{\"_kind\":\"" + d->user_dictionary_kind.toUtf8() +
                         "\",\"word\":\"" + word.toUtf8() + "\"}]}";
    if (!LSCall(d->serviceHandle, "luna://com.palm.db/put", payload.constData(),
              SpellChecker::putCallback, this, NULL, &error)) {
        qWarning("Failed to add '%s' to user dictionary: %s", qPrintable(word), error.message);
        LSErrorFree(&error);
    }
}

//! \brief SpellChecker::setLanguage switches to the given language if possible
//! \param language The new language use "en" or "en_US". If more than one
//! exists, the first one in the directory listing is used
//! \return true if switching the language succeded
bool SpellChecker::setLanguage(const QString &language)
{
    Q_D(SpellChecker);

    qDebug() << "spellechecker.cpp in setLanguage() lang=" << language << "dictPath=" << dictPath();

    QDir dictDir(dictPath());
    QStringList affMatches = dictDir.entryList(QStringList(language+"*.aff"));
    QStringList dicMatches = dictDir.entryList(QStringList(language+"*.dic"));

    if (affMatches.isEmpty() || dicMatches.isEmpty()) {
        QString lang = language;
        lang.truncate(2);
        qWarning() << "Did not find a dictionary for" << language << " - checking for " << lang;
        if (language.length() > 2) {
            return setLanguage(lang);
        }

        qWarning() << "No dictionary found for" << language << "turning off spellchecking";
        d->clear();
        return false;
    }

    d->aff_file = dictPath() + "/" + affMatches[0];
    d->dic_file = dictPath() + "/" + dicMatches[0];

    qDebug() << "spellechecker.cpp in setLanguage() aff_file=" << d->aff_file << "dic_file=" << d->dic_file;

    if (enabled()) {
        setEnabled(false);
        return setEnabled(true);
    } else {
        return true;
    }
}

// static
QString SpellChecker::dictPath()
{
    return QString(HUNSPELL_DICT_PATH);
}
