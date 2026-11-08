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

#ifndef PRE_WORDS_H
#define PRE_WORDS_H

#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

#include <functional>

namespace Pre {

/*!
 * The words likeliest to come next, for the suggestion bar before a word is
 * typed: from next_words.txt (what follows what in everyday text), and ahead of
 * that, what the owner has typed after the same word before.
 *
 * A context is a word in lower case, START (a sentence's start) or COMMA (after
 * a comma).
 */
class NextWords
{
public:
    explicit NextWords(const QStringList &lines);

    static const QString START;
    static const QString COMMA;

    /*!
     * Up to [max] words for after [context], likeliest first: the owner's own
     * habits (seen at least twice, or once when nothing else is known) before the
     * common ones. [ok] filters out what shouldn't be offered.
     */
    QStringList predict(const QString &context, int max = 3,
                        const std::function<bool(const QString &)> &ok = [](const QString &) { return true; }) const;

    //! [word] is one of the words that commonly follow [context]. The common list only, which never changes.
    bool follows(const QString &context, const QString &word) const;

    //! The owner typed [word] after [context].
    void learn(const QString &context, const QString &word);
    bool dirty() const;
    //! The owner's pairs, one per line: context, word, count.
    QString save();
    //! Pairs save() wrote, back in.
    void load(const QStringList &saved);

private:
    void forgetRare();

    QHash<QString, QStringList> m_common;
    mutable QMutex m_lock;
    //! What the owner typed after each context, and how often.
    QHash<QString, QHash<QString, int>> m_owned;
    int m_ownedPairs = 0;
    bool m_dirty = false;
};

/*!
 * The emoji the suggestion bar offers for a word ("pizza" -> 🍕), from
 * emoji_words.txt: for each word, the likeliest emoji first.
 */
class EmojiWords
{
public:
    //! [canDraw] says whether the device draws an emoji; the first one it does is offered.
    explicit EmojiWords(const QStringList &lines,
                        const std::function<bool(const QString &)> &canDraw = [](const QString &) { return true; });
    //! The emoji for [word], or for the word it is the plural or possessive of; empty if none.
    QString forWord(const QString &word) const;

private:
    QHash<QString, QString> m_byWord;
};

} // namespace Pre

#endif // PRE_WORDS_H
