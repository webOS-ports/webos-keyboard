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

#include "precontroller.h"
#include "precorrector.h"
#include "prekeyboardview.h"
#include "preswipedecoder.h"
#include "prewords.h"

#include <maliit/plugins/abstractinputmethodhost.h>

#include <QClipboard>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QSaveFile>
#include <QTimer>

#include <limits>

Q_LOGGING_CATEGORY(lcPre, "luneos.keyboard.pre", QtInfoMsg)

namespace Pre {
namespace {

//! A word the owner kept: as likely, to the swipe decoder, as a fairly common word.
const int LEARNED_SCORE = 10;
//! The longest word looked at around a tapped cursor.
const int MAX_TAPPED = 32;
//! A cursor move this soon after the keyboard changed the text is the keyboard's own.
const qint64 OWN_EDIT_MS = 400;
//! The keyboard's copy of the text is trusted over the application's this long after an edit.
const qint64 SYNC_GRACE_MS = 300;
//! Dictionaries kept in memory: the one in use, and the next and the one before.
const int KEEP_DICTIONARIES = 3;
//! Farther than this from the cursor, a mark isn't looked for in the text: it's dropped.
const int MAX_MARK_REACH = 2000;
const qint64 NEVER = std::numeric_limits<qint64>::min() / 2;

qint64 clockMs()
{
    static QElapsedTimer clock;
    if (!clock.isValid()) clock.start();
    return clock.elapsed();
}

bool isWordChar(QChar ch) { return ch.isLetter() || ch == QLatin1Char('\''); }

} // namespace

// --- The text, as the keyboard knows it -----------------------------------------------------

/*!
 * The field's text around the cursor. The application reports it (surroundingText)
 * only after a round trip, but the Composer reads back what it just wrote within
 * the same key, so the keyboard keeps its own copy: each edit is made to the copy
 * and sent on. A deletion is held back and sent with the commit after it, so a
 * correction reaches the application as one edit. The application's report is
 * taken again once it has had time to catch up.
 */
class Controller::Field : public Composer::Field
{
public:
    MAbstractInputMethodHost *host = nullptr;
    QString text;
    int cursor = 0;
    bool known = false;
    int pendingDelete = 0;
    qint64 editedAt = NEVER;

    //! The application's account of its text, unless the keyboard's own edits are newer.
    //! True when the cursor or text changed from what the keyboard had.
    bool sync(bool force)
    {
        if (!host) return false;
        QString t;
        int pos = 0;
        if (!host->surroundingText(t, pos)) { known = false; return false; }
        if (!force && clockMs() - editedAt < SYNC_GRACE_MS && !(t == text && pos == cursor))
            return false;
        const bool changed = !known || t != text || pos != cursor;
        text = t;
        cursor = std::max(0, std::min(pos, int(t.length())));
        known = true;
        return changed;
    }

    QString before(int n) override
    {
        const int from = std::max(0, cursor - n);
        return text.mid(from, cursor - from);
    }

    QString after(int n) override { return text.mid(cursor, n); }

    void remove(int n) override
    {
        n = std::min(n, cursor);
        if (n <= 0) return;
        text.remove(cursor - n, n);
        cursor -= n;
        pendingDelete += n;
        editedAt = clockMs();
    }

    void commit(const QString &s) override
    {
        if (host) host->sendCommitString(s, -pendingDelete, pendingDelete);
        pendingDelete = 0;
        text.insert(cursor, s);
        cursor += s.length();
        editedAt = clockMs();
    }

    //! A deletion with nothing committed after it: sent on its own.
    void flush()
    {
        if (pendingDelete > 0 && host) host->sendCommitString(QString(), -pendingDelete, pendingDelete);
        pendingDelete = 0;
    }

    //! Text the keyboard put in some other way (a key event): the copy follows it.
    void commitLocal(const QString &s)
    {
        text.insert(cursor, s);
        cursor += s.length();
        editedAt = clockMs();
    }

