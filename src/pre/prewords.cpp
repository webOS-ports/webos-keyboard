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

#include "prewords.h"

#include <algorithm>

namespace Pre {
namespace {
const int MAX_PAIRS = 20000;
}

const QString NextWords::START = QStringLiteral("<s>");
const QString NextWords::COMMA = QStringLiteral("<,>");

NextWords::NextWords(const QStringList &lines)
{
    m_common.reserve(16384);
    for (const QString &line : lines) {
        if (line.isEmpty() || line[0] == QLatin1Char('#'))
            continue;
        const int tab = line.indexOf(QLatin1Char('\t'));
        if (tab <= 0)
            continue;
        m_common.insert(line.left(tab), line.mid(tab + 1).split(QLatin1Char(' '), Qt::SkipEmptyParts));
    }
}

QStringList NextWords::predict(const QString &context, int max, const std::function<bool(const QString &)> &ok) const
{
    QStringList out;
    auto has = [&](const QString &w) {
        for (const QString &o : out)
            if (o.compare(w, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    QList<QPair<QString, int>> mine;
    {
        QMutexLocker l(&m_lock);
        auto it = m_owned.constFind(context);
        if (it != m_owned.cend())
            for (auto w = it->cbegin(); w != it->cend(); ++w)
                mine.append({ w.key(), w.value() });
    }
    std::stable_sort(mine.begin(), mine.end(),
                     [](const QPair<QString, int> &a, const QPair<QString, int> &b) { return a.second > b.second; });
    for (const auto &m : mine)
        if (m.second >= 2 && ok(m.first) && !out.contains(m.first)) out << m.first;
    for (const QString &w : m_common.value(context))
        if (!has(w)) out << w;
    for (const auto &m : mine)
        if (out.size() < max && ok(m.first) && !has(m.first)) out << m.first;
    return out.mid(0, max);
}

bool NextWords::follows(const QString &context, const QString &word) const
{
    for (const QString &w : m_common.value(context))
        if (w.compare(word, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

void NextWords::learn(const QString &context, const QString &word)
{
    QMutexLocker l(&m_lock);
    QHash<QString, int> &m = m_owned[context];
    auto had = m.find(word);
    if (had == m.end()) {
        m.insert(word, 1);
        m_ownedPairs++;
    } else {
        had.value()++;
    }
    m_dirty = true;
    if (m_ownedPairs > MAX_PAIRS)
        forgetRare();
}

void NextWords::forgetRare()
{
    for (auto it = m_owned.begin(); it != m_owned.end();) {
        QHash<QString, int> &m = it.value();
        for (auto w = m.begin(); w != m.end();) {
            if (w.value() < 2) w = m.erase(w);
            else ++w;
        }
        if (m.isEmpty()) it = m_owned.erase(it);
        else ++it;
    }
    m_ownedPairs = 0;
    for (const auto &m : std::as_const(m_owned))
        m_ownedPairs += m.size();
}

bool NextWords::dirty() const
{
    QMutexLocker l(&m_lock);
    return m_dirty;
}

QString NextWords::save()
{
    QMutexLocker l(&m_lock);
    m_dirty = false;
    QString sb;
    for (auto it = m_owned.cbegin(); it != m_owned.cend(); ++it)
        for (auto w = it->cbegin(); w != it->cend(); ++w)
            sb += it.key() + QLatin1Char('\t') + w.key() + QLatin1Char('\t') + QString::number(w.value()) + QLatin1Char('\n');
    return sb;
}

void NextWords::load(const QStringList &saved)
{
    QMutexLocker l(&m_lock);
    for (const QString &line : saved) {
        const QStringList parts = line.split(QLatin1Char('\t'));
        if (parts.size() != 3)
            continue;
        bool ok = false;
        const int n = parts[2].toInt(&ok);
        if (!ok)
            continue;
        QHash<QString, int> &m = m_owned[parts[0]];
        if (!m.contains(parts[1]))
            m_ownedPairs++;
        m.insert(parts[1], n);
    }
}

EmojiWords::EmojiWords(const QStringList &lines)
{
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1Char('#')))
            continue;
        const int tab = line.indexOf(QLatin1Char('\t'));
        if (tab <= 0)
            continue;
        // LuneOS draws them all (Noto Color Emoji): the likeliest one is offered.
        const QStringList e = line.mid(tab + 1).split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (!e.isEmpty())
            m_byWord.insert(line.left(tab), e.first());
    }
}

QString EmojiWords::forWord(const QString &word) const
{
    QString w = word.toLower();
    while (!w.isEmpty() && (w.endsWith(QLatin1Char('\'')) || w.endsWith(QChar(0x2019))))
        w.chop(1);
    if (w.isEmpty())
        return QString();
    auto it = m_byWord.constFind(w);
    if (it != m_byWord.cend())
        return it.value();
    static const QStringList endings = { QStringLiteral("'s"), QStringLiteral("’s"), QStringLiteral("es"), QStringLiteral("s") };
    for (const QString &end : endings) {
        if (w.length() > end.length() + 1 && w.endsWith(end)) {
            auto s = m_byWord.constFind(w.left(w.length() - end.length()));
            if (s != m_byWord.cend())
                return s.value();
        }
    }
    return QString();
}

} // namespace Pre
