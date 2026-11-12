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
#include "hunspelldictionary.h"

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

class SpellCheckerPrivate
{
public:
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

    // Both are owned by this SpellChecker and must be released with it: the
    // poll timer holds a bare "this", and the loop is what LSGmainAttach was
    // handed. A language switch unloads this whole plugin (see
    // WordEnginePrivate::loadPlugin), so anything left registered with glib
    // afterwards fires into freed memory in an unmapped library.
    GMainLoop *mainLoop; //!< The loop the service handle is attached to.
    guint pollSourceId;  //!< The db8 poll timer, 0 when not registered.

    QStringList userWords;
    //! The user words the live Hunspell instance has been given, to take back
    //! out the ones that leave db8 (deleted in Text Assist, or from the keys).
    QStringList appliedWords;

    explicit SpellCheckerPrivate(const QString &user_dictionary_kind);
    ~SpellCheckerPrivate();
    void applyUserWords();
    void clear();
};


SpellCheckerPrivate::SpellCheckerPrivate(const QString &user_dictionary_kind)
    : hunspell(nullptr)
    , codec(nullptr)
    , ignored_words()
    , user_dictionary_kind(user_dictionary_kind)
    , aff_file()
    , dic_file()
    , serviceHandle(nullptr)
    , mainLoop(nullptr)
    , pollSourceId(0)
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
        serviceHandle = nullptr;
    }

    // After LSUnregister, so the handle is detached before the loop it was
    // attached to goes away.
    if (mainLoop) {
        g_main_loop_unref(mainLoop);
        mainLoop = nullptr;
    }
}

//! \brief SpellCheckerPrivate::applyUserWords adds the cached db8 word list
//! to whichever Hunspell instance is currently active.
void SpellCheckerPrivate::applyUserWords()
{
    if (!hunspell)
        return;

    for (const QString &word : appliedWords)
        if (!userWords.contains(word))
            hunspell->remove(codec->fromUnicode(word).toStdString());
    for (const QString &word : userWords)
        hunspell->add(codec->fromUnicode(word).toStdString());
    appliedWords = userWords;
}

//! \brief SpellCheckerPrivate::clear cleans up all memory and does reset
//! everything for a new language
void SpellCheckerPrivate::clear()
{
    delete(hunspell);
    hunspell = nullptr;
    appliedWords.clear();
    aff_file.clear();
    dic_file.clear();
}

SpellChecker::~SpellChecker()
{
    Q_D(SpellChecker);

    // Must happen before ~SpellCheckerPrivate: pollCallback casts its
    // user_data straight back to this object.
    if (d->pollSourceId != 0) {
        g_source_remove(d->pollSourceId);
        d->pollSourceId = 0;
    }
}

