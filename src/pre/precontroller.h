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

#ifndef PRE_CONTROLLER_H
#define PRE_CONTROLLER_H

#include "preemojiwords.h"
#include "presettings.h"
#include "preswipedecoder.h"

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QStringList>
#include <QThreadPool>
#include <QVector>

#include <atomic>
#include <functional>
#include <memory>

class TestPreKeyboard;

namespace Pre {

class KeyboardView;

/*!
 * What the Pre Keyboard needs from LuneOS's keyboard. The Pre Keyboard is the
 * keys and the bar; the text goes through the same editor and word engine as
 * every LuneOS layout, so prediction, autocorrect, auto-capitals, the owner's
 * dictionary and the languages are theirs. The input method fills these in.
 */
struct Bridge
{
    //! A key, as a LuneOS key would send it to the event handler: press, then release.
    //! [action] is the event handler's ("space", "return", "backspace"); empty for text.
    std::function<void(const QString &label, const QString &action)> press;
    std::function<void(const QString &label, const QString &action)> release;
    //! A backspace held down was let go of without deleting (the touch was taken away).
    std::function<void()> cancelRepeat;
    //! A word from the bar: it replaces the one being typed, and goes in with a space after.
    std::function<void(const QString &word)> pick;
    //! A swiped word: it stands as the word being typed, in place of what was (none).
    std::function<void(const QString &word)> setWord;
    //! No word is being typed and the cursor is right after a letter: a swiped word would run into it.
    std::function<bool()> atWordEnd;
    //! Text put in as it is, after the word being typed: emoji, a paste, a link.
    std::function<void(const QString &text)> commit;
    /*!
     * [before] characters before the cursor and [after] after it give way to
     * [text], as it is, the cursor after it. No word is being typed.
     */
    std::function<void(int before, int after, const QString &text)> replace;
    //! Where the cursor is in the field; -1 when the application doesn't say.
    std::function<int()> position;
    /*!
     * What else [word], at the cursor after [left], might be, likeliest first:
     * what was typed, for a word autocorrected; then the spelling checker's.
     */
    std::function<QStringList(const QString &word, const QString &left)> alternatives;
    //! The cursor moved by a key, extending the selection when [select].
    std::function<void(int dx, int dy, bool select)> cursor;
    //! Whether the next letter starts a sentence.
    std::function<bool()> autoCaps;
    //! The last [n] characters before the cursor, the word being typed included; and the next [n] after it.
    std::function<QString(int n)> before;
    std::function<QString(int n)> after;
    //! The field is for prose (free text, not hidden): a period brings its own space there.
    std::function<bool()> prose;
    //! The likeliest next words before any of the next is typed, best first.
    std::function<QStringList()> nextWords;
    //! Whether [word] could be added to the owner's dictionary: it is not known.
    std::function<bool(const QString &word)> canAdd;
    std::function<void(const QString &word)> addWord;
    //! Takes [word] out of the owner's dictionary, where it is one of theirs.
    std::function<void(const QString &word)> removeWord;
    //! Makes [id] the language typed in.
    std::function<void(const QString &id)> useLanguage;
    //! Calls a Luna method with no reply wanted (vibrate, launch).
    std::function<void(const QString &uri, const QString &payload)> luna;
    //! Writes the "pre" object back into the keyboard preference.
    std::function<void(const QJsonObject &pre)> savePrefs;
    //! Takes the keyboard down, as the keyboard's own choice (to open Text Assist).
    std::function<void()> hide;
};

/*!
 * Connects the Pre Keyboard's view to LuneOS's keyboard (see Bridge), and keeps
 * the view in step with the owner's settings while it is on screen.
 */
class Controller : public QObject
{
    Q_OBJECT
    friend class ::TestPreKeyboard;
    //! The Pre Keyboard is the keyboard: set by Keyboard.qml on a phone; a tablet keeps LuneOS's own.
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    // --- From the input method ---------------------------------------------------------

    //! What the keyboard does to the text goes through [bridge]; the controller notes when it did.
    void setBridge(const Bridge &bridge);
    //! Where emoji.txt is, and each language's emoji_words.txt (<dir>/<language>/).
    void setDataDir(const QString &dir) { m_dataDir = dir; }
    //! Where KLIPY keeps its state (its call count and customer id).
    void setStateDir(const QString &dir);
    /*!
     * Where the words to swipe are: the language plugins' presage databases
     * ([libDir]/<id>/database_<id>.db) and hunspell's dictionaries ([dictDir]).
     * Without them there is no swiping.
     */
    void setWordDirs(const QString &libDir, const QString &dictDir);
    //! The words swiped in the language typed in now; null till they are loaded.
    QSharedPointer<SwipeDecoder> swipeWords() const { return m_decoder; }

