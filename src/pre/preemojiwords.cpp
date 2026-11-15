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

#include "preemojiwords.h"

#include <QFile>
#include <algorithm>
#include <QRegularExpression>
#include <QStringList>

namespace Pre {

EmojiWords::EmojiWords(const QString &text, const QSet<QString> &drawable)
{
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1Char('#'))) continue;
        const qsizetype tab = line.indexOf(QLatin1Char('\t'));
        if (tab <= 0) continue;
        QStringList list;
        for (const QString &e : line.mid(tab + 1).split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            const QString emoji = e.trimmed();
            if (emoji.isEmpty() || (!drawable.isEmpty() && !drawable.contains(emoji))) continue;
            list << emoji;
        }
        if (!list.isEmpty()) m_byWord.insert(line.left(tab), list);
    }
    m_words = m_byWord.keys();
    std::sort(m_words.begin(), m_words.end(), [](const QString &a, const QString &b) {
        return a.size() != b.size() ? a.size() < b.size() : a < b;
    });
}

EmojiWords EmojiWords::load(const QString &path, const QSet<QString> &drawable)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return EmojiWords();
    return EmojiWords(QString::fromUtf8(f.readAll()), drawable);
}

QString EmojiWords::clean(const QString &word)
{
    QString w = word.toLower().trimmed();
    while (w.endsWith(QLatin1Char('\'')) || w.endsWith(QChar(0x2019))) w.chop(1);
    return w;
}

QStringList EmojiWords::allFor(const QString &w) const
{
    if (w.isEmpty()) return QStringList();
    const auto exact = m_byWord.constFind(w);
    if (exact != m_byWord.constEnd()) return exact.value();
    // Pizzas, boxes, the dog's.
    static const QString endings[] = { QStringLiteral("'s"), QStringLiteral("’s"), QStringLiteral("es"),
                                       QStringLiteral("s") };
    for (const QString &end : endings) {
        if (w.size() > end.size() + 1 && w.endsWith(end)) {
            const auto stem = m_byWord.constFind(w.chopped(end.size()));
            if (stem != m_byWord.constEnd()) return stem.value();
        }
    }
    return QStringList();
}

QString EmojiWords::forWord(const QString &word) const
{
    const QStringList all = allFor(clean(word));
    return all.isEmpty() ? QString() : all.first();
}

QStringList EmojiWords::search(const QString &query, int limit) const
{
    QStringList parts;
    for (const QString &p : query.split(QRegularExpression(QStringLiteral("[ \t\n]")), Qt::SkipEmptyParts)) {
        const QString w = clean(p);
        if (!w.isEmpty()) parts << w;
    }
    if (parts.isEmpty()) return QStringList();
    // For each word: its own emoji, then those of the words it starts, each once, in order.
    QList<QStringList> each;
    for (const QString &w : parts) {
        QStringList found;
        QSet<QString> seen;
        auto add = [&](const QStringList &list) {
            for (const QString &e : list)
                if (!seen.contains(e)) { seen.insert(e); found << e; }
        };
        add(allFor(w));
        for (const QString &k : m_words)
            if (k.startsWith(w)) add(m_byWord.value(k));
        each << found;
    }
    QStringList result;
    for (const QString &e : each.first()) {
        bool inAll = true;
        for (int i = 1; i < each.size() && inAll; ++i) inAll = each[i].contains(e);
        if (inAll) result << e;
    }
    if (result.isEmpty()) {
        QSet<QString> seen;
        for (const QStringList &list : each)
            for (const QString &e : list)
                if (!seen.contains(e)) { seen.insert(e); result << e; }
    }
    return result.mid(0, limit);
}

} // namespace Pre