//! \brief SpellChecker::enabled returns if the spechchecking is active
//! \return
bool SpellChecker::enabled() const
{
    Q_D(const SpellChecker);
    return (d->hunspell != nullptr);
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
    d->hunspell = nullptr;

    if (!on) {
        return true;
    }

    if (d->aff_file.isEmpty() || d->dic_file.isEmpty()) {
        qWarning() << "no dictionary to turn on spellchecking";
        return false;
    }

    d->hunspell = new Hunspell(d->aff_file.toUtf8().constData(),
                               d->dic_file.toUtf8().constData());

    d->codec = QTextCodec::codecForName(d->hunspell->get_dic_encoding());
    if (!d->codec) {
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

    d->mainLoop = g_main_loop_new(g_main_context_default(), TRUE);

    if (!LSGmainAttach(d->serviceHandle, d->mainLoop, &error)) {
        qWarning("Failed to attach to glib mainloop: %s", error.message);
        LSErrorFree(&error);
        return;
    }

    // The actual freshness mechanism - see the header comment for why this
    // exists instead of relying on watch alone. Registered exactly once,
    // for the life of this SpellChecker, and removed again in ~SpellChecker:
    // it holds a bare "this", and glib knows nothing about our lifetime.
    d->pollSourceId = g_timeout_add_seconds(POLL_INTERVAL_SECONDS,
                                            SpellChecker::pollCallback, this);

    findWordsAndWatch();
}

// Registers ONE long-lived find, with "watch":true alongside "query" (the
// db8 API's actual live-notification flag - see the header comment). Call
// this exactly once: the subscription it creates keeps delivering "fired"
// notifications on its own for as long as it stays open, so it must never
// be re-registered from inside findCallback.
void SpellChecker::findWordsAndWatch()
{
    Q_D(SpellChecker);

    LSError error;
    LSErrorInit(&error);

    const QByteArray payload = "{\"query\":{\"from\":\"" + d->user_dictionary_kind.toUtf8() +
                         "\",\"orderBy\":\"word\"},\"watch\":true}";

    if (!LSCall(d->serviceHandle, "luna://com.palm.db/find", payload.constData(),
              SpellChecker::findCallback, this, nullptr, &error)) {
        qWarning("Loading user dictionary failed: %s", error.message);
        LSErrorFree(&error);
    }
}

// One-shot refresh, no watch - used to pick up what changed once the
// standing subscription above reports a fired notification.
void SpellChecker::refreshWords()
{
    Q_D(SpellChecker);

    LSError error;
    LSErrorInit(&error);

    const QByteArray payload = "{\"query\":{\"from\":\"" +
                         d->user_dictionary_kind.toUtf8() + "\",\"orderBy\":\"word\"}}";

    if (!LSCall(d->serviceHandle, "luna://com.palm.db/find", payload.constData(),
              SpellChecker::findCallback, this, nullptr, &error)) {
        qWarning("Refreshing user dictionary failed: %s", error.message);
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

    const QJsonDocument document = QJsonDocument::fromJson(QByteArray(payload));
    const QJsonObject root = document.object();

    if (root.contains("results") && root.value("results").isArray()) {
        const QJsonArray results = root.value("results").toArray();
        QStringList words;
        for (const QJsonValue entry : results) {
            const QJsonObject obj = entry.toObject();
            if (obj.contains("word") && obj.value("word").isString())
                words.append(obj.value("word").toString());
        }

        SpellChecker *self = static_cast<SpellChecker*>(user_data);
        self->d_func()->userWords = words;
        self->d_func()->applyUserWords();
    }

    // The find+watch subscription's later replies carry "fired":true when
    // the result set changes, and may or may not repeat "results" - fetch
    // it explicitly with a plain one-shot find rather than relying on
    // that. This does NOT re-register the watch: the original find+watch
    // call stays open and keeps delivering future fired notifications by
    // itself. (Re-registering here was an infinite-loop bug - see
    // findWordsAndWatch()'s comment.)
    if (root.value("fired").toBool())
        static_cast<SpellChecker*>(user_data)->refreshWords();

    return true;
}

// Fires on a fixed POLL_INTERVAL_SECONDS cadence for the life of this
// SpellChecker - see the header comment for why this, and not watch alone,
// is what actually keeps userWords current. Returns TRUE unconditionally
// so glib keeps calling it; it cannot loop tighter than its own fixed
// interval, unlike re-arming a watch from its own handler.
gboolean SpellChecker::pollCallback(gpointer user_data)
{
    static_cast<SpellChecker*>(user_data)->refreshWords();
    return G_SOURCE_CONTINUE;
}

bool SpellChecker::putCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    Q_UNUSED(handle);
    Q_UNUSED(user_data);

    if (!message)
        return true;

    const char *payload = LSMessageGetPayload(message);
    if (payload) {
        const QJsonObject root = QJsonDocument::fromJson(QByteArray(payload)).object();
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

    if (!enabled() || d->ignored_words.contains(word)) {
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

    if (!enabled()) {
        return QStringList();
    }

    char **suggestions = nullptr;
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

    if (!enabled()) {
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

    if (!enabled()) {
        return;
    }

    // db8 is the one place this list lives now - org.webosports.app.settings'
    // Text Assist page reads and writes the very same kind, so a word added
    // from either place shows up in both. Applied to the live Hunspell
    // instance immediately rather than waiting on the round trip; the
    // subscription in the constructor will reconfirm it a moment later.
    if (!d->userWords.contains(word)) {
        d->userWords.append(word);
        if (d->hunspell) {
            d->hunspell->add(d->codec->fromUnicode(word).toStdString());
            d->appliedWords.append(word);
        }
    }

    LSError error;
    LSErrorInit(&error);
    // Built as JSON rather than pasted in: a word with a quote or a backslash
    // in it made the put unparseable.
    const QJsonObject object{ { QStringLiteral("_kind"), d->user_dictionary_kind },
                              { QStringLiteral("word"), word } };
    const QByteArray payload = QJsonDocument(QJsonObject{ { QStringLiteral("objects"), QJsonArray{ object } } })
                                   .toJson(QJsonDocument::Compact);
    if (!LSCall(d->serviceHandle, "luna://com.palm.db/put", payload.constData(),
              SpellChecker::putCallback, this, nullptr, &error)) {
        qWarning("Failed to add '%s' to user dictionary: %s", qPrintable(word), error.message);
        LSErrorFree(&error);
    }
}

//! \brief Takes a word out of the user dictionary, if it is there: out of
//! the live Hunspell instance at once, and out of db8, where Text Assist and
//! presage see it go too.
void SpellChecker::removeFromUserWordlist(const QString &word)
{
    Q_D(SpellChecker);

    if (!d->userWords.contains(word))
        return;

    d->userWords.removeAll(word);
    if (d->hunspell && d->appliedWords.contains(word)) {
        d->hunspell->remove(d->codec->fromUnicode(word).toStdString());
        d->appliedWords.removeAll(word);
    }

    const QJsonObject query{
        { QStringLiteral("from"), d->user_dictionary_kind },
        { QStringLiteral("where"), QJsonArray{ QJsonObject{ { QStringLiteral("prop"), QStringLiteral("word") },
                                                            { QStringLiteral("op"), QStringLiteral("=") },
                                                            { QStringLiteral("val"), word } } } } };
    const QByteArray payload = QJsonDocument(QJsonObject{ { QStringLiteral("query"), query } })
                                   .toJson(QJsonDocument::Compact);
    LSError error;
    LSErrorInit(&error);
    if (!LSCall(d->serviceHandle, "luna://com.palm.db/del", payload.constData(),
                SpellChecker::delCallback, this, nullptr, &error)) {
        qWarning("Failed to take '%s' out of the user dictionary: %s", qPrintable(word), error.message);
        LSErrorFree(&error);
    }
}

QStringList SpellChecker::userWords() const
{
    Q_D(const SpellChecker);
    return d->userWords;
}

bool SpellChecker::delCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    Q_UNUSED(handle);
    Q_UNUSED(user_data);

    const char *payload = message ? LSMessageGetPayload(message) : nullptr;
    if (payload && !QJsonDocument::fromJson(QByteArray(payload)).object().value(QStringLiteral("returnValue")).toBool())
        qWarning() << "com.palm.db/del failed:" << payload;
    return true;
}

//! \brief SpellChecker::setLanguage switches to the given language if possible
//! \param language The keyboard's language ("en"), or it and the country it is
//! typed in ("en_NL"): see hunspellDictionary() for which dictionary that is.
//! \return true if switching the language succeded
bool SpellChecker::setLanguage(const QString &language)
{
    Q_D(SpellChecker);

    const int sep = language.indexOf(QLatin1Char('_'));
    const QString id = sep < 0 ? language : language.left(sep);
    const QString country = sep < 0 ? QString() : language.mid(sep + 1);

    const QDir dictDir(dictPath());
    const QString name = MaliitKeyboard::Logic::hunspellDictionary(
        id, country, MaliitKeyboard::Logic::hunspellDictionaries(dictDir.entryList(QDir::Files)));

    if (name.isEmpty()) {
        qWarning() << "No dictionary found for" << language << "turning off spellchecking";
        d->clear();
        return false;
    }

    d->aff_file = dictDir.filePath(name + QStringLiteral(".aff"));
    d->dic_file = dictDir.filePath(name + QStringLiteral(".dic"));

    qInfo("spellchecker: %s is spelled with %s", qPrintable(language), qPrintable(name));

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