    bool active() const { return m_active; }
    void setActive(bool active);
    //! The "pre" object of the keyboard preference: the Pre Keyboard's look.
    void setPrefs(const QJsonObject &pre);
    //! LuneOS's keyboard settings the Pre Keyboard follows.
    void setKeyboardState(bool suggestions, bool sound, const QStringList &enabledLanguages,
                          const QString &activeLanguage);
    //! The word engine's candidates for the word being typed ([typed]); [primary] is what space would put in.
    void setCandidates(const QString &typed, const QString &primary, const QStringList &words);
    //! Whether the field takes words: the bar shows only for one that does.
    void setWordsAllowed(bool allowed);
    //! The field takes only these characters (a number, a phone number), or anything when empty.
    void setDigits(const QString &allowed);

    //! A field took the focus, or the keyboard came up for one.
    void startInput();
    void finishInput();
    //! The application told the keyboard about its text or cursor, or a capital became due.
    void update();
    /*!
     * Space or punctuation put [fixed] in place of the [typed] word. The very
     * next backspace puts back what was typed; a tap on the word offers it.
     */
    void corrected(const QString &typed, const QString &fixed);

    void setView(KeyboardView *view);

    // --- From the view -----------------------------------------------------------------

    void onText(const QString &text);
    //! One backspace, typed: a tap, or a repeat while searching.
    void onBackspace(bool word);
    //! Backspace went down ([held]) or came up; [lifted] is false when the touch was taken away.
    void onBackspaceHeld(bool held, bool lifted = true);
    void onEnter();
    void onCursor(int dx, int dy, bool select);
    void onToggleStyle();
    void onToggleHold();
    bool wantsCaps();
    void onSwipe(const QVector<float> &xs, const QVector<float> &ys, int caps);
    void onSwipeProgress(const QVector<float> &xs, const QVector<float> &ys, int caps);
    //! The [i]th word in the bar was tapped; [caps] is shift as it was: the word goes in so.
    void onPick(int i, int caps);
    bool canAdd(const QString &word);
    void onAddWord(const QString &word);
    //! Whether [word] could be deleted: the dictionaries know it.
    bool canRemove(const QString &word);
    //! [word] is never offered or corrected to again in this language; Text Assist can put it back.
    void onRemoveWord(const QString &word);
    void onPaste();
    //! A GIF or sticker from KLIPY was tapped on the emoji page, found by searching for [query].
    void onMedia(int kind, const QString &link, const QString &slug, const QString &query);
    void onLanguageFlick(int step);
    void onUseLanguage(const QString &id);
    void onOpenSettings();
    void hapticKey();
    void hapticLongPress();
    QString emojiFile() const;
    QStringList recentEmoji() const;
    void addRecentEmoji(const QString &e);

Q_SIGNALS:
    void activeChanged();
    //! The language's words to swipe are in (or it has none).
    void swipeWordsLoaded();

public:
    //! Where the time comes from, in ms: the tests set their own.
    void setClock(const std::function<qint64()> &clock) { m_clock = clock; }

    //! Two spaces at most this far apart (ms) after a word are a period.
    static constexpr qint64 DOUBLE_SPACE_MS = 1000;
    //! The cursor moving this long (ms) after the keyboard's last change to the text is the owner's doing.
    static constexpr qint64 OWN_EDIT_MS = 400;
    //! The buzz for a word autocorrected, with vibration on (ms).
    static constexpr int CORRECTED_MS = 28;

    /*!
     * The word the cursor is in or touching, in text ending in [before] and
     * going on with [after]: its parts either side of the cursor. Empty for
     * none, a number, or part of an address, a link or a #tag.
     */
    static QString wordAround(const QString &before, const QString &after, int *nBefore, int *nAfter);

private:
    /*!
     * The Pre's punctuation, between the keys and the editor: a double space
     * after a word is ". ", a period brings its own space (which a space typed
     * on top of is absorbed by), and sentence punctuation typed after a space
     * goes against the word ("word ," is "word, "). True when it typed [text]
     * itself; false leaves [text] to be typed as it is.
     */
    bool punctuate(const QString &text);
    //! A character or a space, through the event handler, as a key of LuneOS's own.
    void key(const QString &text);
    void backspaceKey();
    //! Whether text ending in [before] wants a space after the period that ends it.
    static bool periodWantsSpace(const QString &before, const QString &after);
    //! The keyboard's own space ([m_autoTail]) is still the last thing typed.
    bool autoSpaced() const;
    //! A space typed after a word is the last thing typed; [digit]: after a number.
    bool spaceAfterWord(bool *digit = nullptr) const;
    qint64 now() const;

