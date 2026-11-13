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

#include "abstractwordengine.h"

#include <algorithm>

namespace MaliitKeyboard {
namespace Logic {

//! \class AbstractWordEngine
//! \brief Provides word candidates based on text model.
//!
//! Derived classes need to provide an implementation for
//! fetchCandidates() and, optionally, addToUserDictionary().
//! \sa Model::Text, computeCandidates().

//! \fn void AbstractWordEngine::enabledChanged(bool enabled)
//! \brief Emitted when word engine toggles word candidate updates on/off.
//! \param enabled Whether word engine is enabled.

//! \fn void AbstractWordEngine::candidatesChanged(const WordCandidateList &candidates)
//! \brief Emitted when new candidates have been computed.
//! \param candidates The list of updated candidates.

//! \fn WordCandidateList AbstractWordEngine::fetchCandidates(Model::Text *text)
//! \brief Returns a list of candidates.
//! \param text The text model.
//!
//! Needs to be implemented by derived classes. Will not be called if engine
//! is disabled or text model has no preedit.

//! \property AbstractWordEngine::enabled
//! \brief Whether the engine provides updates for word candidates.

class AbstractWordEnginePrivate
{
public:
    bool enabled;
    //! The blocked words, in lower case.
    QSet<QString> blocked;

    explicit AbstractWordEnginePrivate();
};

AbstractWordEnginePrivate::AbstractWordEnginePrivate()
    : enabled(false)
{}


//! \brief Constructor.
//! \param parent The owner of this instance. Can be 0, in case QObject
//!               ownership is not required.
AbstractWordEngine::AbstractWordEngine(QObject *parent)
    : QObject(parent)
    , d_ptr(new AbstractWordEnginePrivate)
{}

//! \brief Destructor.
//!
//! Needs to be implemented in derived classes.
AbstractWordEngine::~AbstractWordEngine()
{}


//! \brief Returns whether the word engine is enabled.
//! \sa AbstractWordEngine::enabled
bool AbstractWordEngine::isEnabled() const
{
    Q_D(const AbstractWordEngine);
    return d->enabled;
}


//! \brief Set whether the engine should be enabled.
//! \param enabled Setting to true will be ignored if there's no word
//!                prediction or error correction backend available.
//! \sa AbstractWordEngine::enabled
void AbstractWordEngine::setEnabled(bool enabled)
{
    Q_D(AbstractWordEngine);

    if (d->enabled != enabled) {
        clearCandidates();
        d->enabled = enabled;
        Q_EMIT enabledChanged(isEnabled());
    }
}


//! \brief Clears the current candidates.
//!
//! Only has an effect when word engine is enabled, in which case
//! candidatesCanged() is emitted.
void AbstractWordEngine::clearCandidates()
{
    if (isEnabled()) {
        Q_EMIT candidatesChanged(WordCandidateList());
    }
}


//! \brief Computes new candidates, based on text model.
//! \param text The text model.
//!
//! Can trigger emission of candidatesChanged().
void AbstractWordEngine::computeCandidates(Model::Text *text)
{
    // FIXME: add possiblity to turn off the error correction for
    // entries that does not need it (like password entries).  Also,
    // with that we probably will want to turn off preedit styling at
    // all.

    // A word backspaced away: its candidates go too, rather than staying up
    // for a word that is no longer there.
    if (text && text->preedit().isEmpty()) {
        clearCandidates();
        return;
    }

    if (!isEnabled()
        || !text
        || !text->preedit().at(text->preedit().length() - 1).isLetterOrNumber()) {
        // FIXME: We should here set some special preedit face in text
        // model (say Disabled or None) which would be interpreted by
        // editor to send no formatting informations along with
        // preedit string. When this is done, preedit-string test
        // needs to be adapted.
        return;
    }

    WordCandidateList candidates = fetchCandidates(text);
    Q_D(AbstractWordEngine);
    if (!d->blocked.isEmpty()) {
        // A deleted word is neither offered nor what space puts in.
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                        [this](const WordCandidate &c) { return isBlocked(c.word()); }),
                         candidates.end());
        if (isBlocked(text->primaryCandidate()))
            text->setPrimaryCandidate(candidates.isEmpty() ? QString() : candidates.first().label());
    }
    // A word of the owner's typed in another case is theirs as they saved it
    // ("webos" is "webOS"): first, and what space puts in.
    const QString preedit = text->preedit();
    for (const QString &own : userWords()) {
        if (own == preedit || own.compare(preedit, Qt::CaseInsensitive) != 0 || isBlocked(own))
            continue;
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                        [&own](const WordCandidate &c) {
                                            return c.word().compare(own, Qt::CaseInsensitive) == 0;
                                        }),
                         candidates.end());
        candidates.prepend(WordCandidate(WordCandidate::SourceSpellChecking, own));
        text->setPrimaryCandidate(own);
        break;
    }
    Q_EMIT candidatesChanged(candidates);
}

QStringList AbstractWordEngine::userWords()
{
    return QStringList();
}

QStringList AbstractWordEngine::nextWords(const QString &left, int limit)
{
    QStringList out;
    if (!isEnabled()) return out;
    for (const QString &w : predictNext(left)) {
        if (out.size() >= limit) break;
        if (!w.isEmpty() && !isBlocked(w) && !out.contains(w)) out << w;
    }
    return out;
}

QStringList AbstractWordEngine::predictNext(const QString &left)
{
    Q_UNUSED(left);
    return QStringList();
}

void AbstractWordEngine::setBlockedWords(const QStringList &words)
{
    Q_D(AbstractWordEngine);
    d->blocked.clear();
    for (const QString &w : words)
        if (!w.isEmpty()) d->blocked.insert(w.toLower());
}

bool AbstractWordEngine::isBlocked(const QString &word) const
{
    Q_D(const AbstractWordEngine);
    return !word.isEmpty() && d->blocked.contains(word.toLower());
}

//! \brief Adds a word to user dictionary.
//! \param word A word.
//!
//! Needs to be implemented in derived classes. This does nothing.
void AbstractWordEngine::addToUserDictionary(const QString &word)
{
    Q_UNUSED(word);
}

void AbstractWordEngine::removeFromUserDictionary(const QString &word)
{
    Q_UNUSED(word);
}

bool AbstractWordEngine::isMisspelled(const QString &word)
{
    Q_UNUSED(word);
    return false;
}

bool AbstractWordEngine::marksMisspellings() const
{
    return false;
}

QStringList AbstractWordEngine::spellingSuggestions(const QString &word, int limit)
{
    Q_UNUSED(word);
    Q_UNUSED(limit);
    return QStringList();
}

//!
void AbstractWordEngine::setWordPredictionEnabled(bool on)
{
    Q_UNUSED(on);
    qDebug() << Q_FUNC_INFO << "should be implemented by inherited class";
}

//!
void AbstractWordEngine::setSpellcheckerEnabled(bool on)
{
    Q_UNUSED(on);
    qDebug() << Q_FUNC_INFO << "should be implemented by inherited class";
}
//!
void AbstractWordEngine::setAutoCorrectEnabled(bool on)
{
    Q_UNUSED(on);
}

/*
AbstractLanguageFeature* AbstractWordEngine::languageFeature()
{
    return new AbstractLanguageFeature;
}
*/
}} // namespace MaliitKeyboard, Logic
