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
#include <QStringList>

namespace Pre {

EmojiWords::EmojiWords(const QString &text, const QSet<QString> &drawable)
{
    for (const QString &line : text.split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1Char('#'))) continue;
        const qsizetype tab = line.indexOf(QLatin1Char('\t'));
        if (tab <= 0) continue;
        for (const QString &e : line.mid(tab + 1).split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            const QString emoji = e.trimmed();
            if (emoji.isEmpty() || (!drawable.isEmpty() && !drawable.contains(emoji))) continue;
            m_byWord.insert(line.left(tab), emoji);
            break;
        }
    }
}

EmojiWords EmojiWords::load(const QString &path, const QSet<QString> &drawable)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return EmojiWords();
    return EmojiWords(QString::fromUtf8(f.readAll()), drawable);
}

QString EmojiWords::forWord(const QString &word) const
{
    QString w = word.toLower();
    while (w.endsWith(QLatin1Char('\'')) || w.endsWith(QChar(0x2019))) w.chop(1);
    if (w.isEmpty()) return QString();
    const QString exact = m_byWord.value(w);
    if (!exact.isEmpty()) return exact;
    // Pizzas, boxes, the dog's.
    static const QString endings[] = { QStringLiteral("'s"), QStringLiteral("’s"), QStringLiteral("es"),
                                       QStringLiteral("s") };
    for (const QString &end : endings) {
        if (w.size() > end.size() + 1 && w.endsWith(end)) {
            const QString stem = m_byWord.value(w.chopped(end.size()));
            if (!stem.isEmpty()) return stem;
        }
    }
    return QString();
}

} // namespace Pre