    void deletedBefore(int n)
    {
        n = std::min(n, cursor);
        text.remove(cursor - n, n);
        cursor -= n;
        editedAt = clockMs();
    }
};

struct Controller::Dictionary
{
    QSharedPointer<Corrector> corrector;
    QSharedPointer<SwipeDecoder> decoder;
    QStringList learned;
};

struct Controller::FamilyWords
{
    QSharedPointer<NextWords> next;
    QSharedPointer<EmojiWords> emoji;
};

Controller::Controller(QObject *parent)
    : QObject(parent),
      m_field(new Field),
      m_composer(m_field, clockMs),
      m_ownEditAt(NEVER)
{
    m_loading.setMaxThreadCount(1);
    m_suggest.setMaxThreadCount(1);
    m_live.setMaxThreadCount(1);
    m_composer.onLearned = [this](const QString &w) {
        QStringList learned = Settings::learned(m_prefs, m_language);
        if (!learned.contains(w.toLower())) learned << w.toLower();
        writePrefs(Settings::withLearned(m_prefs, m_language, learned));
        if (m_dictionary) m_dictionary->decoder->add(w, LEARNED_SCORE);
    };
    m_composer.onCorrected = [this](const QString &typed, const QString &fixed, int tail) {
        if (m_settings.vibrate && m_lunaCall)
            m_lunaCall(QStringLiteral("luna://com.palm.vibrate/vibrate"), QStringLiteral("{\"period\":0,\"duration\":28}"));
        // Once the whole key's work is done: then the word is marked.
        QTimer::singleShot(0, this, [this, typed, fixed, tail] { markCorrection(typed, fixed, tail); });
    };
    connect(this, &Controller::wordsLoaded, this, [this](quint64 generation) {
        if (generation != m_wordsGeneration) return;
        syncLearned();
        refreshBar();
    }, Qt::QueuedConnection);
    connect(this, &Controller::barReady, this,
            [this](quint64 gen, const QStringList &words, bool verbatim, bool emoji, bool tapped) {
        if (gen != m_barGeneration) return;
        if (tapped) m_barWord = wordAtCursor();
        show(words, verbatim, emoji);
    }, Qt::QueuedConnection);
    connect(this, &Controller::liveReady, this, [this](quint64 gen, const QStringList &found) {
        if (gen != m_liveGeneration || found.isEmpty() || !m_view) return;
        m_view->setSuggestions({ found.value(1), found.value(0), found.value(2) });
    }, Qt::QueuedConnection);
    if (QClipboard *cb = QGuiApplication::clipboard()) {
        connect(cb, &QClipboard::dataChanged, this, [this] {
            m_clipDone.clear();
            offerPaste();
        });
    }
}

Controller::~Controller()
{
    saveNextWords();
    m_loading.waitForDone();
    m_suggest.waitForDone();
    m_live.waitForDone();
    delete m_field;
}

qint64 Controller::now() const { return clockMs(); }

void Controller::setView(KeyboardView *view)
{
    m_view = view;
    if (m_view) {
        m_view->applySettings(m_settings);
        m_view->setWordsAllowed(m_prose);
        m_view->setSuggestions(m_bar);
        offerPaste();
    }
}

void Controller::setActive(bool active)
{
    if (active == m_active) return;
    m_active = active;
    qCInfo(lcPre, "Pre Keyboard %s", active ? "on" : "off");
    if (m_active) useLanguage(m_settings.language);
    else saveNextWords();
    Q_EMIT activeChanged();
}

void Controller::setPrefs(const QJsonObject &pre)
{
    const Settings s = Settings::fromJson(pre);
    const bool learnedChanged = pre.value(QLatin1String("learned")) != m_prefs.value(QLatin1String("learned"));
    m_prefs = pre;
    if (s != m_settings) {
        m_settings = s;
        if (m_view) m_view->applySettings(s);
    }
    if (m_active && s.language != m_language) useLanguage(s.language);
    if (learnedChanged) syncLearned();
    refreshBar();
}

void Controller::writePrefs(const QJsonObject &pre)
{
    m_prefs = pre;
    if (m_savePrefs) m_savePrefs(pre);
}

// --- The language's words -------------------------------------------------------------------

QStringList Controller::readLines(const QString &name) const
{
    QFile f(m_dataDir + QLatin1Char('/') + name);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(lcPre, "cannot read %s", qPrintable(f.fileName()));
        return {};
    }
    return QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
}

QSharedPointer<Controller::Dictionary> Controller::readDictionary(const Language &lang)
{
    QElapsedTimer t;
    t.start();
    QStringList all;
    for (const QString &d : lang.dictionaries()) all += readLines(d);
    QSharedPointer<Dictionary> dict(new Dictionary);
    dict->corrector.reset(new Corrector(all, lang));
    if (lang.has(Language::CONTRACTIONS)) dict->corrector->loadContractions(readLines(lang.asset(Language::CONTRACTIONS)));
    dict->decoder.reset(new SwipeDecoder(dict->corrector->entries()));
    qCWarning(lcPre, "%s: %d words in %lld ms", qPrintable(lang.id), dict->corrector->size(), t.elapsed());
    return dict;
}

QString Controller::nextWordsFile(const Language &lang) const
{
    return m_stateDir + QStringLiteral("/next_words_mine_") + lang.family() + QStringLiteral(".txt");
}

QSharedPointer<Controller::FamilyWords> Controller::readFamily(const Language &lang)
{
    QSharedPointer<FamilyWords> fam(new FamilyWords);
    fam->emoji.reset(new EmojiWords(readLines(lang.asset(Language::EMOJI_WORDS))));
    fam->next.reset(new NextWords(readLines(lang.asset(Language::NEXT_WORDS))));
    QFile mine(nextWordsFile(lang));
    if (mine.open(QIODevice::ReadOnly))
        fam->next->load(QString::fromUtf8(mine.readAll()).split(QLatin1Char('\n')));
    return fam;
}

