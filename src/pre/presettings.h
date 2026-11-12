/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
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

#ifndef PRE_SETTINGS_H
#define PRE_SETTINGS_H

#include "prelanguage.h"
#include "prepainter.h"

#include <QJsonObject>
#include <QList>
#include <QStringList>

namespace Pre {

/*!
 * What the Pre Keyboard follows. Its look is its own, in the "pre" object of the
 * system "keyboard" preference (Text Assist writes it, and the keys switch the
 * style and Hold for More):
 *
 *   "keyboard": { ..., "pre": { "style": "orange", "height": "medium",
 *                               "holdForMore": false, "vibrate": false,
 *                               "followShift": false, "swipe": true,
 *                               "doubleSpacePeriod": true, "emojiSuggestions": false,
 *                               "recentEmoji": ["🙂"] } }
 *
 * Everything about words - suggestions, autocorrect, the owner's dictionary -
 * and the languages and the key click are the LuneOS keyboard's own settings,
 * which the controller copies in (Controller::setKeyboardState).
 */
struct Settings
{
    enum class Height { Small, Medium, Large };

    // From the "pre" object.
    Style style = Style::Orange;
    Height height = Height::Medium;
    //! Press and hold a key for its extra characters (accents, more symbols), each such key dotted.
    bool holdForMore = false;
    //! A vibration as each key goes down...
    bool vibrate = false;
    //! ...this strong: 1 (Light, the tap it always was), 2 (Firmer) or 3 (Strong).
    int vibrateLevel = 1;
    static constexpr int VIBRATE_LEVELS = 3;
    /*!
     * A key's buzz at the chosen strength, in ms. LuneOS's vibrator
     * (com.palm.vibrate) takes a length but no strength, so a firmer buzz is
     * a longer one.
     */
    int keyBuzzMs() const;
    //! The letters on the keys follow Shift (small till it is on); off, they are capitals, as printed on the Pre.
    bool followShift = false;
    /*!
     * Swipe to type: a finger drawn across the letters types the word they
     * spell. The controller turns it off for a language with no words to
     * swipe (Chinese, whose words are not its keys' letters) and till a
     * language's words are loaded.
     */
    bool swipe = true;
    //! Two quick spaces after a word type ". ".
    bool doubleSpacePeriod = true;
    //! An emoji for the word in the bar's right-hand place ("pizza" -> 🍕), for a language that has them.
    bool emojiSuggestions = false;

    // From the LuneOS keyboard's settings.
    //! Prediction or spell checking is on: the suggestion bar shows.
    bool suggestions = false;
    //! The key click (keyPressFeedback).
    bool sound = false;
    //! The languages enabled under Regional Settings, in their order: a flick on the space bar goes through them.
    QList<Language> languages;
    //! The one typed in now, always one of [languages].
    Language language;

    bool showsBar() const { return suggestions || swipe || emojiSuggestions; }
    //! Row heights in dp. Medium is close to a Pre key's proportions on a modern phone.
    int heightDp() const { return height == Height::Small ? 46 : height == Height::Large ? 62 : 54; }

    //! [into] with the "pre" object's choices read over it.
    static Settings fromJson(const QJsonObject &pre, Settings into);
    static Settings fromJson(const QJsonObject &pre);
    //! The choices into [pre], leaving what isn't a choice (recent emoji) as it was.
    QJsonObject toJson(QJsonObject pre) const;

    bool operator==(const Settings &o) const;
    bool operator!=(const Settings &o) const { return !(*this == o); }

    /*!
     * The words the owner deleted in language [id] (held in the bar, "Delete
     * from dictionary?"): never offered, nor corrected to. Kept in the "pre"
     * object as "removed": { <language>: [words] }, the Android Pre Keyboard's
     * shape, for Text Assist to put back.
     */
    static QStringList removed(const QJsonObject &pre, const QString &id);
    static QJsonObject withRemoved(QJsonObject pre, const QString &id, const QStringList &words);

    static QStringList recentEmoji(const QJsonObject &pre);
    static QJsonObject withRecentEmoji(QJsonObject pre, const QString &emoji);
    static constexpr int RECENT_MAX = 32;
};

} // namespace Pre

#endif // PRE_SETTINGS_H