    void applySettings();
    //! The settings as the view has them: swiping only once there are words to swipe.
    Settings shown() const;
    void loadWords();
    void wordsLoaded(const QString &id, const QSharedPointer<SwipeDecoder> &words);
    //! [word] as shift typed it.
    QString cased(const QString &word, int caps) const;
    //! A swipe's words, best first, as shift typed them ([caps]): they go in.
    void swiped(QStringList found, int caps);
    //! The guesses for a swipe still going on, if they are still the latest.
    void liveGuesses(quint64 generation, const QStringList &found);
    void showBar();
    //! With no word typed, after a space or at the start: the likeliest next words for the bar.
    QStringList nextWords();
    //! With no word typed: what the word at the cursor might be, if it was tapped; else the next words.
    QStringList idleWords();
    //! The word at the cursor was tapped, or the cursor is inside it: what else it might be.
    QStringList wordFixes();
    //! A backspace straight after an autocorrection: what was typed is put back. False if it wasn't.
    bool undoCorrection();
    void offerPaste();
    void writePrefs(const QJsonObject &pre);
    void vibrate(int ms);

    QPointer<KeyboardView> m_view;
    Bridge m_bridge;
    QString m_dataDir;
    bool m_active = false;
    QJsonObject m_prefs;
    Settings m_settings;
    bool m_wordsAllowed = true;
    QString m_digits;

    QString m_typed;
    QString m_primary;
    QStringList m_candidates;
    //! The next words, and the text before the cursor they were asked for.
    QStringList m_next;
    QString m_nextFor;
    QStringList m_bar = { QString(), QString(), QString() };
    bool m_deleting = false;
    QString m_clipDone;

    // Punctuation (see punctuate()).
    std::function<qint64()> m_clock;
    qint64 m_lastSpaceAt;
    //! The text the keyboard's own space ends (". "), while it is the last thing typed.
    QString m_autoTail;
    //! A space typed after a word is the last thing typed.
    bool m_spaceIsLast = false;

    //! The bar's right-hand place holds an emoji for the middle word, not a word.
    bool m_barEmoji = false;

    // Fixing words.
    //! When the keyboard last changed the text; and the cursor moved since by the owner's hand.
    qint64 m_ownEditAt;
    bool m_tapped = false;
    int m_position = -1;
    //! The last autocorrection, till anything but a backspace is typed.
    QString m_fixTyped;
    QString m_fixed;
    //! The word at the cursor the bar offers others for (its parts either side of the cursor).
    QString m_barWord;
    int m_barWordBefore = 0;
    int m_barWordAfter = 0;
    //! What it was asked about, and the answer.
    QString m_fixesFor;
    QStringList m_fixes;
    //! The emoji for words in [m_emojiFor], read the first time it is wanted.
    EmojiWords m_emojiWords;
    QString m_emojiFor;
    const EmojiWords &emojiWords();

    // Swipe to type.
    QString m_libDir;
    QString m_dictDir;
    //! Each language's words, the last few used; and the one's typed in now.
    QHash<QString, QSharedPointer<SwipeDecoder>> m_decoders;
    QStringList m_decoderOrder;
    QSharedPointer<SwipeDecoder> m_decoder;
    QString m_loading;
    //! The swiped word standing as the word being typed, and the other guesses for it.
    QString m_swiped;
    QStringList m_swipeGuesses;
    //! The guesses while the finger is still moving.
    QStringList m_live;
    //! Which swipe the guesses being worked out are for: any newer one makes them moot.
    std::shared_ptr<std::atomic<quint64>> m_liveGeneration = std::make_shared<std::atomic<quint64>>(0);
    //! Loading and live guesses, one at a time, off the keyboard's thread. Last, so it goes first.
    QThreadPool m_pool;
};

} // namespace Pre

#endif // PRE_CONTROLLER_H
