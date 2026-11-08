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

#ifndef PRE_CORRECTOR_H
#define PRE_CORRECTOR_H

#include "prelanguage.h"

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
 * Autocorrect: turns a finished word into the word that was probably meant, or
 * leaves it be.
 *
 * - Contractions typed without their apostrophe get it back: "dont" -> "don't",
 *   "im" -> "I'm". Only the ones whose bare form is not itself an everyday word:
 *   "well", "were", "its", "ill", "hell", "shell", "lets" and "wed" are left
 *   alone, but the suggestion bar offers their contraction beside them, and
 *   where the word before makes the contraction all but certain ("I think ill",
 *   "that its"), they get the apostrophe too.
 * - A lone "i", and a proper noun typed in lower case, get their capital.
 * - An unknown word becomes the dictionary word closest to it, where "close" is
 *   an edit distance that knows the Pre's keyboard: hitting the key next door,
 *   swapping two letters or doubling one costs less than any other slip.
 *   Distance is traded against how common the word is, so "teh" becomes "the"
 *   rather than "ten".
 * - Two words run together, the space missed or hit as a letter above it, come
 *   apart: "theyare" and "theynare" -> "they are".
 * - In other languages: a missing accent is the cheapest slip of all ("etre" ->
 *   "être"), the contraction rules are English only, and a word elided against
 *   the next ("l'homme") is the word after its apostrophe, with the apostrophe
 *   put in where it was left out ("cest" -> "c'est").
 *
 * Words are read once and then only looked up, so the suggestion thread may use
 * it while the keyboard thread learns a word: the owner's words are guarded.
 */
class Corrector
{
public:
    explicit Corrector(const QStringList &lines, const Language &lang = Language::enUS());

    const Language &lang() const { return m_lang; }
    int size() const { return m_words.size(); }

    //! contractions.txt: word, context, times as the word, times as the contraction.
    void loadContractions(const QStringList &lines);

    //! Whether one word commonly follows another (from NextWords), to tell run-together words apart.
    std::function<bool(const QString &, const QString &)> follows;

    //! Every word with its score, for the swipe decoder.
    QVector<QPair<QString, int>> entries() const;

    void learn(const QString &word);
    void learnAll(const QStringList &words);
    //! The owner took [word] out of their dictionary: corrected again, unless the dictionary has it.
    void unlearn(const QString &word);

    bool isKnown(const QString &word) const;
    //! [word] spelt as the dictionary spells it ("i'm" -> "I'm"), or empty for a word it doesn't know.
    QString spelling(const QString &word) const;

    //! The correction for [typed], or empty to leave it as typed. [context] is what it follows.
    QString correct(const QString &typed, const QString &context = QString()) const;
    //! Words for the suggestion bar while [typed] is being typed, likeliest first.
    QStringList suggestions(const QString &typed, const QString &context = QString(), int max = 3) const;
    //! Other words [word] might have been meant as, for a word tapped in the text. Never [word] itself.
    QStringList alternatives(const QString &word, int max = 3) const;

    bool adjacent(QChar a, QChar b) const;

    //! [word] with the capitals of [typed]: "Teh" -> "The". A word's own capitals stay.
    static QString casedLike(const QString &typed, const QString &word);

    static const QHash<QString, QString> &contractions();
    static const QHash<QString, QString> &ambiguous();

private:
    struct Entry { QString word; int score = 0; };
    typedef QPair<QString, float> Scored;

    bool learned(const QString &lower) const;
    bool contractionMeant(const QString &word, const QString &context) const;
    Scored accented(const QString &w) const;
    Scored elision(const QString &w) const;
    Scored split(const QString &w) const;
    QStringList completions(const QString &prefix, int max) const;
    QStringList nearest(const QString &w, int max) const;
    QList<Scored> nearestScored(const QString &w, int max) const;
    QList<Scored> pickScored(const QHash<QString, float> &cands, float maxCost, int minScore, int max,
                             float weight) const;
    void edits(const QString &w, float base, QHash<QString, float> &out, bool full) const;
    void accentEdits(const QString &w, const std::function<void(const QString &)> &add) const;
    bool adjacentToNeighbour(const QString &w, int i) const;
    static bool vowelSwap(const QString &a, const QString &b);
    static int halfScore(const QString &w);

    Language m_lang;
    //! Keyed by lower case; the value keeps the word's own case ("I", "London", "don't").
    QHash<QString, Entry> m_words;
    //! For a language with accents: each word in plain letters, and the commonest word spelt so.
    QHash<QString, Entry> m_plain;
    //! Every key, sorted, for finding the words a typed prefix begins.
    QStringList m_sorted;
    QHash<QString, QPair<int, int>> m_odds;
    QHash<QChar, QPair<int, int>> m_position;

    mutable QMutex m_learnedLock;
    QSet<QString> m_learned;
};

} // namespace Pre

#endif // PRE_CORRECTOR_H
