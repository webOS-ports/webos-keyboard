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

} // namespace

Settings Settings::fromJson(const QJsonObject &pre, Settings into)
{
    Settings &s = into;
    const QString style = pre.value(QLatin1String("style")).toString();
    s.style = style == QLatin1String("white") ? Style::White : Style::Orange;
    const QString h = pre.value(QLatin1String("height")).toString();
    s.height = h == QLatin1String("small") ? Height::Small : h == QLatin1String("large") ? Height::Large : Height::Medium;
    s.holdForMore = flag(pre, "holdForMore", false);
    s.vibrate = flag(pre, "vibrate", false);
    return s;
}

Settings Settings::fromJson(const QJsonObject &pre)
{
    return fromJson(pre, Settings());
}

QJsonObject Settings::toJson(QJsonObject pre) const
{
    pre.insert(QLatin1String("style"), style == Style::White ? QLatin1String("white") : QLatin1String("orange"));
    pre.insert(QLatin1String("height"), height == Height::Small ? QLatin1String("small")
               : height == Height::Large ? QLatin1String("large") : QLatin1String("medium"));
    pre.insert(QLatin1String("holdForMore"), holdForMore);
    pre.insert(QLatin1String("vibrate"), vibrate);
    return pre;
}

bool Settings::operator==(const Settings &o) const
{
    return style == o.style && height == o.height && holdForMore == o.holdForMore && vibrate == o.vibrate &&
           suggestions == o.suggestions && sound == o.sound && swipe == o.swipe &&
           languages == o.languages && language == o.language;
}

QStringList Settings::recentEmoji(const QJsonObject &pre)
{
    QStringList out;
    for (const auto &v : pre.value(QLatin1String("recentEmoji")).toArray())
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
