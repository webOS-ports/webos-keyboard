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

#ifndef PRE_SWIPEDECODER_H
#define PRE_SWIPEDECODER_H

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QPair>
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
    /*!
     * Where the letters are: each letter key's character (lower case) and the
     * centre of its key, and a key's size, in pixels. Any alphabet: the words
     * are spelt out on whatever keys the page has.
     */
    struct Layout
    {
        QString keys;
        QVector<float> x;
        QVector<float> y;
        float keyWidth = 0;
        float keyHeight = 0;

        bool isValid() const
        {
            return keys.size() >= 2 && x.size() == keys.size() && y.size() == keys.size() && keyWidth > 0 &&
                   keyHeight > 0;
        }
    };

    //! [entries]: words and their scores; a word given twice counts at the higher.
    explicit SwipeDecoder(QVector<QPair<QString, int>> entries);

    //! A word to offer, with its log2 frequency score (see the dictionary).
    void add(const QString &word, int score);
    //! How many words there are to swipe (before the keys are known: all that could be).
    int size() const;
    //! A word the owner took out of their dictionary: gone, unless the dictionary has it too.
    void remove(const QString &word);
    //! A word the owner deleted from the dictionary: gone, wherever it came from.
    void forget(const QString &word);

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
    /*
     * A language can have a third of a million words (a hunspell dictionary's
     * stems): kept small, their text in one string and their keys in one array.
     */
    struct Word
    {
        quint32 at = 0;       // in m_text
        quint32 keysAt = 0;   // in m_keys
        quint16 length = 0;
        //! How many keys it is swiped over, a double letter once; 0 if it can't be on these keys.
        quint8 keys = 0;
        bool learned = false;
        bool gone = false;
        float score = 0;
    };

    QStringView text(const Word &w) const { return QStringView(m_text).mid(w.at, w.length); }
    const uchar *keysOf(const Word &w) const { return reinterpret_cast<const uchar *>(m_keys.constData()) + w.keysAt; }
    void append(const QString &word, float score, bool learned);
    //! The index of [word] in m_words, or -1.
    int find(QStringView word) const;
    //! [w]'s keys on m_alphabet, a double letter once; empty when a letter has no key.
    QByteArray spell(QStringView w) const;
    void index(int i);
    //! The words put on [alphabet]'s keys, if they are not already.
    void useKeys(const QString &alphabet);
    float warp(const float *ax, const float *ay, const float *bx, const float *by);

    mutable QMutex m_lock;
    QString m_text;
    QByteArray m_keys;
    QVector<Word> m_words;
    //! The first m_sorted words are in order (binary searched); the learned ones after.
    int m_sorted = 0;
    //! The keys the words are spelt on now, and the words by first and last key.
    QString m_alphabet;
    QHash<int, QVector<int>> m_buckets;
};

} // namespace Pre

#endif // PRE_SWIPEDECODER_H
