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

#include "presettings.h"

#include <QJsonArray>

#include <algorithm>

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
    s.vibrateLevel = std::clamp(pre.value(QLatin1String("vibrateLevel")).toInt(1), 1, VIBRATE_LEVELS);
    s.followShift = flag(pre, "followShift", false);
    s.swipe = flag(pre, "swipe", true);
    s.doubleSpacePeriod = flag(pre, "doubleSpacePeriod", true);
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
    pre.insert(QLatin1String("vibrateLevel"), vibrateLevel);
    pre.insert(QLatin1String("followShift"), followShift);
    pre.insert(QLatin1String("swipe"), swipe);
    pre.insert(QLatin1String("doubleSpacePeriod"), doubleSpacePeriod);
    return pre;
}

bool Settings::operator==(const Settings &o) const
{
    return style == o.style && height == o.height && holdForMore == o.holdForMore && vibrate == o.vibrate &&
           vibrateLevel == o.vibrateLevel &&
           followShift == o.followShift && suggestions == o.suggestions && sound == o.sound && swipe == o.swipe &&
           doubleSpacePeriod == o.doubleSpacePeriod &&
           languages == o.languages && language == o.language;
}

int Settings::keyBuzzMs() const
{
    static const int ms[VIBRATE_LEVELS] = { 12, 20, 30 };
    return ms[std::clamp(vibrateLevel, 1, VIBRATE_LEVELS) - 1];
}

QStringList Settings::removed(const QJsonObject &pre, const QString &id)
{
    QStringList out;
    for (const QJsonValue &v : pre.value(QLatin1String("removed")).toObject().value(id).toArray())
        if (v.isString() && !v.toString().isEmpty()) out << v.toString();
    return out;
}

QJsonObject Settings::withRemoved(QJsonObject pre, const QString &id, const QStringList &words)
{
    QJsonObject removed = pre.value(QLatin1String("removed")).toObject();
    if (words.isEmpty()) removed.remove(id);
    else removed.insert(id, QJsonArray::fromStringList(words));
    pre.insert(QLatin1String("removed"), removed);
    return pre;
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
