/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PRE_DICTATION_H
#define PRE_DICTATION_H

#include <QObject>
#include <QString>

namespace Pre {

/*!
 * Speech to text, for the period's microphone: listening starts, the words
 * heard so far come in partial() as they are spoken, and each phrase in
 * final() once it is finished. Listening goes on after a phrase, so a long
 * message can be spoken in one go, until it is stopped or the speaker has
 * been quiet a while; then stopped().
 */
class Dictation : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    //! There is a speech model for [language] (a keyboard language id: "en", "de").
    virtual bool available(const QString &language) const = 0;
    //! Listens for [language]. Nothing happens while already listening.
    virtual void start(const QString &language) = 0;
    //! Stops listening; what was being said still comes in final() before stopped().
    virtual void stop() = 0;
    //! Stops listening at once: nothing more comes but stopped().
    virtual void cancel() = 0;
    virtual bool listening() const = 0;

Q_SIGNALS:
    //! The words heard so far of what is being said; "" when there are none any more.
    void partial(const QString &text);
    //! A phrase, finished.
    void final(const QString &text);
    //! Listening ended; [problem] says why when it wasn't asked to stop or the speaker went quiet.
    void stopped(const QString &problem);
};

namespace Speech {

/*!
 * The model for keyboard language [language] among the models in [root], one
 * folder each, named by language tag ("en-US", "de"): the folder named for the
 * language itself, else the first, by name, of its language ("en" ->
 * "en-US"). Empty when there is none. A folder is a model when it has
 * an acoustic model (am/final.mdl).
 */
QString modelFor(const QString &root, const QString &language);

//! The text in a recogniser's result, [key] of its JSON ("text", "partial"); empty when there is none.
QString resultText(const QByteArray &json, const char *key);

/*!
 * [spoken] as it goes into the text after [before]: trimmed, a space in front
 * when it follows a word, a capital first when [capital], and in English "i"
 * and its contractions as "I".
 */
QString join(const QString &spoken, const QString &before, bool capital, const QString &language);

} // namespace Speech

} // namespace Pre

#endif // PRE_DICTATION_H
