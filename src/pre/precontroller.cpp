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

#include "precontroller.h"
#include "preklipy.h"
#include "prekeyboardview.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QDir>
#include <QGuiApplication>
#include <QLocale>
#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(lcPre, "luneos.keyboard.pre", QtInfoMsg)

namespace Pre {
namespace {

const QString SPACE = QStringLiteral("space");
const QString RETURN = QStringLiteral("return");
const QString BACKSPACE = QStringLiteral("backspace");

QString sha1(const QString &t)
{
    return QString::fromLatin1(QCryptographicHash::hash(t.toUtf8(), QCryptographicHash::Sha1).toHex());
}

} // namespace

Controller::Controller(QObject *parent)
    : QObject(parent)
{
}

Controller::~Controller() = default;

void Controller::setStateDir(const QString &dir)
{
    QDir().mkpath(dir);
    Klipy::instance().setStateDir(dir);
}

void Controller::setView(KeyboardView *view)
{
    m_view = view;
    if (m_view) {
        applySettings();
        showBar();
        offerPaste();
    }
}

void Controller::setActive(bool active)
{
    if (active == m_active) return;
    m_active = active;
    qCInfo(lcPre, "Pre Keyboard %s", active ? "on" : "off");
    Q_EMIT activeChanged();
}

void Controller::setPrefs(const QJsonObject &pre)
{
    m_prefs = pre;
    const Settings s = Settings::fromJson(pre, m_settings);
    if (s != m_settings) {
        m_settings = s;
        applySettings();
    }
}

void Controller::setKeyboardState(bool suggestions, bool sound, const QStringList &enabledLanguages,
                                  const QString &activeLanguage)
{
    Settings s = m_settings;
    s.suggestions = suggestions;
    s.sound = sound;
    s.languages.clear();
    for (const QString &id : enabledLanguages) {
        const Language l = Language::byId(id);
        if (l.isValid() && !s.languages.contains(l)) s.languages << l;
    }
    const Language now = Language::orEnglish(activeLanguage);
    if (!s.languages.contains(now)) s.languages.prepend(now);
    s.language = now;
    if (s != m_settings) {
        m_settings = s;
        applySettings();
        showBar();
    }
}

void Controller::applySettings()
{
    if (!m_view) return;
    m_view->applySettings(m_settings);
    m_view->setWordsAllowed(m_wordsAllowed);
    m_view->setDigits(m_digits);
}

void Controller::setDigits(const QString &allowed)
{
    m_digits = allowed;
    if (m_view) m_view->setDigits(allowed);
}

void Controller::setWordsAllowed(bool allowed)
{
    if (allowed == m_wordsAllowed) return;
    m_wordsAllowed = allowed;
    if (m_view) m_view->setWordsAllowed(allowed);
    showBar();
}

void Controller::writePrefs(const QJsonObject &pre)
{
    m_prefs = pre;
    if (m_bridge.savePrefs) m_bridge.savePrefs(pre);
}

void Controller::startInput()
{
    m_deleting = false;
    if (m_view) {
        applySettings();
        m_view->reset();
    }
    showBar();
    offerPaste();
}

void Controller::finishInput()
{
    if (m_view) m_view->hidden();
    if (m_deleting) {
        m_deleting = false;
        if (m_bridge.cancelRepeat) m_bridge.cancelRepeat();
    }
}

void Controller::update()
{
    if (m_view) m_view->updateCaps();
    offerPaste();
}

bool Controller::wantsCaps()
{
    return m_bridge.autoCaps && m_bridge.autoCaps();
}

// --- The bar --------------------------------------------------------------------------------

void Controller::setCandidates(const QString &typed, const QString &primary, const QStringList &words)
{
    m_typed = typed;
    m_primary = primary;
    m_candidates = words;
    showBar();
}

/*!
 * The Pre's bar: what space would put in, in the middle. While a word is typed
 * and that is a correction, the word as typed is on the left, to keep it; the
 * next likeliest fills the other places.
 */
void Controller::showBar()
{
    QStringList bar;
    if (m_settings.showsBar() && m_wordsAllowed) {
        // What space puts in: the correction, else the word as typed. With no
        // word typed, the likeliest next word.
        const QString best = !m_primary.isEmpty() ? m_primary
                           : !m_typed.isEmpty() ? m_typed : m_candidates.value(0);
        QStringList others;
        for (const QString &w : m_candidates)
            if (!w.isEmpty() && w != best && w != m_typed && !others.contains(w)) others << w;
        if (!m_typed.isEmpty() && !best.isEmpty() && best != m_typed)
            bar = { m_typed, best, others.value(0) };
        else
            bar = { others.value(0), best, others.value(1) };
    }
    m_bar = { bar.value(0), bar.value(1), bar.value(2) };
    if (m_view) m_view->setSuggestions(m_bar);
}

void Controller::onPick(int i, int caps)
{
    QString w = m_bar.value(i);
    if (w.isEmpty() || !m_bridge.pick) return;
    if (caps == KeyboardView::CAPS_ALL) w = QLocale(m_settings.language.id).toUpper(w);
    else if (caps == KeyboardView::CAPS_FIRST) w = w.left(1).toUpper() + w.mid(1);
    m_bridge.pick(w);
}

bool Controller::canAdd(const QString &word)
{
    return !word.isEmpty() && m_bridge.canAdd && m_bridge.canAdd(word);
}

void Controller::onAddWord(const QString &word)
{
    if (m_bridge.addWord) m_bridge.addWord(word);
}

// --- Keys -----------------------------------------------------------------------------------

void Controller::onText(const QString &text)
{
    if (text.isEmpty()) return;
    // A character goes in as a LuneOS key's would, to be predicted and corrected;
    // anything longer (an emoji, an emoticon) as it is.
    if (text == QLatin1String(" ")) {
        if (m_bridge.press) m_bridge.press(text, SPACE);
        if (m_bridge.release) m_bridge.release(text, SPACE);
    } else if (text.length() == 1) {
        if (m_bridge.press) m_bridge.press(text, QString());
        if (m_bridge.release) m_bridge.release(text, QString());
    } else if (m_bridge.commit) {
        m_bridge.commit(text);
    }
}

void Controller::onBackspace(bool)
{
    if (m_bridge.press) m_bridge.press(QString(), BACKSPACE);
    if (m_bridge.release) m_bridge.release(QString(), BACKSPACE);
}

void Controller::onBackspaceHeld(bool held, bool lifted)
{
    // The editor repeats a held backspace, and goes over to whole words, as it
    // does for LuneOS's own keys.
    if (held) {
        m_deleting = true;
        if (m_bridge.press) m_bridge.press(QString(), BACKSPACE);
        return;
    }
    if (!m_deleting) return;
    m_deleting = false;
    if (lifted) {
        if (m_bridge.release) m_bridge.release(QString(), BACKSPACE);
    } else if (m_bridge.cancelRepeat) {
        m_bridge.cancelRepeat();
    }
}

void Controller::onEnter()
{
    if (m_bridge.press) m_bridge.press(QString(), RETURN);
    if (m_bridge.release) m_bridge.release(QString(), RETURN);
}

void Controller::onCursor(int dx, int dy, bool select)
{
    if (m_bridge.cursor) m_bridge.cursor(dx, dy, select);
}

void Controller::onSwipe(const QVector<float> &, const QVector<float> &, int)
{
    // Swipe typing waits for the language plugins' words (Settings::swipe is off).
}

void Controller::onSwipeProgress(const QVector<float> &, const QVector<float> &, int)
{
}

// --- Settings and languages -----------------------------------------------------------------

void Controller::onToggleStyle()
{
    m_settings.style = m_settings.style == Style::Orange ? Style::White : Style::Orange;
    writePrefs(m_settings.toJson(m_prefs));
    applySettings();
}

void Controller::onToggleHold()
{
    m_settings.holdForMore = !m_settings.holdForMore;
    writePrefs(m_settings.toJson(m_prefs));
    applySettings();
}

void Controller::onLanguageFlick(int step)
{
    const QList<Language> &langs = m_settings.languages;
    if (langs.size() < 2 || !m_bridge.useLanguage) return;
    const int at = std::max(0, int(langs.indexOf(m_settings.language)));
    const int n = int(langs.size());
    m_bridge.useLanguage(langs[((at + step) % n + n) % n].id);
}

void Controller::onUseLanguage(const QString &id)
{
    if (m_bridge.useLanguage && Language::byId(id).isValid()) m_bridge.useLanguage(id);
}

void Controller::onOpenSettings()
{
    if (m_bridge.hide) m_bridge.hide();
    if (m_bridge.luna)
        m_bridge.luna(QStringLiteral("palm://com.webos.service.applicationManager/launch"),
                      QStringLiteral("{\"id\":\"org.webosports.app.settings.textassist\"}"));
}

void Controller::vibrate(int ms)
{
    if (!m_settings.vibrate || !m_bridge.luna) return;
    m_bridge.luna(QStringLiteral("palm://com.palm.vibrate/vibrate"),
                  QStringLiteral("{\"period\":0,\"duration\":%1}").arg(ms));
}

void Controller::hapticKey() { vibrate(12); }

void Controller::hapticLongPress() { vibrate(20); }

// --- Emoji, paste and GIFs ------------------------------------------------------------------

QString Controller::emojiFile() const { return m_dataDir + QStringLiteral("/emoji.txt"); }

QStringList Controller::recentEmoji() const { return Settings::recentEmoji(m_prefs); }

void Controller::addRecentEmoji(const QString &e)
{
    writePrefs(Settings::withRecentEmoji(m_prefs, e));
}

void Controller::offerPaste()
{
    if (!m_view) return;
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    m_view->setPasteOffered(!t.isEmpty() && sha1(t) != m_clipDone && m_settings.showsBar() && m_wordsAllowed);
}

void Controller::onPaste()
{
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    if (m_view) m_view->setPasteOffered(false);
    if (t.isEmpty()) return;
    m_clipDone = sha1(t);
    if (m_bridge.commit) m_bridge.commit(t);
}

/*!
 * A GIF or sticker tapped on the emoji page. LuneOS's applications take no
 * pictures from a keyboard, so its link is typed in, and KLIPY is told it was shared.
 */
void Controller::onMedia(int kind, const QString &link, const QString &slug, const QString &query)
{
    if (link.isEmpty()) return;
    if (m_bridge.commit) m_bridge.commit(link);
    Klipy::Item item;
    item.slug = slug;
    Klipy::instance().shared(kind == int(Klipy::Kind::Gif) ? Klipy::Kind::Gif : Klipy::Kind::Sticker, item, query);
}

} // namespace Pre