/*!
 * Makes [lang] the language typed in. A dictionary is 60,000 to 90,000 words:
 * read off the keyboard's thread, with nothing corrected or suggested until it is in.
 */
void Controller::useLanguage(const Language &lang)
{
    m_language = lang;
    if (m_wordsFor == lang && m_dictionary) return;
    // What was learned in the language before is kept before its words are put away.
    saveNextWords();
    m_wordsFor = lang;
    const quint64 generation = ++m_wordsGeneration;
    m_composer.corrector.reset();
    m_dictionary.reset();
    m_family.reset();
    m_swipeGuesses.clear();
    m_swipeLikelySet = false;
    refreshBar();
    QList<Language> around;
    const QList<Language> &all = m_settings.languages;
    const int i = all.indexOf(lang);
    if (i >= 0 && all.size() > 1) {
        around << all[(i + 1) % all.size()];
        const Language back = all[(i - 1 + all.size()) % all.size()];
        if (back != around.first()) around << back;
    }
    // The loading thread owns the kept dictionaries; the result comes back queued.
    m_loading.start([this, lang, around, generation] {
        QSharedPointer<FamilyWords> fam;
        QSharedPointer<Dictionary> dict;
        {
            fam = m_families.value(lang.family());
            if (!fam) { fam = readFamily(lang); m_families.insert(lang.family(), fam); }
            dict = m_dictionaries.value(lang.id);
            if (!dict) { dict = readDictionary(lang); m_dictionaries.insert(lang.id, dict); }
            m_dictionaryOrder.removeAll(lang.id);
            m_dictionaryOrder << lang.id;
        }
        NextWords *next = fam->next.data();
        dict->corrector->follows = [next](const QString &a, const QString &b) { return next->follows(a, b); };
        QMetaObject::invokeMethod(this, [this, dict, fam, generation] {
            if (generation != m_wordsGeneration) return;   // flicked on again meanwhile
            m_dictionary = dict;
            m_family = fam;
            m_composer.corrector = dict->corrector;
            Q_EMIT wordsLoaded(generation);
        }, Qt::QueuedConnection);
        // The languages a flick would go to next, read ahead; the one in use stays newest.
        for (const Language &l : around) {
            if (generation != m_wordsGeneration) break;
            if (!m_dictionaries.contains(l.id)) {
                m_dictionaries.insert(l.id, readDictionary(l));
                m_dictionaryOrder.prepend(l.id);
            }
            if (!m_families.contains(l.family())) m_families.insert(l.family(), readFamily(l));
        }
        while (m_dictionaryOrder.size() > KEEP_DICTIONARIES) {
            const QString old = m_dictionaryOrder.takeFirst();
            if (old != lang.id) m_dictionaries.remove(old);
        }
    });
}

void Controller::onLanguageFlick(int step)
{
    const QList<Language> &all = m_settings.languages;
    if (all.size() < 2) return;
    const int n = all.size();
    const int i = all.indexOf(m_settings.language);
    onUseLanguage(all[((i + step) % n + n) % n].id);
}

void Controller::onUseLanguage(const QString &id)
{
    const Language next = Language::byId(id);
    if (!next.isValid() || next == m_settings.language) return;
    m_field->flush();
    m_composer.forget();
    m_settings.language = next;
    writePrefs(m_settings.toJson(m_prefs));
    if (m_view) {
        m_view->applySettings(m_settings);
        m_view->flashLanguage();
    }
    useLanguage(next);
}

void Controller::onUseLuneOSKeyboard()
{
    if (m_switchLayout) m_switchLayout(QStringLiteral("LuneOS"));
}

//! Words added or taken out in Text Assist (or added here): the keyboard follows.
void Controller::syncLearned()
{
    if (!m_dictionary) return;
    const QStringList now = Settings::learned(m_prefs, m_language);
    for (const QString &w : m_dictionary->learned)
        if (!now.contains(w)) { m_dictionary->corrector->unlearn(w); m_dictionary->decoder->remove(w); }
    for (const QString &w : now)
        if (!m_dictionary->learned.contains(w)) { m_dictionary->corrector->learn(w); m_dictionary->decoder->add(w, LEARNED_SCORE); }
    m_dictionary->learned = now;
}

void Controller::saveNextWords()
{
    if (!m_family || !m_wordsFor.isValid() || !m_family->next->dirty()) return;
    const QString text = m_family->next->save();
    QDir().mkpath(m_stateDir);
    QSaveFile f(nextWordsFile(m_wordsFor));
    if (f.open(QIODevice::WriteOnly)) {
        f.write(text.toUtf8());
        f.commit();
    }
}

// --- The field ------------------------------------------------------------------------------

