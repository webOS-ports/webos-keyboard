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

#ifndef PRE_SWIPEWORDS_H
#define PRE_SWIPEWORDS_H

#include <QPair>
#include <QString>
#include <QVector>

namespace Pre {

/*!
 * The words to swipe in a language, from what the LuneOS word engine already
 * has for it, with a log2 frequency score on the scale SwipeDecoder expects
 * (25 for the commonest word, 5 and under too rare to offer):
 *
 * - presage's database for the language plugin (database_<id>.db, built from
 *   the plugin's corpus): each word it saw, scored by how often, 10 to 25;
 * - the hunspell dictionary (<id>.dic, or <id>_*.dic): the stems the corpus
 *   never had, at 8, a name (capitalised) at 6. A stem without its endings is
 *   still most of what a language's swiped words are.
 *
 * Either may be missing; with neither the list is empty and there is no swiping.
 */
namespace SwipeWords {

using Entries = QVector<QPair<QString, int>>;

//! Everything for [languageId], from presage databases under [libDir] and hunspell's [dictDir],
//! with the dictionary the language is spelled with in [country] (see hunspellDictionary()).
Entries load(const QString &languageId, const QString &libDir, const QString &dictDir,
             const QString &country = QString());

//! A presage database's 1-grams, scored (see above).
Entries fromPresage(const QString &dbFile);
//! A hunspell dictionary's words, without their affix flags, scored (see above).
Entries fromHunspell(const QString &dicFile);
//! Where [languageId]'s hunspell dictionary for [country] is in [dictDir]; empty if it has none.
QString hunspellFile(const QString &languageId, const QString &dictDir,
                     const QString &country = QString());

//! Scores for the presage words: the commonest word [max] times, at 25.
int presageScore(qint64 count, qint64 max);
static const int HUNSPELL_SCORE = 8;
static const int HUNSPELL_NAME_SCORE = 6;

} // namespace SwipeWords
} // namespace Pre

#endif // PRE_SWIPEWORDS_H
