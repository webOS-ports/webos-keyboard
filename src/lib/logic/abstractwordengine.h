/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 Openismus GmbH
 *
 * Contact: maliit-discuss@lists.maliit.org
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

#ifndef MALIIT_KEYBOARD_ABSTRACTWORDENGINE_H
#define MALIIT_KEYBOARD_ABSTRACTWORDENGINE_H

#include "models/text.h"
#include "models/wordcandidate.h"
#include <QtCore>

class AbstractLanguageFeatures;

namespace MaliitKeyboard {
namespace Logic {

class AbstractWordEnginePrivate;

class AbstractWordEngine
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractWordEngine)
    Q_DECLARE_PRIVATE(AbstractWordEngine)

    Q_PROPERTY(bool enabled READ isEnabled
                            WRITE setEnabled
                            NOTIFY enabledChanged)

public:
    explicit AbstractWordEngine(QObject *parent = nullptr);
    ~AbstractWordEngine() override;

    virtual bool isEnabled() const;
    Q_SLOT virtual void setEnabled(bool enabled);
    Q_SIGNAL void enabledChanged(bool enabled);

    Q_SLOT virtual void setWordPredictionEnabled(bool on);
    //! The "Spell checking" setting: a word spelt wrong is marked and offered to be added.
    Q_SLOT virtual void setSpellcheckerEnabled(bool on);
    //! The "Autocorrect" setting, which needs to know a word is wrong as much.
    Q_SLOT virtual void setAutoCorrectEnabled(bool on);

    void clearCandidates();
    void computeCandidates(Model::Text *text);
    Q_SIGNAL void candidatesChanged(const WordCandidateList &candidates);

    virtual void addToUserDictionary(const QString &word);
    //! \brief Takes \a word out of the owner's dictionary, if it is there. This does nothing.
    virtual void removeFromUserDictionary(const QString &word);

    /*!
     * \brief Words never to offer or correct to, whatever the dictionaries
     * say: the ones the owner deleted (the Pre Keyboard's "removed"), for the
     * language typed in. Matched without regard to case.
     */
    void setBlockedWords(const QStringList &words);
    bool isBlocked(const QString &word) const;

    //! \brief The owner's own words, as saved: one typed in another case
    //! ("webos" for "webOS") is put in as saved. None here.
    virtual QStringList userWords();

    //! \brief The likeliest next words after \a left, before any of the next is
    //! typed: at most \a limit, best first, none the owner deleted. None here.
    QStringList nextWords(const QString &left, int limit);
protected:
    virtual QStringList predictNext(const QString &left);
public:

    //! \brief Whether \a word is spelled wrongly. False when nothing can say.
    //! Known with autocorrect on alone too; whether to show it is marksMisspellings().
    virtual bool isMisspelled(const QString &word);

    //! \brief Whether a word spelt wrong is to be marked, and offered to be
    //! added to the owner's words: the "Spell checking" setting. None here.
    virtual bool marksMisspellings() const;

    //! \brief What could replace \a word, best first, at most \a limit of them.
    virtual QStringList spellingSuggestions(const QString &word, int limit);

    virtual AbstractLanguageFeatures* languageFeature() = 0;

private:
    virtual WordCandidateList fetchCandidates(Model::Text *text) = 0;
    const QScopedPointer<AbstractWordEnginePrivate> d_ptr;
};

}} // namespace MaliitKeyboard, Logic

#endif // MALIIT_KEYBOARD_ABSTRACTWORDENGINE_H