void Controller::setProse(bool prose, bool quiet)
{
    m_prose = prose;
    m_quiet = quiet;
    if (m_view) m_view->setWordsAllowed(prose);
}

void Controller::startInput()
{
    m_field->host = m_host;
    m_field->pendingDelete = 0;
    m_field->editedAt = NEVER;
    m_field->sync(true);
    m_composer.reset();
    m_marks.clear();
    // Where the cursor is as the field opens, the owner put it (or left it): a word
    // it touches gets alternatives, as a word tapped would.
    m_tapped = true;
    m_lastCursor = m_field->known ? m_field->cursor : -1;
    if (m_view) {
        m_view->applySettings(m_settings);
        m_view->setWordsAllowed(m_prose);
        m_view->reset();
    }
    refreshBar();
    offerPaste();
}

void Controller::finishInput()
{
    m_field->flush();
    m_marks.clear();
    m_deleting = false;
    saveNextWords();
}

/*!
 * The application has told the keyboard about its text. A cursor that moved without
 * the keyboard moving it was moved by the owner: a tap on a corrected word switches
 * it, a tap on any word gets alternatives in the bar.
 */
void Controller::update()
{
    m_field->host = m_host;
    if (!m_field->sync(false)) return;
    const int cursor = m_field->cursor;
    const int was = m_lastCursor;
    m_lastCursor = cursor;
    m_swipeLikelySet = false;
    checkMarks();
    bool selection = false;
    if (m_host) {
        bool valid = false;
        selection = m_host->hasSelection(valid) && valid;
    }
    const bool own = now() - m_ownEditAt < OWN_EDIT_MS;
    if (!own && !selection && was >= 0 && cursor != was) {
        // A tap on a corrected word: the cursor lands in it, from somewhere else. It swaps.
        for (int i = 0; i < m_marks.size(); ++i) {
            const Mark &m = m_marks[i];
            if (cursor >= m.start && cursor <= m.end() && !(was >= m.start && was <= m.end())) {
                toggleMark(i, was);
                return;
            }
        }
    }
    // A selection, or the cursor taken elsewhere: the keyboard's last space is now ordinary.
    if (selection) { m_composer.forget(); m_tapped = false; }
    else if (!own && cursor != was) { m_composer.forget(); m_tapped = true; }
    if (m_view) m_view->updateCaps();
    refreshBar();
}

Composer::Rules Controller::rules() const
{
    Composer::Rules r;
    r.autocorrect = m_settings.autocorrect && m_prose && !m_quiet;
    r.doubleSpacePeriod = m_settings.doubleSpacePeriod && m_prose;
    r.prose = m_prose;
    return r;
}

void Controller::ownEdit()
{
    m_ownEditAt = now();
    m_tapped = false;
}

void Controller::sendKey(int key, Qt::KeyboardModifiers modifiers)
{
    if (!m_host) return;
    m_host->sendKeyEvent(QKeyEvent(QEvent::KeyPress, key, modifiers));
    m_host->sendKeyEvent(QKeyEvent(QEvent::KeyRelease, key, modifiers));
}

bool Controller::wantsCaps()
{
    // In a field for words, a capital at the start of the text and of a sentence.
    // (The field's own auto-capitalisation hint is opt-in under text-input, and
    // hardly any field sets it.)
    if (!m_prose) return false;
    QString b = m_field->before(MAX_TAPPED);
    if (!m_field->known) return false;
    int n = b.length();
    if (n == 0) return true;
    if (!b[n - 1].isSpace()) return false;
    while (n > 0 && b[n - 1].isSpace()) {
        if (b[n - 1] == QLatin1Char('\n')) return true;
        n--;
    }
    if (n == 0) return m_field->cursor <= MAX_TAPPED;
    return QStringLiteral(".!?").contains(b[n - 1]);
}

// --- What the keyboard reports --------------------------------------------------------------

void Controller::onText(const QString &text)
{
    ownEdit();
    m_composer.type(text, rules());
    m_field->flush();
    if (text == QLatin1String(" ") || (text.length() == 1 && QStringLiteral(".,?!:;").contains(text[0]))) learnPair();
    m_lastCursor = m_field->cursor;
    if (m_view) m_view->updateCaps();
    refreshBar();
}

void Controller::onBackspace(bool word)
{
    ownEdit();
    m_deletes++;
    if (!word && m_composer.backspace()) { m_field->flush(); m_lastCursor = m_field->cursor; refreshBar(); return; }
    if (!word && backspaceIntoMark()) { m_lastCursor = m_field->cursor; refreshBar(); return; }
    if (!word) {
        bool valid = false;
        const bool selection = m_host && m_host->hasSelection(valid) && valid;
        sendKey(Qt::Key_Backspace);
        if (!selection) m_field->deletedBefore(1);
        m_lastCursor = m_field->cursor;
        if (m_view) m_view->updateCaps();
        refreshBar();
        return;
    }
    // Held long enough, backspace takes the word before the cursor, and the spaces after it.
    const QString before = m_field->before(64);
    int end = before.length();
    while (end > 0 && before[end - 1].isSpace()) end--;
    int start = end;
    while (start > 0 && before[start - 1] != QLatin1Char(' ') && before[start - 1] != QLatin1Char('\n') &&
           before[start - 1] != QLatin1Char('\t'))
        start--;
    if (!before.isEmpty()) {
        m_field->remove(before.length() - start);
        m_field->flush();
    }
    m_lastCursor = m_field->cursor;
    if (m_view) m_view->updateCaps();
}

