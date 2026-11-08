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

#include "presettings.h"

#include <QJsonArray>

namespace Pre {
namespace {

bool flag(const QJsonObject &o, const char *key, bool fallback)
{
    const QJsonValue v = o.value(QLatin1String(key));
    return v.isBool() ? v.toBool() : fallback;
}

QString familyOf(const Language &lang)
{
    return lang.family();
}

} // namespace

Settings Settings::fromJson(const QJsonObject &pre)
{
    Settings s;
    s.style = pre.value(QLatin1String("style")).toString() == QLatin1String("white") ? Style::White : Style::Orange;
    const QString h = pre.value(QLatin1String("height")).toString();
    s.height = h == QLatin1String("small") ? Height::Small : h == QLatin1String("large") ? Height::Large : Height::Medium;
    s.autocorrect = flag(pre, "autocorrect", s.autocorrect);
    s.doubleSpacePeriod = flag(pre, "doubleSpacePeriod", s.doubleSpacePeriod);
    s.suggestions = flag(pre, "suggestions", s.suggestions);
    s.swipe = flag(pre, "swipe", s.swipe);
    s.holdForMore = flag(pre, "holdForMore", s.holdForMore);
    s.emojiSuggestions = flag(pre, "emojiSuggestions", s.emojiSuggestions);
    s.vibrate = flag(pre, "vibrate", s.vibrate);
    s.sound = flag(pre, "sound", s.sound);
    // Text Assist's order, whatever order they were checked in; never none.
    QStringList ids;
    for (const QJsonValue &v : pre.value(QLatin1String("languages")).toArray())
        ids << v.toString();
    QList<Language> langs;
    for (const Language &l : Language::all())
        if (ids.contains(l.id))
            langs << l;
    if (!langs.isEmpty())
        s.languages = langs;
    const Language now = Language::byId(pre.value(QLatin1String("language")).toString());
    s.language = (now.isValid() && s.languages.contains(now)) ? now : s.languages.first();
    return s;
}

QJsonObject Settings::toJson(QJsonObject pre) const
{
    pre.insert(QLatin1String("style"), style == Style::White ? QLatin1String("white") : QLatin1String("orange"));
    pre.insert(QLatin1String("height"), height == Height::Small ? QLatin1String("small")
               : height == Height::Large ? QLatin1String("large") : QLatin1String("medium"));
    pre.insert(QLatin1String("autocorrect"), autocorrect);
    pre.insert(QLatin1String("doubleSpacePeriod"), doubleSpacePeriod);
    pre.insert(QLatin1String("suggestions"), suggestions);
    pre.insert(QLatin1String("swipe"), swipe);
    pre.insert(QLatin1String("holdForMore"), holdForMore);
    pre.insert(QLatin1String("emojiSuggestions"), emojiSuggestions);
    pre.insert(QLatin1String("vibrate"), vibrate);
    pre.insert(QLatin1String("sound"), sound);
    QJsonArray ids;
    for (const Language &l : languages)
        ids.append(l.id);
    pre.insert(QLatin1String("languages"), ids);
    pre.insert(QLatin1String("language"), language.id);
    return pre;
}

bool Settings::operator==(const Settings &o) const
{
    return style == o.style && height == o.height && autocorrect == o.autocorrect &&
           doubleSpacePeriod == o.doubleSpacePeriod && suggestions == o.suggestions && swipe == o.swipe &&
           holdForMore == o.holdForMore && emojiSuggestions == o.emojiSuggestions && vibrate == o.vibrate &&
           sound == o.sound && languages == o.languages && language == o.language;
}

QStringList Settings::learned(const QJsonObject &pre, const Language &lang)
{
    QStringList out;
    for (const QJsonValue &v : pre.value(QLatin1String("learned")).toObject().value(familyOf(lang)).toArray())
        out << v.toString();
    return out;
}

QJsonObject Settings::withLearned(QJsonObject pre, const Language &lang, const QStringList &words)
{
    QJsonObject learned = pre.value(QLatin1String("learned")).toObject();
    learned.insert(familyOf(lang), QJsonArray::fromStringList(words));
    pre.insert(QLatin1String("learned"), learned);
    return pre;
}

QStringList Settings::recentEmoji(const QJsonObject &pre)
{
    QStringList out;
    for (const QJsonValue &v : pre.value(QLatin1String("recentEmoji")).toArray())
        out << v.toString();
    return out;
}

QJsonObject Settings::withRecentEmoji(QJsonObject pre, const QString &emoji)
{
    QStringList list = recentEmoji(pre);
    list.removeAll(emoji);
    list.prepend(emoji);
    pre.insert(QLatin1String("recentEmoji"), QJsonArray::fromStringList(list.mid(0, RECENT_MAX)));
    return pre;
}

} // namespace Pre
