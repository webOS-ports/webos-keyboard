/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
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

#ifndef PRE_SETTINGS_H
#define PRE_SETTINGS_H

#include "prelanguage.h"
#include "prepainter.h"

#include <QJsonObject>
#include <QList>
#include <QStringList>

namespace Pre {

/*!
 * The owner's choices, from the "pre" object of the system "keyboard"
 * preference, which Text Assist writes. The keyboard follows a change at once,
 * with the keyboard still on screen.
 *
 *   "keyboard": { "keyboardLayout": "Pre Keyboard",
 *                 "pre": { "style": "orange", "height": "medium", "autocorrect": true, ...,
 *                          "languages": ["en_US", "fr"], "language": "en_US",
 *                          "learned": { "en": ["luneos"] }, "recentEmoji": ["🙂"] } }
 */
struct Settings
{
    enum class Height { Small, Medium, Large };

    Style style = Style::Orange;
    Height height = Height::Medium;
    bool autocorrect = true;
    bool doubleSpacePeriod = true;
    bool suggestions = false;
    bool swipe = false;
    //! Press and hold a key for its extra characters (accents, more symbols), each such key dotted.
    bool holdForMore = false;
    //! An emoji for the word in the suggestion bar's right-hand place, when there is one.
    bool emojiSuggestions = false;
    //! A light vibration as each key goes down.
    bool vibrate = false;
    //! A click as each key goes down.
    bool sound = false;
    //! The languages checked in Text Assist, in its order: a flick on the space bar goes through them.
    QList<Language> languages = { Language::enUS() };
    //! The one typed in now, always one of [languages].
    Language language = Language::enUS();

    //! Swiping and emoji suggestions need the bar too: it is where they show.
    bool showsBar() const { return suggestions || swipe || emojiSuggestions; }
    //! Row heights in dp. Medium is close to a Pre key's proportions on a modern phone.
    int heightDp() const { return height == Height::Small ? 46 : height == Height::Large ? 62 : 54; }

    static Settings fromJson(const QJsonObject &pre);
    //! The choices into [pre], leaving what isn't a choice (words, emoji) as it was.
    QJsonObject toJson(QJsonObject pre) const;

    bool operator==(const Settings &o) const;
    bool operator!=(const Settings &o) const { return !(*this == o); }

    //! Where a language's family keeps the owner's words: "en", "fr".
    static QStringList learned(const QJsonObject &pre, const Language &lang);
    static QJsonObject withLearned(QJsonObject pre, const Language &lang, const QStringList &words);
    static QStringList recentEmoji(const QJsonObject &pre);
    static QJsonObject withRecentEmoji(QJsonObject pre, const QString &emoji);
    static const int RECENT_MAX = 32;
};

} // namespace Pre

#endif // PRE_SETTINGS_H