void Controller::onBackspaceHeld(bool held)
{
    m_deleting = held;
    m_deletes = 0;
    if (!held) refreshBar();
    else m_barGeneration++;
}

void Controller::onEnter()
{
    ownEdit();
    m_composer.beforeEnter(rules());
    m_field->flush();
    sendKey(Qt::Key_Return);
    m_field->commitLocal(QStringLiteral("\n"));
    m_lastCursor = m_field->cursor;
    if (m_view) m_view->updateCaps();
    refreshBar();
}

void Controller::onCursor(int dx, int dy, bool select)
{
    m_field->flush();
    m_composer.forget();
    ownEdit();
    const int key = dx > 0 ? Qt::Key_Right : dx < 0 ? Qt::Key_Left : dy > 0 ? Qt::Key_Down : Qt::Key_Up;
    sendKey(key, select ? Qt::ShiftModifier : Qt::NoModifier);
    // Where the cursor goes, the application says.
    m_field->editedAt = NEVER;
}

void Controller::onToggleStyle()
{
    m_settings.style = m_settings.style == Style::Orange ? Style::White : Style::Orange;
    writePrefs(m_settings.toJson(m_prefs));
    if (m_view) m_view->applySettings(m_settings);
}

void Controller::onToggleHold()
{
    m_settings.holdForMore = !m_settings.holdForMore;
    writePrefs(m_settings.toJson(m_prefs));
    if (m_view) m_view->applySettings(m_settings);
}

void Controller::onOpenSettings()
{
    if (m_host) m_host->notifyImInitiatedHiding();
    if (m_lunaCall)
        m_lunaCall(QStringLiteral("luna://com.webos.service.applicationManager/launch"),
                   QStringLiteral("{\"id\":\"org.webosports.app.settings\",\"params\":{\"page\":\"TextAssist\"}}"));
}

void Controller::hapticKey()
{
    if (m_lunaCall)
        m_lunaCall(QStringLiteral("luna://com.palm.vibrate/vibrate"), QStringLiteral("{\"period\":0,\"duration\":12}"));
}

void Controller::hapticLongPress()
{
    if (m_lunaCall)
        m_lunaCall(QStringLiteral("luna://com.palm.vibrate/vibrate"), QStringLiteral("{\"period\":0,\"duration\":20}"));
}

QString Controller::emojiFile() const { return m_dataDir + QStringLiteral("/emoji.txt"); }

QStringList Controller::recentEmoji() const { return Settings::recentEmoji(m_prefs); }

void Controller::addRecentEmoji(const QString &e)
{
    writePrefs(Settings::withRecentEmoji(m_prefs, e));
}

// --- Swipe ----------------------------------------------------------------------------------

QString Controller::cased(const QString &word, int caps) const
{
    if (caps == KeyboardView::CAPS_ALL) return word.toUpper();
    if (caps == KeyboardView::CAPS_FIRST && !word.isEmpty()) return word[0].toUpper() + word.mid(1);
    return word;
}

/*!
 * The words that often come next after the text before the cursor, for the swipe
 * decoder: looked up once a swipe, as it starts.
 */
QStringList Controller::swipeContext()
{
    if (m_swipeLikelySet) return m_swipeLikely;
    const QString ctx = m_composer.nextContext();
    m_swipeLikely = (!m_family || ctx.isEmpty()) ? QStringList() : m_family->next->predict(ctx, 8);
    m_swipeLikelySet = true;
    return m_swipeLikely;
}

/*!
 * Guesses while the finger is still moving: the word it would be if lifted now, in
 * the middle, between the next two. Worked out on a thread of its own so the trail
 * never stutters; a newer piece of the swipe, or the swipe's end, makes an older one moot.
 */
void Controller::onSwipeProgress(const QVector<float> &xs, const QVector<float> &ys, int caps)
{
    if (!m_dictionary || !m_view) return;
    SwipeDecoder::Layout layout;
    if (!m_view->swipeLayout(layout)) return;
    const quint64 gen = ++m_liveGeneration;
    m_liveLatest.storeRelease(gen);
    const QStringList likely = swipeContext();
    QSharedPointer<SwipeDecoder> d = m_dictionary->decoder;
    m_live.clear();
    m_live.start([this, d, xs, ys, layout, likely, gen, caps] {
        if (m_liveLatest.loadAcquire() != gen) return;
        QStringList found = d->decode(xs, ys, layout, 4, likely, [this, gen] { return m_liveLatest.loadAcquire() != gen; });
        for (QString &w : found) w = cased(w, caps);
        Q_EMIT liveReady(gen, found);
    });
}

