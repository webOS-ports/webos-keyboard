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

#ifndef PRE_EMOJIWORDS_H
#define PRE_EMOJIWORDS_H

#include <QHash>
#include <QSet>
#include <QString>

namespace Pre {

/*!
 * The emoji the bar offers for a word ("pizza" -> 🍕), from a language's
 * emoji_words.txt (data/pre/<language>/, see data/pre/README.md): a line per
 * word, "word<TAB>emoji emoji ...", the likeliest emoji first. Of each word's
 * emoji the first the emoji page has is the one offered, so the bar never
 * shows one the page can't.
 */
class EmojiWords
{
public:
    EmojiWords() = default;
    //! From the text of an emoji_words.txt; [drawable] empty takes every emoji.
    EmojiWords(const QString &text, const QSet<QString> &drawable);
    //! From [path]; none if it can't be read.
    static EmojiWords load(const QString &path, const QSet<QString> &drawable);

    //! The emoji for [word], or for the word it is the plural or possessive of; empty if none.
    QString forWord(const QString &word) const;
    int size() const { return int(m_byWord.size()); }

private:
    QHash<QString, QString> m_byWord;
};

} // namespace Pre

#endif // PRE_EMOJIWORDS_H
