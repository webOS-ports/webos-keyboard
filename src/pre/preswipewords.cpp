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

#include "preswipewords.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStringDecoder>

#include <sqlite3.h>

#include <algorithm>
#include <cmath>

namespace Pre {
namespace SwipeWords {
namespace {

//! Hunspell's name for a language whose keyboard is called otherwise.
QString dictionaryId(const QString &id)
{
    if (id == QLatin1String("no")) return QStringLiteral("nb");
    return id;
}

//! The encoding a hunspell dictionary is in: its .aff's SET line; UTF-8 when it has none.
QByteArray encodingOf(const QString &dicFile)
{
    QFile aff(dicFile.left(dicFile.length() - 4) + QStringLiteral(".aff"));
    if (!aff.open(QIODevice::ReadOnly)) return QByteArrayLiteral("UTF-8");
    for (int i = 0; i < 400 && !aff.atEnd(); ++i) {
        const QByteArray line = aff.readLine().trimmed();
        if (line.startsWith("SET ")) return line.mid(4).trimmed();
    }
    return QByteArrayLiteral("UTF-8");
}

} // namespace

int presageScore(qint64 count, qint64 max)
{
    if (count <= 0 || max <= 0) return 0;
    const double share = std::log2(double(count) + 1) / std::log2(double(std::max(count, max)) + 1);
    return std::min(25, 10 + int(std::lround(15 * share)));
}

Entries fromPresage(const QString &dbFile)
{
    Entries out;
    if (!QFileInfo::exists(dbFile)) return out;
    // SQLite itself, as presage uses it: on every device that has presage's
    // databases, with no Qt driver plugin to go missing. Read-only, so a
    // database that is not there is never made empty instead.
    sqlite3 *db = nullptr;
    if (sqlite3_open_v2(dbFile.toUtf8().constData(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        sqlite3_close(db);   // open hands back a handle even when it fails
        return out;
    }
    sqlite3_stmt *q = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT word, count FROM _1_gram", -1, &q, nullptr) == SQLITE_OK) {
        QVector<QPair<QString, qint64>> rows;
        qint64 max = 0;
        while (sqlite3_step(q) == SQLITE_ROW) {
            // The text first, then its length: the length is of that text.
            const auto *word = reinterpret_cast<const char *>(sqlite3_column_text(q, 0));
            const int bytes = sqlite3_column_bytes(q, 0);
            const qint64 count = sqlite3_column_int64(q, 1);
            rows.append({ QString::fromUtf8(word, bytes), count });
            max = std::max(max, count);
        }
        out.reserve(rows.size());
        for (const auto &r : rows) out.append({ r.first, presageScore(r.second, max) });
    }
    sqlite3_finalize(q);
    sqlite3_close(db);
    return out;
}

QString hunspellFile(const QString &languageId, const QString &dictDir)
{
    const QString id = dictionaryId(languageId);
    const QDir dir(dictDir);
    QStringList tries = { id + QStringLiteral(".dic") };
    if (id == QLatin1String("en")) tries << QStringLiteral("en_US.dic");
    tries << id + QLatin1Char('_') + id.toUpper() + QStringLiteral(".dic");
    for (const QString &t : tries)
        if (dir.exists(t)) return dir.filePath(t);
    const QStringList any = dir.entryList({ id + QStringLiteral("_*.dic") }, QDir::Files, QDir::Name);
    return any.isEmpty() ? QString() : dir.filePath(any.first());
}

Entries fromHunspell(const QString &dicFile)
{
    Entries out;
    QFile f(dicFile);
    if (dicFile.isEmpty() || !f.open(QIODevice::ReadOnly)) return out;
    QStringDecoder decode(encodingOf(dicFile).constData());
    if (!decode.isValid()) return out;
    bool first = true;
    while (!f.atEnd()) {
        const QByteArray raw = f.readLine();
        if (first) { first = false; continue; }   // how many words there are
        // "word/FLAGS morphology": the word, up to an unescaped slash or a blank.
        QByteArray w;
        for (int i = 0; i < raw.size(); ++i) {
            const char c = raw.at(i);
            if (c == '\\' && i + 1 < raw.size() && raw.at(i + 1) == '/') { w.append('/'); ++i; continue; }
            if (c == '/' || c == '\t' || c == ' ' || c == '\n' || c == '\r') break;
            w.append(c);
        }
        if (w.isEmpty()) continue;
        const QString word = decode(w);
        if (decode.hasError()) { decode.resetState(); continue; }
        int upper = 0;
        for (QChar c : word) if (c.isUpper()) upper++;
        // An abbreviation in capitals is typed, not swiped.
        if (upper > 1) continue;
        out.append({ word, upper && word.at(0).isUpper() ? HUNSPELL_NAME_SCORE : HUNSPELL_SCORE });
    }
    return out;
}

Entries load(const QString &languageId, const QString &libDir, const QString &dictDir)
{
    Entries out = fromPresage(QStringLiteral("%1/%2/database_%2.db").arg(libDir, languageId));
    out += fromHunspell(hunspellFile(languageId, dictDir));
    return out;
}

} // namespace SwipeWords
} // namespace Pre
