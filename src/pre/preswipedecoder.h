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

#ifndef PRE_SWIPEDECODER_H
#define PRE_SWIPEDECODER_H

#include <QHash>
#include <QMutex>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

namespace Pre {

/*!
 * Swipe typing: the words a finger's path across the letters most likely spells.
 *
 * Each dictionary word has an ideal path, straight lines through the centres of
 * its letters' keys. The finger's path is compared with it, and the closeness is
 * traded against how common the word is. A quick finger is sloppy in known ways:
 * it rounds its corners, aims a little off each letter, starts late and lifts
 * early, and hurries along some stretches more than others. So nothing a quick
 * finger does throws a word out; it only costs it:
 *
 * - First a cheap pass over every word whose first and last letters are near the
 *   path's ends and whose path is about as long: how far each letter is from
 *   where the finger went, and how far the ends are off. Only the best few
 *   hundred go on.
 * - Then the paths are lined up elastically (dynamic time warping, as in
 *   Kristensson and Zhai's SHARK² "location channel" but letting either path run
 *   ahead of the other for a while), so a stretch swiped fast or a corner cut
 *   short doesn't count as being off the word.
 *
 * Distances are in keys: across in key widths, down in row heights. Safe to use
 * from two threads: the live guesses are worked out off the keyboard's thread
 * while a learned word may be added on it.
 */
class SwipeDecoder
{
public:
    //! Where the letters are: centres of the keys a to z, and a key's size, in pixels.
    struct Layout
    {
        float x[26];
        float y[26];
        float keyWidth = 0;
        float keyHeight = 0;
    };

    explicit SwipeDecoder(const QVector<QPair<QString, int>> &entries);

    //! A word to offer, with its log2 frequency score (see the dictionary).
    void add(const QString &word, int score);
    //! A word the owner took out of their dictionary: gone, unless the dictionary has it too.
    void remove(const QString &word);

    /*!
     * The likeliest words for the path ([px], [py] in pixels), best first; empty
     * for a tap. [likely] are words that often follow the word before, likeliest
     * first: where the path can't tell two words apart ("of", "if"), they lean
     * that way. [stale] says the answer is no longer wanted: it gives up.
     */
    QStringList decode(const QVector<float> &px, const QVector<float> &py, const Layout &layout, int max = 4,
                       const QStringList &likely = {},
                       const std::function<bool()> &stale = [] { return false; });

private:
    struct Word
    {
        QString word;
        QVector<int> letters;   // 0..25
        float score = 0;
        bool learned = false;
    };

    void addWord(const QString &word, int score, bool learned);
    float warp(const float *ax, const float *ay, const float *bx, const float *by);

    QMutex m_lock;
    //! By first and last letter.
    QHash<int, QVector<Word>> m_buckets;
    //! Words without an apostrophe, to tell "were" from "we're".
    QSet<QString> m_plain;
};

} // namespace Pre

#endif // PRE_SWIPEDECODER_H