void Controller::onSwipe(const QVector<float> &xs, const QVector<float> &ys, int caps)
{
    m_liveGeneration++;
    m_liveLatest.storeRelease(m_liveGeneration);
    m_live.clear();
    SwipeDecoder::Layout layout;
    if (!m_dictionary || !m_view || !m_view->swipeLayout(layout)) { m_swipeLikelySet = false; return; }
    QElapsedTimer t;
    t.start();
    QStringList found = m_dictionary->decoder->decode(xs, ys, layout, 4, swipeContext());
    m_swipeLikelySet = false;
    qCDebug(lcPre, "swipe: %d points, %lld ms, %s", int(xs.size()), t.elapsed(), qPrintable(found.join(QLatin1Char(' '))));
    if (found.isEmpty()) { refreshBar(); return; }
    ownEdit();
    for (QString &w : found) w = cased(w, caps);
    m_swipeGuesses = found;
    m_composer.swipe(found.first(), rules());
    m_field->flush();
    m_lastCursor = m_field->cursor;
    learnPair();
    refreshBar();
}

// --- The suggestion bar -----------------------------------------------------------------------

/*!
 * What the bar shows. After a swipe: the word swiped, in the middle, between the
 * next two guesses. While a word is being typed: what space would make of it in
 * the middle, the word as typed on the left when that differs, and other likely
 * words. Before one: the likeliest next words. Otherwise nothing.
 */
void Controller::refreshBar()
{
    if (!m_view) return;
    // Whatever is still being worked out for the bar is out of date now.
    const quint64 gen = ++m_barGeneration;
    m_barWord = WordAt();
    // While backspace is down, every deletion would look the word up again: wait till it lets go.
    if (m_deleting) {
        if (m_deletes > 1) show({}, false, false);
        return;
    }
    if (!m_settings.showsBar() || !m_prose) { show({}, false, false); return; }
    // The cursor moved away from the swiped word: it is no longer the one to replace.
    if (!m_composer.swiped().isEmpty() && !m_composer.swipedIsLast()) m_composer.forget();
    const WordAt at = wordAtCursor();
    QSharedPointer<Corrector> c = m_composer.corrector;
    // A word tapped on, or the cursor inside one: other words it might have been.
    if (at.valid && c && (m_tapped || !at.after.isEmpty())) {
        const QString word = at.word();
        m_suggest.start([this, c, word, gen] {
            const QStringList alt = c->alternatives(word);
            Q_EMIT barReady(gen, { alt.value(1), alt.value(0), alt.value(2) }, false, false, true);
        });
        return;
    }
    const bool emojiOn = m_settings.emojiSuggestions;
    const QString sw = m_composer.swiped();
    if (!sw.isEmpty()) {
        QStringList others;
        for (const QString &g : m_swipeGuesses) if (g != sw) others << g;
        const QStringList words = { others.value(0), sw, others.value(1) };
        const QString e = (emojiOn && m_family) ? m_family->emoji->forWord(sw) : QString();
        if (!e.isEmpty()) show({ words[0], words[1], e }, false, true);
        else show(words, false, false);
        return;
    }
    const QString typed = m_composer.currentWord();
    const QString typedAfter = m_composer.wordContext();
    if (!c) { show({}, false, false); return; }
    if (typed.isEmpty()) { show(predict(), false, false); return; }
    QSharedPointer<EmojiWords> emoji = (emojiOn && m_family) ? m_family->emoji : QSharedPointer<EmojiWords>();
    // Looking a word up can take a good part of a second on an old phone: on a
    // thread of its own, so typing never waits for it. The bar keeps what it shows till then.
    m_suggest.start([this, c, typed, typedAfter, emoji, gen] {
        const QStringList list = c->suggestions(typed, typedAfter);
        const QString best = list.value(0, typed);
        QStringList others;
        for (const QString &w : list) if (w != best && w != typed) others << w;
        const bool verbatim = best != typed;
        const QStringList words = verbatim ? QStringList{ typed, best, others.value(0) }
                                           : QStringList{ others.value(0), best, others.value(1) };
        const QString e = (emoji && !words[1].isEmpty()) ? emoji->forWord(words[1]) : QString();
        if (!e.isEmpty()) Q_EMIT barReady(gen, { words[0], words[1], e }, verbatim, true, false);
        else Q_EMIT barReady(gen, words, verbatim, false, false);
    });
}

void Controller::show(const QStringList &words, bool verbatim, bool emoji)
{
    m_bar = { words.value(0), words.value(1), words.value(2) };
    m_barVerbatim = verbatim;
    m_barEmoji = emoji;
    if (m_view) m_view->setSuggestions(m_bar);
}

