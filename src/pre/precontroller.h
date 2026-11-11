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

#include "presettings.h"

#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVector>

#include <functional>

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
    //! A word from the bar: it replaces the one being typed.
    std::function<void(const QString &word)> pick;
    //! Text put in as it is, after the word being typed: emoji, a paste, a link.
    std::function<void(const QString &text)> commit;
    //! The cursor moved by a key, extending the selection when [select].
    std::function<void(int dx, int dy, bool select)> cursor;
    //! Whether the next letter starts a sentence.
    std::function<bool()> autoCaps;
    //! Whether [word] could be added to the owner's dictionary: it is not known.
    std::function<bool(const QString &word)> canAdd;
    std::function<void(const QString &word)> addWord;
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
    //! The Pre Keyboard is the keyboard: set by Keyboard.qml on a phone; a tablet keeps LuneOS's own.
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    // --- From the input method ---------------------------------------------------------

    void setBridge(const Bridge &bridge) { m_bridge = bridge; }
    //! Where emoji.txt is.
    void setDataDir(const QString &dir) { m_dataDir = dir; }
    //! Where KLIPY keeps its state (its call count and customer id).
    void setStateDir(const QString &dir);

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
    //! Words can't be taken out of LuneOS's dictionaries from the bar.
    bool canRemove(const QString &) { return false; }
    void onRemoveWord(const QString &) {}
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

private:
    void applySettings();
    void showBar();
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
    QStringList m_bar = { QString(), QString(), QString() };
    bool m_deleting = false;
    QString m_clipDone;
};

} // namespace Pre

#endif // PRE_CONTROLLER_H
