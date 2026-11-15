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

#include "predictation.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace Pre {
namespace Speech {

namespace {

//! A tag compared as a tag: "en_US" and "en-us" are the same.
QString tag(const QString &s)
{
    QString t = s.toLower();
    t.replace(QLatin1Char('_'), QLatin1Char('-'));
    return t;
}

QString primary(const QString &tagged)
{
    return tagged.section(QLatin1Char('-'), 0, 0);
}

bool isModel(const QString &dir)
{
    return QFileInfo::exists(dir + QStringLiteral("/am/final.mdl"));
}

} // namespace

QString modelFor(const QString &root, const QString &language)
{
    if (language.isEmpty()) return QString();
    const QString want = tag(language);
    const QStringList dirs = QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &d : dirs)
        if (tag(d) == want && isModel(root + QLatin1Char('/') + d)) return root + QLatin1Char('/') + d;
    for (const QString &d : dirs)
        if (primary(tag(d)) == primary(want) && isModel(root + QLatin1Char('/') + d))
            return root + QLatin1Char('/') + d;
    return QString();
}

QString recordingSource(const QString &defaultSource, const QList<QPair<QString, QString>> &sources)
{
    QString first;
    for (const auto &s : sources) {
        if (s.second != QLatin1String("sound")) continue;
        if (s.first == defaultSource) return s.first;
        if (first.isEmpty()) first = s.first;
    }
    return first;
}

QString resultText(const QByteArray &json, const char *key)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return QString();
    return doc.object().value(QLatin1String(key)).toString().simplified();
}

QString join(const QString &spoken, const QString &before, bool capital, const QString &language)
{
    QString s = spoken.simplified();
    if (s.isEmpty()) return s;
    if (primary(tag(language)) == QLatin1String("en")) {
        // The recogniser writes English in small letters, the pronoun too.
        static const QRegularExpression pronoun(QStringLiteral("(?<![\\w'])i(?=$|[\\s']|’)"));
        s.replace(pronoun, QStringLiteral("I"));
    }
    if (capital) s[0] = s.at(0).toUpper();
    static const QString opening = QStringLiteral("([{\"'“‘¿¡");
    const bool space = !before.isEmpty() && !before.at(before.size() - 1).isSpace()
                       && !opening.contains(before.at(before.size() - 1));
    return space ? QLatin1Char(' ') + s : s;
}

} // namespace Speech
} // namespace Pre