/*!
 * Before a word is typed: the three likeliest next words, the likeliest in the
 * middle, with a capital where the field wants one. Empty when there is nothing to go on.
 */
QStringList Controller::predict()
{
    if (!m_family || !m_composer.corrector) return {};
    const QString ctx = m_composer.nextContext();
    if (ctx.isEmpty()) return {};
    QSharedPointer<Corrector> c = m_composer.corrector;
    const QStringList found = m_family->next->predict(ctx, 3, [c](const QString &w) { return c->isKnown(w); });
    if (found.isEmpty()) return {};
    const bool caps = wantsCaps();
    QStringList w;
    for (const QString &f : found) w << (caps && !f.isEmpty() ? f[0].toUpper() + f.mid(1) : f);
    return { w.value(1), w.value(0), w.value(2) };
}

//! A word was just finished: remember what it followed, so it can be offered there next time.
void Controller::learnPair()
{
    if (!m_family || !m_composer.corrector || !m_prose || m_quiet) return;
    const QPair<QString, QString> pair = m_composer.finishedPair();
    if (pair.first.isEmpty()) return;
    const QString spelt = m_composer.corrector->spelling(pair.second);
    if (spelt.isEmpty()) return;
    const QPair<QString, QString> now(pair.first, spelt);
    if (now == m_lastPair) return;
    m_lastPair = now;
    m_family->next->learn(pair.first, spelt);
}

void Controller::onPick(int i)
{
    const QString word = m_bar.value(i);
    if (word.isEmpty()) return;
    if (m_barWord.valid) { replaceTapped(m_barWord, word); return; }
    ownEdit();
    const bool learn = i == 0 && m_barVerbatim && m_composer.corrector && !m_composer.corrector->isKnown(word);
    m_composer.pick(word, learn);
    m_field->flush();
    m_lastCursor = m_field->cursor;
    if (i == 2 && m_barEmoji) addRecentEmoji(word);
    learnPair();
    if (m_view) m_view->updateCaps();
    refreshBar();
}

//! The word the cursor is in or touching, when it is a plain word standing on its own.
Controller::WordAt Controller::wordAtCursor()
{
    if (!m_field->known) return WordAt();
    const QString b = m_field->before(MAX_TAPPED + 1);
    const QString a = m_field->after(MAX_TAPPED + 1);
    int i = b.length();
    while (i > 0 && isWordChar(b[i - 1])) i--;
    int j = 0;
    while (j < a.length() && isWordChar(a[j])) j++;
    if (i == b.length() && j == 0) return WordAt();
    if (i > 0 && (b[i - 1].isLetterOrNumber() || QStringLiteral("@#_/.-").contains(b[i - 1]))) return WordAt();
    if (i == 0 && b.length() > MAX_TAPPED) return WordAt();
    if (j < a.length() && (a[j].isDigit() || QStringLiteral("@_/").contains(a[j]))) return WordAt();
    if (j == a.length() && a.length() > MAX_TAPPED) return WordAt();
    WordAt w;
    w.before = b.mid(i);
    w.after = a.left(j);
    QString core = w.word();
    while (core.startsWith(QLatin1Char('\''))) core.remove(0, 1);
    while (core.endsWith(QLatin1Char('\''))) core.chop(1);
    w.valid = !core.isEmpty() && w.word()[0].isLetter();
    return w;
}

/*!
 * A word picked for a tapped one takes its place, the cursor at its end. The bar
 * goes on offering others for it, the word it replaced among them.
 */
void Controller::replaceTapped(const WordAt &at, const QString &word)
{
    const WordAt now = wordAtCursor();
    if (!now.valid || now.before != at.before || now.after != at.after) { refreshBar(); return; }
    m_ownEditAt = this->now();
    if (m_host) m_host->sendCommitString(word, -at.before.length(), at.before.length() + at.after.length());
    m_field->text.replace(m_field->cursor - at.before.length(), at.before.length() + at.after.length(), word);
    m_field->cursor = m_field->cursor - at.before.length() + word.length();
    m_field->editedAt = clockMs();
    m_lastCursor = m_field->cursor;
    m_tapped = true;
    m_composer.forget();
    refreshBar();
}

bool Controller::canAdd(const QString &word)
{
    if (!m_composer.corrector || word.isEmpty() || !word[0].isLetter()) return false;
    for (QChar c : word) if (!c.isLetter() && c != QLatin1Char('\'')) return false;
    return !m_composer.corrector->isKnown(word);
}

void Controller::onAddWord(const QString &word)
{
    if (m_composer.corrector) m_composer.corrector->learn(word);
    QStringList learned = Settings::learned(m_prefs, m_language);
    if (!learned.contains(word.toLower())) learned << word.toLower();
    writePrefs(Settings::withLearned(m_prefs, m_language, learned));
    if (m_dictionary) {
        m_dictionary->decoder->add(word.toLower(), LEARNED_SCORE);
        m_dictionary->learned = learned;
    }
    refreshBar();
}

