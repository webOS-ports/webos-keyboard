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

#ifndef PRE_CONTROLLER_H
#define PRE_CONTROLLER_H

#include "precomposer.h"
#include "presettings.h"

#include <QAtomicInt>
#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QStringList>
#include <QThreadPool>
#include <QVector>

#include <functional>

class MAbstractInputMethodHost;

namespace Pre {

class Corrector;
class EmojiWords;
class KeyboardView;
class NextWords;
class SwipeDecoder;

/*!
 * The Maliit side of the Pre Keyboard. The view decides what was pressed; the
 * Composer decides what that does to the text; this class connects them to
 * whatever field has focus, and keeps the keyboard in step with the owner's
 * settings while it is on screen.
 *
 * The application only tells the keyboard what its text is after a round trip,
 * so the keyboard keeps its own copy of the text around the cursor, changed as it
 * changes the field, and takes the application's word for it again once that has
 * caught up (see Field below).
 */
class Controller : public QObject
{
    Q_OBJECT
    //! The Pre Keyboard is the one chosen ("Pre Keyboard" in Text Assist).
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    // --- From the input method ---------------------------------------------------------

    //! Where the keyboard's data is (dictionary.txt and the rest).
    void setDataDir(const QString &dir) { m_dataDir = dir; }
    //! Where the owner's next words are kept.
    void setStateDir(const QString &dir);
    void setHost(MAbstractInputMethodHost *host) { m_host = host; }
    //! Calls a Luna method with no reply wanted (vibrate, launch).
    void setLunaCall(const std::function<void(const QString &, const QString &)> &call) { m_lunaCall = call; }
    //! Writes the "pre" object back into the keyboard preference.
    void setSavePrefs(const std::function<void(const QJsonObject &)> &save) { m_savePrefs = save; }
    //! Switches the keyboard (keyboardLayout): "LuneOS" goes back to LuneOS's own.
    void setSwitchLayout(const std::function<void(const QString &)> &sw) { m_switchLayout = sw; }

    bool active() const { return m_active; }
    void setActive(bool active);
    //! The "pre" object of the keyboard preference: the owner's choices and words.
    void setPrefs(const QJsonObject &pre);

    //! A field took the focus, or the keyboard came up for one.
    void startInput();
    void finishInput();
    //! The application told the keyboard about its text or cursor.
    void update();
    //! A field for words: not a password, an address or a URL.
    void setProse(bool prose, bool quiet);

    void setView(KeyboardView *view);

    // --- From the view -----------------------------------------------------------------

    void onText(const QString &text);
    void onBackspace(bool word);
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
    //! Whether [word], held in the bar, may be deleted from the dictionary: it is in it.
    bool canRemove(const QString &word);
    void onRemoveWord(const QString &word);
    void onBackspaceHeld(bool held);
    void onPaste();
    //! A GIF or sticker from KLIPY was tapped on the emoji page, found by searching for [query].
    void onMedia(int kind, const QString &link, const QString &slug, const QString &query);
    void onLanguageFlick(int step);
    void onUseLanguage(const QString &id);
    void onUseLuneOSKeyboard();
    void onOpenSettings();
    void hapticKey();
    void hapticLongPress();
    QString emojiFile() const;
    QStringList recentEmoji() const;
    void addRecentEmoji(const QString &e);

Q_SIGNALS:
    void activeChanged();
    //! Results from a worker thread, back on this one.
    void wordsLoaded(quint64 generation);
    void barReady(quint64 generation, const QStringList &words, bool verbatim, bool emoji, bool tapped);
    void liveReady(quint64 generation, const QStringList &words);

private:
    class Field;
    struct Dictionary;
    struct FamilyWords;
    //! A word autocorrect changed, where it is in the text: tapped, it switches back.
    struct Mark { int start; QString text, typed, fixed; int end() const { return start + text.length(); } };
    //! A word in the text around the cursor: the part before it and the part after.
    struct WordAt { QString before, after; bool valid = false; QString word() const { return before + after; } };

    void useLanguage(const Language &lang);
    QSharedPointer<Dictionary> readDictionary(const Language &lang);
    QSharedPointer<FamilyWords> readFamily(const Language &lang);
    QStringList readLines(const QString &name) const;
    QString nextWordsFile(const Language &lang) const;
    void saveNextWords();
    void syncLearned();
    void syncRemoved();
    QString withCaps(const QString &word, int caps) const;
    void writePrefs(const QJsonObject &pre);

    Composer::Rules rules() const;
    void refreshBar();
    void show(const QStringList &words, bool verbatim, bool emoji);
    QStringList predict();
    void learnPair();
    WordAt wordAtCursor();
    void replaceTapped(const WordAt &at, const QString &word);
    void ownEdit();
    QString cased(const QString &word, int caps) const;
    QStringList swipeContext();
    void markCorrection(const QString &typed, const QString &fixed, int tail);
    bool backspaceIntoMark();
    void checkMarks();
    void toggleMark(int i, int from);
    void offerPaste();
    void sendKey(int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    qint64 now() const;

    QPointer<KeyboardView> m_view;
    MAbstractInputMethodHost *m_host = nullptr;
    std::function<void(const QString &, const QString &)> m_lunaCall;
    std::function<void(const QJsonObject &)> m_savePrefs;
    std::function<void(const QString &)> m_switchLayout;
    QString m_dataDir;
    QString m_stateDir;
    bool m_active = false;
    QJsonObject m_prefs;
    Settings m_settings;
    bool m_prose = true;
    bool m_quiet = false;

    Field *m_field;
    Composer m_composer;

    Language m_language;
    Language m_wordsFor;
    quint64 m_wordsGeneration = 0;
    QSharedPointer<Dictionary> m_dictionary;
    QSharedPointer<FamilyWords> m_family;
    //! Read ahead and kept: the dictionary in use and the ones a flick either way would bring.
    QHash<QString, QSharedPointer<Dictionary>> m_dictionaries;
    QStringList m_dictionaryOrder;
    QHash<QString, QSharedPointer<FamilyWords>> m_families;
    QThreadPool m_loading;
    QThreadPool m_suggest;
    QThreadPool m_live;

    QStringList m_swipeGuesses;
    QStringList m_bar = { QString(), QString(), QString() };
    bool m_barVerbatim = false;
    bool m_barEmoji = false;
    quint64 m_barGeneration = 0;
    quint64 m_liveGeneration = 0;
    QAtomicInteger<quint64> m_liveLatest;
    WordAt m_barWord;
    bool m_deleting = false;
    int m_deletes = 0;
    QPair<QString, QString> m_lastPair;
    QStringList m_swipeLikely;
    bool m_swipeLikelySet = false;

    QList<Mark> m_marks;
    bool m_tapped = false;
    qint64 m_ownEditAt;
    int m_lastCursor = -1;
    int m_lastAnchor = -1;
    QString m_clipDone;
};

} // namespace Pre

#endif // PRE_CONTROLLER_H
