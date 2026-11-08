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

#ifndef PRE_COMPOSER_H
#define PRE_COMPOSER_H

#include <QPair>
#include <QSharedPointer>
#include <QString>

#include <functional>

namespace Pre {

class Corrector;

/*!
 * What typing does to the text: autocorrect when a word is finished, undoing a
 * correction with the very next backspace, and a double-tapped space becoming
 * ". ".
 *
 * A period and a swiped word each bring the space after them (an "auto space").
 * It stays the keyboard's while it is the last thing typed: a space typed on top
 * of it is absorbed rather than doubled, and sentence punctuation goes against
 * the word, so a period after a swiped word lands on the word and "..." stays
 * together. A backspace after a period takes only its space; after a swipe it
 * takes the word and its spaces.
 *
 * A space typed after a word (or the one a picked word brings) is the owner's,
 * but sentence punctuation typed straight after it still goes against the word,
 * and the space moves to after the punctuation: "word ," becomes "word, ".
 *
 * It works on a Field rather than on Maliit's host, so the tests can drive it
 * with a plain string.
 */
class Composer
{
public:
    //! The text being edited, as much of it as the keyboard needs.
    class Field
    {
    public:
        virtual ~Field() {}
        virtual QString before(int n) = 0;
        virtual QString after(int n) { Q_UNUSED(n); return QString(); }
        virtual void remove(int n) = 0;
        virtual void commit(const QString &s) = 0;
    };

    //! What may happen in the field at hand: the owner's toggles, less what the field rules out.
    struct Rules
    {
        bool autocorrect = true;
        bool doubleSpacePeriod = true;
        bool prose = true;
    };

    Composer(Field *field, std::function<qint64()> clock);

    QSharedPointer<Corrector> corrector;
    //! Told about a word the owner picked as typed over its correction, so it can be remembered.
    std::function<void(const QString &)> onLearned;
    /*!
     * Told when autocorrect changed a word: what was typed, what it became, and
     * how many characters the cursor is past its end once the edit is done.
     */
    std::function<void(const QString &, const QString &, int)> onCorrected;

    //! The word the last swipe typed, while it is still the last thing typed; empty when none.
    QString swiped() const { return m_swiped; }

    void reset();
    //! Something typed on the keyboard.
    void type(const QString &text, const Rules &rules);
    //! Return is about to be sent: finish the word first. Return itself is the caller's to send.
    void beforeEnter(const Rules &rules);
    //! Backspace. Straight after a correction it puts back what was typed; true when it did something.
    bool backspace();
    //! Anything else that moves on from the last correction (the cursor moved, say).
    void forget();
    //! The swiped word is still the last thing typed (with its space, if that is still there).
    bool swipedIsLast();
    //! A swiped word: a space in front when it follows a word, and an auto space after.
    void swipe(const QString &word, const Rules &rules);
    //! A word tapped in the suggestion bar. [learn]: the owner chose what they typed over the correction.
    void pick(const QString &word, bool learn);
    //! The word the cursor is at the end of, as far as typed: empty when there is none.
    QString currentWord();
    //! What the next word follows, for guessing it before it is typed; empty when nothing.
    QString nextContext();
    //! The word just finished and what it followed, for learning; an empty pair when none.
    QPair<QString, QString> finishedPair();
    //! What the word being typed follows, for weighing what it may be.
    QString wordContext();

    static bool isSeparator(const QString &text);
    static const qint64 DOUBLE_SPACE_MS = 1000;

private:
    struct Correction { QString typed, fixed, separator; bool valid = false; };

    void markSpace();
    bool spaceTyped();
    bool autoSpaced();
    bool periodWantsSpace();
    QString contextOf(const QString &before);
    Correction correctWord(const QString &separator);
    bool endsWithWordAndSpace();

    Field *m_field;
    std::function<qint64()> m_clock;
    Correction m_last;
    qint64 m_lastSpaceAt;
    QString m_swiped;
    //! The swipe put a space in front of its word, which a backspace takes too.
    bool m_swipedSpace = false;
    //! The text an auto space ends, space included (". " or "word "), while it is the last thing typed.
    QString m_autoTail;
    //! The auto space the last swipe followed, given back when a backspace takes the swipe.
    QString m_swipedOn;
    //! A word's last character and the space typed after it, while that space is the last thing typed.
    QString m_spaceTail;
};

} // namespace Pre

#endif // PRE_COMPOSER_H