// --- Corrected words ------------------------------------------------------------------------

/*!
 * Autocorrect just made [fixed] of [typed], [tail] characters before the cursor. It
 * is remembered by where it is in the text, so a tap on it switches it back, and a
 * backspace into it puts back what was typed. (On Android it is also underlined;
 * LuneOS's applications don't say where their words are on screen.)
 */
void Controller::markCorrection(const QString &typed, const QString &fixed, int tail)
{
    const QString before = m_field->before(fixed.length() + tail);
    if (before.length() != fixed.length() + tail || !before.startsWith(fixed)) return;
    const int start = m_field->cursor - tail - fixed.length();
    if (start < 0) return;
    m_marks.append(Mark{ start, fixed, typed, fixed });
}

/*!
 * Backspace is about to take the last letter of a word autocorrect changed: it puts
 * back what was typed instead, and the next backspace deletes from there.
 */
bool Controller::backspaceIntoMark()
{
    const int cursor = m_field->cursor;
    for (int i = 0; i < m_marks.size(); ++i) {
        const Mark m = m_marks[i];
        if (m.end() != cursor || m.text != m.fixed) continue;
        if (m_field->before(m.text.length()) != m.text) return false;
        m_marks.removeAt(i);
        m_composer.forget();
        m_field->remove(m.text.length());
        m_field->commit(m.typed);
        return true;
    }
    return false;
}

/*!
 * A mark the cursor is at or before may have been typed over, deleted or moved
 * along by new text: each such one is checked against the text, and dropped unless
 * it still holds its word.
 */
void Controller::checkMarks()
{
    const int cursor = m_field->cursor;
    for (int i = m_marks.size() - 1; i >= 0; --i) {
        const Mark &m = m_marks[i];
        if (m.end() < cursor - 1) continue;
        bool ok;
        if (m.end() - cursor > MAX_MARK_REACH || cursor - m.start > MAX_MARK_REACH) ok = false;
        else ok = m.start >= 0 && m.end() <= m_field->text.length() && m_field->text.mid(m.start, m.text.length()) == m.text;
        if (!ok) m_marks.removeAt(i);
    }
}

/*!
 * The owner tapped a corrected word: it changes to the other (what was typed, or
 * back to the correction), and the cursor goes back to [from], where it was before
 * the tap, so typing carries on there and the next tap on the word swaps it again.
 */
void Controller::toggleMark(int i, int from)
{
    Mark m = m_marks[i];
    const QString other = m.text == m.fixed ? m.typed : m.fixed;
    const int cursor = m_field->cursor;
    m_composer.forget();
    ownEdit();
    // Replace the word, from wherever the tap left the cursor inside it.
    if (m_host) m_host->sendCommitString(other, m.start - cursor, m.text.length());
    m_field->text.replace(m.start, m.text.length(), other);
    m_field->cursor = m.start + other.length();
    m_field->editedAt = clockMs();
    // Where the owner was, moved along by the change in length if it was after the word.
    const int back = from >= m.end() ? from + other.length() - m.text.length()
                   : from <= m.start ? from : m.start + other.length();
    if (m_host && back != m_field->cursor) {
        m_host->setSelection(back, 0);
        m_field->cursor = back;
    }
    m.text = other;
    m_marks[i] = m;
    m_lastCursor = m_field->cursor;
    // Showing what was typed, the keyboard offers to keep it.
    if (other == m.typed && canAdd(m.typed) && m_view) m_view->ask(m.typed);
    refreshBar();
}

// --- Paste ----------------------------------------------------------------------------------

/*!
 * The clipboard is offered while it holds something not yet pasted: the paste icon
 * stays in the bar, through typing and from field to field, till it is tapped. A
 * new copy or cut always is.
 */
void Controller::offerPaste()
{
    if (!m_view) return;
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    const QString key = t.isEmpty() ? QString()
        : QString::fromLatin1(QCryptographicHash::hash(t.toUtf8(), QCryptographicHash::Sha1).toHex());
    m_view->setPasteOffered(!t.isEmpty() && key != m_clipDone && m_settings.showsBar());
}

void Controller::onPaste()
{
    QClipboard *cb = QGuiApplication::clipboard();
    const QString t = cb ? cb->text() : QString();
    if (m_view) m_view->setPasteOffered(false);
    if (t.isEmpty()) return;
    m_clipDone = QString::fromLatin1(QCryptographicHash::hash(t.toUtf8(), QCryptographicHash::Sha1).toHex());
    ownEdit();
    m_composer.forget();
    m_field->commit(t);
    m_lastCursor = m_field->cursor;
    refreshBar();
}

} // namespace Pre
