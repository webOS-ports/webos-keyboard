/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
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

// The Pre Keyboard on LuneOS's own words: a key for every language plugin's
// letters, and its keys handed to the same editor as every LuneOS layout's.

#include "precontroller.h"
#include "preemojiwords.h"
#include "prekeyboardview.h"
#include "prekeymap.h"
#include "prelanguage.h"
#include "presettings.h"
#include "preswipedecoder.h"
#include "preswipewords.h"

#include "abstracttexteditor.h"
#include "logic/abstractlanguagefeatures.h"
#include "logic/eventhandler.h"
#include "models/text.h"

#include <QDir>
#include <QFileInfo>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>
#include <QClipboard>

#include <cmath>

using namespace MaliitKeyboard;

namespace {

// --- A LuneOS editor that counts what reaches the application ---------------------------------

class NoFeatures : public AbstractLanguageFeatures
{
public:
    bool activateAutoCaps(const QString &) const override { return false; }
    QString appendixForReplacedPreedit(const QString &) const override { return QString(); }
};

class NoWords : public Logic::AbstractWordEngine
{
public:
    AbstractLanguageFeatures *languageFeature() override { return &features; }

private:
    WordCandidateList fetchCandidates(Model::Text *) override { return WordCandidateList(); }
    NoFeatures features;
};

//! Offers [words] for whatever is typed, the first as what space puts in, as WordEngine does.
class SetWords : public Logic::AbstractWordEngine
{
public:
    AbstractLanguageFeatures *languageFeature() override { return &features; }
    QStringList words;
    //! The owner's dictionary.
    QStringList own;
    QStringList userWords() override { return own; }
    //! The words spelt wrong.
    QStringList misspelled;
    bool isMisspelled(const QString &w) override { return misspelled.contains(w) || isBlocked(w); }
    //! What follows anything.
    QStringList next;

protected:
    QStringList predictNext(const QString &) override { return next; }

private:
    WordCandidateList fetchCandidates(Model::Text *text) override
    {
        WordCandidateList out;
        for (const QString &w : words) out << WordCandidate(WordCandidate::SourcePrediction, w);
        text->setPrimaryCandidate(words.value(0));
        return out;
    }
    NoFeatures features;
};

//! Offers the same two words for whatever is typed: [first], then "pi".
class TwoWords : public Logic::AbstractWordEngine
{
public:
    AbstractLanguageFeatures *languageFeature() override { return &features; }
    //! Another language's words.
    QString first = QStringLiteral("pa");

private:
    WordCandidateList fetchCandidates(Model::Text *) override
    {
        return WordCandidateList{ WordCandidate(WordCandidate::SourcePrediction, first),
                                  WordCandidate(WordCandidate::SourcePrediction, QStringLiteral("pi")) };
    }
    NoFeatures features;
};

//! An editor whose application is a string: what it commits goes on the end, a backspace takes one off.
class StringEditor : public AbstractTextEditor
{
public:
    explicit StringEditor(Logic::AbstractWordEngine *words)
        : AbstractTextEditor(EditorOptions(), new Model::Text, words)
    {}

    QString committed;
    QString preedit;

    //! As the application would report it back: the text before the cursor.
    void report()
    {
        text()->setSurrounding(committed);
        text()->setSurroundingOffset(uint(committed.length()));
    }

private:
    void sendPreeditString(const QString &p, Model::Text::PreeditFace, const Replacement &) override { preedit = p; }
    void sendCommitString(const QString &s) override { committed += s; preedit.clear(); }
    void sendKeyEvent(const QKeyEvent &ev) override
    {
        if (ev.type() != QEvent::KeyPress) return;
        if (ev.key() == Qt::Key_Backspace) committed.chop(1);
        else if (ev.key() == Qt::Key_Return) committed += QStringLiteral("\r");
    }
    void invokeAction(const QString &, const QKeySequence &) override {}
};

class CountingEditor : public AbstractTextEditor
{
public:
    explicit CountingEditor(Logic::AbstractWordEngine *words = new NoWords)
        : AbstractTextEditor(EditorOptions(), new Model::Text, words)
    {}

    int backspaces = 0;

private:
    void sendPreeditString(const QString &, Model::Text::PreeditFace, const Replacement &) override {}
    void sendCommitString(const QString &) override {}
    void sendKeyEvent(const QKeyEvent &ev) override
    {
        if (ev.type() == QEvent::KeyPress && ev.key() == Qt::Key_Backspace)
            ++backspaces;
    }
    void invokeAction(const QString &, const QKeySequence &) override {}
};

// --- A bridge that writes down what the controller asked for -----------------------------------

struct Recorder
{
    QStringList calls;
    //! The "pre" object last saved.
    QJsonObject saved;
    bool autoCaps = false;
    bool unknown = true;

    Pre::Bridge bridge()
    {
        Pre::Bridge b;
        b.press = [this](const QString &l, const QString &a) { calls << QStringLiteral("press %1|%2").arg(l, a); };
        b.release = [this](const QString &l, const QString &a) { calls << QStringLiteral("release %1|%2").arg(l, a); };
        b.cancelRepeat = [this] { calls << QStringLiteral("cancel"); };
        b.pick = [this](const QString &w) { calls << QStringLiteral("pick ") + w; };
        b.commit = [this](const QString &t) { calls << QStringLiteral("commit ") + t; };
        b.cursor = [this](int dx, int dy, bool s) { calls << QStringLiteral("cursor %1 %2 %3").arg(dx).arg(dy).arg(s); };
        b.autoCaps = [this] { return autoCaps; };
        b.canAdd = [this](const QString &) { return unknown; };
        b.addWord = [this](const QString &w) { calls << QStringLiteral("add ") + w; };
        b.useLanguage = [this](const QString &id) { calls << QStringLiteral("language ") + id; };
        b.luna = [this](const QString &uri, const QString &payload) { calls << QStringLiteral("luna %1 %2").arg(uri, payload); };
        b.savePrefs = [this](const QJsonObject &pre) { calls << QStringLiteral("save"); saved = pre; };
        b.removeWord = [this](const QString &w) { calls << QStringLiteral("remove ") + w; };
        b.hide = [this] { calls << QStringLiteral("hide"); };
        return b;
    }
};

/*!
 * A text field with a cursor, as the bridge sees it: keys type into it (a
 * backspace deletes, Return is "\n"), and words and replacements go in.
 * [onKey] stands in for the editor reacting to a key (autocorrecting).
 */
struct Field
{
    QString text;
    int cursor = 0;
    std::function<void(const QString &)> onKey;

    void set(const QString &t, int at = -1) { text = t; cursor = at < 0 ? int(t.size()) : at; }
    void insert(const QString &t) { text.insert(cursor, t); cursor += int(t.size()); }

    Pre::Bridge bridge(Recorder &rec)
    {
        Pre::Bridge b = rec.bridge();
        const auto press = b.press;
        b.release = [this, press](const QString &l, const QString &a) {
            if (a == QLatin1String("backspace")) {
                if (cursor > 0) { text.remove(cursor - 1, 1); --cursor; }
            } else if (a == QLatin1String("return")) {
                insert(QStringLiteral("\n"));
            } else {
                insert(l);
            }
            if (onKey) onKey(l);
        };
        b.before = [this](int n) { return text.left(cursor).right(n); };
        b.after = [this](int n) { return text.mid(cursor, n); };
        b.position = [this] { return cursor; };
        b.prose = [] { return true; };
        b.replace = [this, &rec](int before, int after, const QString &t) {
            rec.calls << QStringLiteral("replace %1 %2 %3").arg(before).arg(after).arg(t);
            text.replace(cursor - before, before + after, t);
            cursor += int(t.size()) - before;
        };
        return b;
    }
};

//! The language codes of the plugins' phone layouts: Keyboard_<code>_phone.qml.
QStringList pluginLanguages()
{
    QStringList ids;
    QDir plugins(QStringLiteral(PLUGINS_DIR));
    for (const QString &dir : plugins.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir qml(plugins.filePath(dir + QStringLiteral("/qml")));
        for (const QString &f : qml.entryList({ QStringLiteral("Keyboard_*_phone.qml") }, QDir::Files))
            ids << f.mid(int(strlen("Keyboard_")), f.length() - int(strlen("Keyboard_")) - int(strlen("_phone.qml")));
    }
    ids.sort();
    return ids;
}

QList<const Pre::Key *> charKeys(const Pre::Rows &rows, int row)
{
    QList<const Pre::Key *> out;
    for (const Pre::Slot &s : rows[row])
        if (s.drawn && s.key->kind == Pre::Kind::Char) out << s.key.data();
    return out;
}

} // namespace

class TestPreKeyboard : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // --- Languages and keys ------------------------------------------------------------------

    // Every language plugin has a Pre layout: one added without running
    // tools/gen_pre_keymaps.py again would fall back to English.
    void everyPluginHasALanguage()
    {
        const QStringList plugins = pluginLanguages();
        QVERIFY(plugins.size() >= 19);
        QStringList ids;
        for (const Pre::Language &l : Pre::Language::all()) ids << l.id;
        ids.sort();
        QCOMPARE(ids, plugins);
    }

    void everyLanguageHasFourRowsOfKeys_data()
    {
        QTest::addColumn<QString>("id");
        for (const Pre::Language &l : Pre::Language::all())
            QTest::newRow(qPrintable(l.id)) << l.id;
    }

    void everyLanguageHasFourRowsOfKeys()
    {
        QFETCH(QString, id);
        const Pre::Language lang = Pre::Language::byId(id);
        QVERIFY(lang.isValid());
        QVERIFY(!lang.name.isEmpty());
        const Pre::Rows &rows = Pre::Keymap::letters(lang);
        QCOMPARE(rows.size(), 4);
        // Letters on the first three rows, backspace ending the second, the
        // orange key opening the third and Return ending it.
        for (int r = 0; r < 3; ++r) {
            QVERIFY2(charKeys(rows, r).size() >= 6, qPrintable(QString::number(r)));
            for (const Pre::Key *k : charKeys(rows, r)) QVERIFY(!k->text.isEmpty());
        }
        QCOMPARE(rows[1].last().key->kind, Pre::Kind::Backspace);
        QCOMPARE(rows[2].first().key->kind, Pre::Kind::Orange);
        QCOMPARE(rows[2].last().key->kind, Pre::Kind::Return);
        if (lang.layout == Pre::Layout::Own) {
            // The phone layout's letters, all of them, in its order.
            for (int r = 0; r < 3; ++r) {
                QStringList want, got;
                for (const Pre::LetterKey &k : lang.letters[r]) want << k.text;
                for (const Pre::Key *k : charKeys(rows, r)) got << k->text;
                if (r == 2) got.removeLast(); // the comma
                QCOMPARE(got, want);
            }
        }
    }

    void preGridKeepsThePresAlternates()
    {
        const Pre::Language enLang = Pre::Language::byId(QStringLiteral("en"));
        const Pre::Rows &en = Pre::Keymap::letters(enLang);
        QCOMPARE(charKeys(en, 0).first()->text, QStringLiteral("q"));
        QCOMPARE(charKeys(en, 0).first()->alt, QStringLiteral("/"));
        const Pre::Language deLang = Pre::Language::byId(QStringLiteral("de"));
        const Pre::Rows &de = Pre::Keymap::letters(deLang);
        QCOMPARE(charKeys(de, 0)[5]->text, QStringLiteral("z"));
        const Pre::Language frLang = Pre::Language::byId(QStringLiteral("fr"));
        const Pre::Rows &fr = Pre::Keymap::letters(frLang);
        QCOMPARE(charKeys(fr, 0).first()->text, QStringLiteral("a"));
        // German's own accents first in the holds.
        QCOMPARE(charKeys(de, 1).first()->extended.value(1), QStringLiteral("ä"));
    }

    // Shift is not always a capital: Arabic shifts to other letters, Hebrew to
    // Latin ones. Those come from the layout; a capital is left to the view.
    void shiftComesFromTheLayout()
    {
        const Pre::Language arLang = Pre::Language::byId(QStringLiteral("ar"));
        const Pre::Rows &ar = Pre::Keymap::letters(arLang);
        bool found = false;
        for (const Pre::Key *k : charKeys(ar, 0))
            if (k->text == QStringLiteral("غ")) { QCOMPARE(k->shifted, QStringLiteral("إ")); found = true; }
        QVERIFY(found);
        const Pre::Language heLang = Pre::Language::byId(QStringLiteral("he"));
        const Pre::Rows &he = Pre::Keymap::letters(heLang);
        QCOMPARE(charKeys(he, 0)[2]->shifted, QStringLiteral("E"));
        const Pre::Language daLang = Pre::Language::byId(QStringLiteral("da"));
        const Pre::Rows &da = Pre::Keymap::letters(daLang);
        for (const Pre::Key *k : charKeys(da, 0)) QVERIFY2(k->shifted.isEmpty(), qPrintable(k->text));
    }

    // Cyrillic shifts to Cyrillic: Russian's у once shifted to a Latin Y.
    void cyrillicShiftsToCyrillic_data()
    {
        QTest::addColumn<QString>("id");
        QTest::newRow("ru") << QStringLiteral("ru");
        QTest::newRow("uk") << QStringLiteral("uk");
    }

    void cyrillicShiftsToCyrillic()
    {
        QFETCH(QString, id);
        const Pre::Language lang = Pre::Language::byId(id);
        for (const QList<Pre::LetterKey> &row : lang.letters)
            for (const Pre::LetterKey &k : row) {
                const QString up = k.shifted.isEmpty() ? k.text.toUpper() : k.shifted;
                for (QChar c : up)
                    QVERIFY2(c.script() == QChar::Script_Cyrillic, qPrintable(k.text + QStringLiteral(" -> ") + up));
            }
    }

    void unknownLanguageIsEnglish()
    {
        QCOMPARE(Pre::Language::orEnglish(QStringLiteral("xx")).id, QStringLiteral("en"));
        QVERIFY(!Pre::Language::byId(QStringLiteral("xx")).isValid());
    }

    // --- Settings -----------------------------------------------------------------------------

    // The "pre" object holds the look only; the words are LuneOS's.
    void prefsHoldTheLookOnly()
    {
        Pre::Settings s;
        s.style = Pre::Style::White;
        s.height = Pre::Settings::Height::Large;
        s.holdForMore = true;
        s.vibrate = true;
        s.followShift = true;
        s.swipe = false;
        s.suggestions = true;
        const QJsonObject pre = s.toJson(QJsonObject{ { QStringLiteral("recentEmoji"), QJsonArray{ QStringLiteral("🙂") } } });
        const QStringList keys = pre.keys();
        QCOMPARE(QSet<QString>(keys.begin(), keys.end()),
                 (QSet<QString>{ QStringLiteral("style"), QStringLiteral("height"), QStringLiteral("holdForMore"),
                                 QStringLiteral("vibrate"), QStringLiteral("vibrateLevel"),
                                 QStringLiteral("followShift"), QStringLiteral("doubleSpacePeriod"),
                                 QStringLiteral("emojiSuggestions"),
                                 QStringLiteral("swipe"), QStringLiteral("recentEmoji") }));
        const Pre::Settings back = Pre::Settings::fromJson(pre);
        QCOMPARE(back.style, Pre::Style::White);
        QCOMPARE(back.height, Pre::Settings::Height::Large);
        QVERIFY(back.holdForMore && back.vibrate && back.followShift && !back.swipe);
        // Swiping is on till the owner turns it off.
        QVERIFY(Pre::Settings::fromJson(QJsonObject()).swipe);
        QVERIFY(!back.suggestions);
    }

    void recentEmojiAreCapped()
    {
        QJsonObject pre;
        for (int i = 0; i < Pre::Settings::RECENT_MAX + 5; ++i)
            pre = Pre::Settings::withRecentEmoji(pre, QString::number(i));
        const QStringList r = Pre::Settings::recentEmoji(pre);
        QCOMPARE(r.size(), Pre::Settings::RECENT_MAX);
        QCOMPARE(r.first(), QString::number(Pre::Settings::RECENT_MAX + 4));
    }

    // --- The controller -----------------------------------------------------------------------

    void keysGoToTheEditorAsLuneOSKeysDo()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.onText(QStringLiteral("a"));
        c.onText(QStringLiteral(" "));
        c.onEnter();
        c.onText(QStringLiteral("🙂"));
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("press a|"), QStringLiteral("release a|"),
                                          QStringLiteral("press  |space"), QStringLiteral("release  |space"),
                                          QStringLiteral("press |return"), QStringLiteral("release |return"),
                                          QStringLiteral("commit 🙂") }));
    }

    void backspaceIsPressedAndReleased()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.onBackspaceHeld(true);
        c.onBackspaceHeld(false, true);
        c.onBackspaceHeld(true);
        c.onBackspaceHeld(false, false);
        // Up without having gone down: nothing.
        c.onBackspaceHeld(false, true);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("press |backspace"), QStringLiteral("release |backspace"),
                                          QStringLiteral("press |backspace"), QStringLiteral("cancel") }));
    }

    void barPutsWhatSpaceTypesInTheMiddle()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        // A correction: the word as typed on the left, to keep it.
        c.setCandidates(QStringLiteral("teh"), QStringLiteral("the"),
                        { QStringLiteral("the"), QStringLiteral("then"), QStringLiteral("ten") });
        c.onPick(0, Pre::KeyboardView::CAPS_NONE);
        c.onPick(1, Pre::KeyboardView::CAPS_FIRST);
        c.onPick(2, Pre::KeyboardView::CAPS_ALL);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("pick teh"), QStringLiteral("pick The"),
                                          QStringLiteral("pick THEN") }));
        // No correction: the likeliest in the middle, the next ones either side.
        rec.calls.clear();
        c.setCandidates(QStringLiteral("the"), QString(),
                        { QStringLiteral("the"), QStringLiteral("they"), QStringLiteral("then") });
        for (int i = 0; i < 3; ++i) c.onPick(i, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("pick they"), QStringLiteral("pick the"),
                                          QStringLiteral("pick then") }));
        // No correction for a word the engine does not know (autocorrect off):
        // space keeps it as typed, so that is the middle, not the likeliest word.
        rec.calls.clear();
        c.setCandidates(QStringLiteral("quic"), QString(),
                        { QStringLiteral("quickly"), QStringLiteral("quickened") });
        for (int i = 0; i < 3; ++i) c.onPick(i, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("pick quickly"), QStringLiteral("pick quic"),
                                          QStringLiteral("pick quickened") }));
        // No word typed: the likeliest next word in the middle.
        rec.calls.clear();
        c.setCandidates(QString(), QString(),
                        { QStringLiteral("fox"), QStringLiteral("brown"), QStringLiteral("dog") });
        for (int i = 0; i < 3; ++i) c.onPick(i, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("pick brown"), QStringLiteral("pick fox"),
                                          QStringLiteral("pick dog") }));
    }

    void barIsEmptyWithoutSuggestionsOrWords()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.setKeyboardState(false, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.setCandidates(QStringLiteral("teh"), QStringLiteral("the"), { QStringLiteral("the") });
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.setWordsAllowed(false);
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        QVERIFY(rec.calls.isEmpty());
    }

    void languagesAreTheKeyboards()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        // A code without a plugin is skipped; the active one is always there.
        c.setKeyboardState(true, false, { QStringLiteral("en"), QStringLiteral("xx"), QStringLiteral("ru") },
                           QStringLiteral("ru"));
        c.onLanguageFlick(1);
        c.onLanguageFlick(-1);
        c.onUseLanguage(QStringLiteral("xx"));
        c.onUseLanguage(QStringLiteral("de"));
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("language en"), QStringLiteral("language en"),
                                          QStringLiteral("language de") }));
        rec.calls.clear();
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("de"));
        c.onLanguageFlick(1);
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("language en") });
    }

    /*
     * Space held still and let go: the languages, over the space bar. Held
     * and slid: the cursor moves, and no menu. With one language, no menu.
     */
    void holdingSpaceStillOffersTheLanguages()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        Pre::KeyboardView view;
        view.setControllerObject(&c);
        const Pre::Language en = Pre::Language::byId(QStringLiteral("en"));
        const Pre::Language de = Pre::Language::byId(QStringLiteral("de"));
        showKeys(view, en, 1080, 1080, false, Pre::Settings::Height::Medium);
        Pre::Settings s = view.settings();
        s.languages = { en, de };
        view.applySettings(s);
        int space = -1;
        for (int i = 0; i < view.m_zones.size(); ++i)
            if (view.m_zones[i].slot.key->kind == Pre::Kind::Space) { space = i; break; }
        QVERIFY(space >= 0);
        const QPointF at = view.m_zones[space].rect.center();

        // Held still, let go: the menu, above the space bar, and no space typed.
        QVERIFY(view.down(at.x(), at.y()));
        view.onHold();
        view.lift(true);
        QCOMPARE(view.m_menu, (QStringList{ en.name, de.name }));
        QVERIFY(view.m_menuFrame.bottom() <= view.m_zones[space].rect.top() + 1);
        QVERIFY(!rec.calls.join(QLatin1Char(',')).contains(QStringLiteral("|space")));
        view.reset();

        // Held and slid: the cursor, and no menu.
        rec.calls.clear();
        QVERIFY(view.down(at.x(), at.y()));
        view.onHold();
        view.move(at.x() + 40 * view.m_dp, at.y());
        view.lift(true);
        QVERIFY(view.m_menu.isEmpty());
        QVERIFY(rec.calls.contains(QStringLiteral("cursor 1 0 0")));
        view.reset();

        // One language: nothing to choose.
        s.languages = { en };
        view.applySettings(s);
        QVERIFY(view.down(at.x(), at.y()));
        view.onHold();
        view.lift(true);
        QVERIFY(view.m_menu.isEmpty());
    }

    // Vibration only when it is switched on, a long press included.
    void vibratesOnlyWhenAsked()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.hapticKey();
        c.hapticLongPress();
        QVERIFY(rec.calls.isEmpty());
        c.setPrefs(QJsonObject{ { QStringLiteral("vibrate"), true } });
        c.hapticLongPress();
        QCOMPARE(rec.calls.size(), 1);
        QVERIFY(rec.calls.first().contains(QStringLiteral("\"duration\":20")));
    }

    /*
     * Light, Firmer and Strong: each a longer buzz than the one before (the
     * vibrator takes no strength), Light the 12 ms it always was, and a long
     * press a little more than a key. Anything else read is Light or Strong.
     */
    void vibrationHasThreeStrengths()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        QList<int> key, hold;
        for (int level = 1; level <= Pre::Settings::VIBRATE_LEVELS; ++level) {
            c.setPrefs(QJsonObject{ { QStringLiteral("vibrate"), true }, { QStringLiteral("vibrateLevel"), level } });
            rec.calls.clear();
            c.hapticKey();
            c.hapticLongPress();
            QCOMPARE(rec.calls.size(), 2);
            const QRegularExpression ms(QStringLiteral("\"duration\":(\\d+)"));
            key << ms.match(rec.calls[0]).captured(1).toInt();
            hold << ms.match(rec.calls[1]).captured(1).toInt();
        }
        QCOMPARE(key.first(), 12);
        for (int i = 1; i < key.size(); ++i) QVERIFY2(key[i] > key[i - 1], "a level no firmer than the one before");
        for (int i = 0; i < key.size(); ++i) QVERIFY(hold[i] > key[i]);
        QCOMPARE(Pre::Settings::fromJson(QJsonObject{ { QStringLiteral("vibrateLevel"), 0 } }).vibrateLevel, 1);
        QCOMPARE(Pre::Settings::fromJson(QJsonObject{ { QStringLiteral("vibrateLevel"), 9 } }).vibrateLevel, 3);
        QCOMPARE(Pre::Settings::fromJson(QJsonObject()).vibrateLevel, 1);
    }

    /*
     * Hold a word the dictionaries know, and it can be deleted: kept per
     * language under "removed" in the "pre" object, taken out of the owner's
     * dictionary, and off the bar at once. Adding it again puts it back.
     */
    void wordsCanBeDeleted()
    {
        Recorder rec;
        rec.unknown = false;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.setKeyboardState(true, false, { QStringLiteral("en"), QStringLiteral("de") }, QStringLiteral("en"));
        QVERIFY(c.canRemove(QStringLiteral("yest")));
        QVERIFY(!c.canRemove(QString()));
        c.setCandidates(QStringLiteral("yes"), QStringLiteral("yest"),
                        { QStringLiteral("yest"), QStringLiteral("yes"), QStringLiteral("yesterday") });
        QVERIFY(c.m_bar.contains(QStringLiteral("yest")));
        rec.calls.clear();
        c.onRemoveWord(QStringLiteral("yest"));
        QVERIFY(rec.calls.contains(QStringLiteral("remove yest")));
        QCOMPARE(Pre::Settings::removed(rec.saved, QStringLiteral("en")), QStringList{ QStringLiteral("yest") });
        QVERIFY(Pre::Settings::removed(rec.saved, QStringLiteral("de")).isEmpty());
        QVERIFY(!c.m_bar.contains(QStringLiteral("yest")));
        // Deleted twice is still one.
        c.setPrefs(rec.saved);
        c.onRemoveWord(QStringLiteral("yest"));
        QCOMPARE(Pre::Settings::removed(rec.saved, QStringLiteral("en")).size(), 1);
        // Wanted after all: added, and off the list.
        c.setPrefs(rec.saved);
        rec.unknown = true;
        c.onAddWord(QStringLiteral("yest"));
        QVERIFY(Pre::Settings::removed(rec.saved, QStringLiteral("en")).isEmpty());
        QVERIFY(!rec.saved.value(QStringLiteral("removed")).toObject().contains(QStringLiteral("en")));
    }

    // The word engine neither offers a deleted word nor puts it in for space, whatever its case.
    void deletedWordsAreNotOffered()
    {
        SetWords words;
        words.setEnabled(true);
        words.words = { QStringLiteral("yest"), QStringLiteral("yes"), QStringLiteral("yesterday") };
        Model::Text text;
        text.setPreedit(QStringLiteral("yes"));
        WordCandidateList got;
        QObject::connect(&words, &Logic::AbstractWordEngine::candidatesChanged,
                         [&](const WordCandidateList &c) { got = c; });
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("yest"));
        words.setBlockedWords({ QStringLiteral("Yest") });
        QVERIFY(words.isBlocked(QStringLiteral("YEST")));
        QVERIFY(!words.isBlocked(QStringLiteral("yes")));
        words.computeCandidates(&text);
        QStringList offered;
        for (const WordCandidate &c : got) offered << c.word();
        QCOMPARE(offered, (QStringList{ QStringLiteral("yes"), QStringLiteral("yesterday") }));
        QCOMPARE(text.primaryCandidate(), QStringLiteral("yes"));
        words.setBlockedWords({});
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("yest"));
    }

    /*
     * A word the owner saved keeps its capitals: "webos" typed is "webOS",
     * first in the bar and what space puts in, in place of the dictionary's
     * own guesses at it. Typed as saved, or a different word, nothing changes;
     * a saved word that was deleted isn't put in either.
     */
    void savedWordsKeepTheirCapitals()
    {
        SetWords words;
        words.setEnabled(true);
        words.words = { QStringLiteral("webs"), QStringLiteral("WebOS"), QStringLiteral("web") };
        words.own = { QStringLiteral("LuneOS"), QStringLiteral("webOS") };
        Model::Text text;
        WordCandidateList got;
        QObject::connect(&words, &Logic::AbstractWordEngine::candidatesChanged,
                         [&](const WordCandidateList &c) { got = c; });
        auto offered = [&] {
            QStringList out;
            for (const WordCandidate &c : got) out << c.word();
            return out;
        };

        text.setPreedit(QStringLiteral("webos"));
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("webOS"));
        QCOMPARE(offered(), (QStringList{ QStringLiteral("webOS"), QStringLiteral("webs"), QStringLiteral("web") }));

        text.setPreedit(QStringLiteral("WEBOS"));
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("webOS"));

        // As saved: the engine's own answer stands.
        text.setPreedit(QStringLiteral("webOS"));
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("webs"));

        // Only the whole word: "webo" is on its way to something else.
        text.setPreedit(QStringLiteral("webo"));
        words.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("webs"));

        words.setBlockedWords({ QStringLiteral("webos") });
        text.setPreedit(QStringLiteral("webos"));
        words.computeCandidates(&text);
        QVERIFY(text.primaryCandidate() != QStringLiteral("webOS"));
    }

    // The editor: a period typed after a word that would be corrected keeps it, and the correction waits.
    void periodAfterACorrectableWordIsKept()
    {
        SetWords *words = new SetWords;
        words->setEnabled(true);
        words->words = { QStringLiteral("the") };
        StringEditor editor(words);
        editor.setPreeditEnabled(true);
        editor.setAutoCorrectEnabled(true);
        Logic::EventHandler handler;
        QObject::connect(&handler, &Logic::EventHandler::keyPressed, &editor, &AbstractTextEditor::onKeyPressed);
        QObject::connect(&handler, &Logic::EventHandler::keyReleased, &editor, &AbstractTextEditor::onKeyReleased);
        auto key = [&](const QString &label, const QString &action = QString()) {
            handler.onKeyPressed(label, action);
            handler.onKeyReleased(label, action);
        };
        auto type = [&](const QString &t) {
            for (const QChar c : t) {
                if (c == QLatin1Char(' ')) key(QStringLiteral(" "), QStringLiteral("space"));
                else key(QString(c));
            }
        };
        // Spelt right: the period goes on the word as typed, and space adds nothing more.
        type(QStringLiteral("teh. "));
        QCOMPARE(editor.committed, QStringLiteral("teh. "));
        // Spelt wrong: corrected, as space would, and the period kept.
        words->misspelled = { QStringLiteral("teh") };
        editor.committed.clear();
        type(QStringLiteral("teh, "));
        QCOMPARE(editor.committed, QStringLiteral("the, "));
        // Inside a word an apostrophe is the word's.
        words->words = { QStringLiteral("don't") };
        words->misspelled.clear();
        editor.committed.clear();
        type(QStringLiteral("don't"));
        QCOMPARE(editor.preedit, QStringLiteral("don't"));
        QVERIFY(editor.committed.isEmpty());
        // Autocorrect off: as typed.
        editor.setAutoCorrectEnabled(false);
        words->words = { QStringLiteral("the") };
        words->misspelled = { QStringLiteral("teh") };
        editor.committed.clear();
        type(QStringLiteral(" teh?"));
        QCOMPARE(editor.committed, QStringLiteral("don't teh?"));
    }

    /*
     * The Pre's punctuation, through the event handler and the editor into a
     * string standing for the application: each case is what was typed, and
     * what the field holds after. "_" is a pause long enough that two spaces
     * are not a double space.
     */
    void punctuation_data()
    {
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QString>("field");
        QTest::addColumn<bool>("prose");
        QTest::addColumn<bool>("doubleSpace");
        QTest::newRow("double space") << "hi  " << "hi. " << true << true;
        QTest::newRow("double space, slow") << "hi _ " << "hi  " << true << true;
        QTest::newRow("double space off") << "hi  " << "hi  " << true << false;
        QTest::newRow("double space after nothing") << "  " << "  " << true << true;
        QTest::newRow("period brings a space") << "hi.yo" << "hi. yo" << true << true;
        QTest::newRow("space on the period's") << "hi. yo" << "hi. yo" << true << true;
        QTest::newRow("no space in a number") << "3.14" << "3.14" << true << true;
        QTest::newRow("no space in an address") << "a@b.com" << "a@b.com" << true << true;
        QTest::newRow("no space in www") << "www.x" << "www.x" << true << true;
        QTest::newRow("no space outside prose") << "hi.yo" << "hi.yo" << false << true;
        QTest::newRow("comma after a space") << "hi ,yo" << "hi, yo" << true << true;
        QTest::newRow("question after a space") << "hi ?" << "hi? " << true << true;
        QTest::newRow("ellipsis") << "hi..." << "hi... " << true << true;
        // Against the period, as on Android: the owner typed no space to move after it.
        QTest::newRow("question on the period") << "hi.?" << "hi.?" << true << true;
        QTest::newRow("a number's period stays") << "3 .5" << "3 .5" << true << true;
        QTest::newRow("double space, then a comma") << "hi  ," << "hi.," << true << true;
        QTest::newRow("backspace takes the period's space") << "hi.<x" << "hi.x" << true << true;
        QTest::newRow("return drops the period's space") << "hi.>" << "hi.\r" << true << true;
    }

    void punctuation()
    {
        QFETCH(QString, typed);
        QFETCH(QString, field);
        QFETCH(bool, prose);
        QFETCH(bool, doubleSpace);
        SetWords *words = new SetWords;
        StringEditor editor(words);
        Logic::EventHandler handler;
        QObject::connect(&handler, &Logic::EventHandler::keyPressed, &editor, &AbstractTextEditor::onKeyPressed);
        QObject::connect(&handler, &Logic::EventHandler::keyReleased, &editor, &AbstractTextEditor::onKeyReleased);
        Pre::Bridge b;
        b.press = [&](const QString &l, const QString &a) { handler.onKeyPressed(l, a); };
        b.release = [&](const QString &l, const QString &a) { handler.onKeyReleased(l, a); };
        b.before = [&](int n) { return (editor.committed + editor.preedit).right(n); };
        b.after = [](int) { return QString(); };
        b.prose = [&] { return prose; };
        qint64 clock = 0;
        Pre::Controller c;
        c.setBridge(b);
        c.setClock([&] { return clock; });
        c.setPrefs(QJsonObject{ { QStringLiteral("doubleSpacePeriod"), doubleSpace } });
        c.startInput();
        for (const QChar ch : typed) {
            clock += 150;
            if (ch == QLatin1Char('_')) clock += 2 * Pre::Controller::DOUBLE_SPACE_MS;
            else if (ch == QLatin1Char('<')) c.onBackspace(false);
            else if (ch == QLatin1Char('>')) c.onEnter();
            else c.onText(QString(ch));
        }
        // Whatever is still being typed, as the field shows it.
        QCOMPARE(editor.committed + editor.preedit, field);
    }

    /*
     * Before a word is started - after a space, or at the start - the bar
     * offers the likeliest next words, the likeliest in the middle; tapping
     * one puts it in. Not inside or right after a word, nor with suggestions
     * off, nor in a field that isn't for prose.
     */
    void nextWordsBeforeAWord()
    {
        Recorder rec;
        QString before = QStringLiteral("I ");
        bool prose = true;
        int asked = 0;
        Pre::Bridge b = rec.bridge();
        b.before = [&](int n) { return before.right(n); };
        b.prose = [&] { return prose; };
        b.nextWords = [&] { ++asked; return QStringList{ QStringLiteral("think"), QStringLiteral("don't"),
                                                         QStringLiteral("know") }; };
        Pre::Controller c;
        c.setBridge(b);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.setCandidates(QString(), QString(), {});
        QCOMPARE(c.m_bar, (QStringList{ QStringLiteral("don't"), QStringLiteral("think"), QStringLiteral("know") }));
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        QVERIFY(rec.calls.contains(QStringLiteral("pick think")));
        // Asked once for the same text.
        c.setCandidates(QString(), QString(), {});
        c.update();
        QCOMPARE(asked, 1);
        // The start of the field.
        before.clear();
        c.setCandidates(QString(), QString(), {});
        QCOMPARE(c.m_bar.value(1), QStringLiteral("think"));
        // Right after a word: that word is the one being typed.
        before = QStringLiteral("I");
        c.setCandidates(QString(), QString(), {});
        QCOMPARE(c.m_bar, (QStringList{ QString(), QString(), QString() }));
        // A word being typed has its own candidates.
        before = QStringLiteral("I th");
        c.setCandidates(QStringLiteral("th"), QString(), { QStringLiteral("the") });
        QCOMPARE(c.m_bar.value(1), QStringLiteral("th"));
        // Not for an address, nor with suggestions off.
        before = QStringLiteral("I ");
        prose = false;
        c.setCandidates(QString(), QString(), {});
        QVERIFY(c.m_bar.value(1).isEmpty());
        prose = true;
        // Swipe keeps the bar up with suggestions off: still no next words.
        QTemporaryDir dir;
        writePresage(dir.filePath(QStringLiteral("lib/en/database_en.db")), { { QStringLiteral("the"), 400 } });
        QSignalSpy loaded(&c, &Pre::Controller::swipeWordsLoaded);
        c.setWordDirs(dir.filePath(QStringLiteral("lib")), dir.filePath(QStringLiteral("dict")));
        QVERIFY(loaded.wait(5000));
        c.setKeyboardState(false, false, { QStringLiteral("en") }, QStringLiteral("en"));
        QVERIFY(c.shown().showsBar());
        c.setCandidates(QString(), QString(), {});
        QVERIFY(c.m_bar.value(1).isEmpty());
    }

    // The engine's next words: at most the number asked, none deleted, none while it is off.
    void nextWordsFromTheEngine()
    {
        SetWords words;
        words.next = { QStringLiteral("think"), QStringLiteral("yest"), QStringLiteral("think"),
                       QStringLiteral("know"), QStringLiteral("am") };
        QVERIFY(words.nextWords(QStringLiteral("I "), 3).isEmpty());
        words.setEnabled(true);
        words.setBlockedWords({ QStringLiteral("yest") });
        QCOMPARE(words.nextWords(QStringLiteral("I "), 3),
                 (QStringList{ QStringLiteral("think"), QStringLiteral("know"), QStringLiteral("am") }));
        QCOMPARE(words.nextWords(QStringLiteral("I "), 1), QStringList{ QStringLiteral("think") });
    }

    // A word's emoji: the first the emoji page has, for the word or what it is the plural or possessive of.
    void emojiForAWord()
    {
        const QString pizza = QStringLiteral("🍕"), box = QStringLiteral("📦"), ab = QStringLiteral("🆎");
        const QString a = QStringLiteral("🅰"), cat = QStringLiteral("🐈"), catFace = QStringLiteral("🐱");
        const Pre::EmojiWords words(
            QStringLiteral("# made up\npizza\t%1\nbox\t%2\nab\tX %3\nnone\tX\n\tY\na\t%4\ncat\t%5 %6\n")
                .arg(pizza, box, ab, a, cat, catFace),
            { pizza, box, ab, a, cat, catFace });
        QCOMPARE(words.size(), 5);
        QCOMPARE(words.forWord(QStringLiteral("cat")), cat);
        QCOMPARE(words.forWord(QStringLiteral("a")), a);
        QVERIFY(words.forWord(QStringLiteral("as")).isEmpty()); // too little left to be a plural
        QCOMPARE(words.forWord(QStringLiteral("Pizza")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("pizzas")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("pizza's")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("pizza\u2019s")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("pizzas'")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("pizzas\u2019")), pizza);
        QCOMPARE(words.forWord(QStringLiteral("boxes")), box);
        QCOMPARE(words.forWord(QStringLiteral("ab")), ab);
        QCOMPARE(words.forWord(QStringLiteral("abs")), ab);
        QVERIFY(words.forWord(QStringLiteral("none")).isEmpty());
        QVERIFY(words.forWord(QStringLiteral("piz")).isEmpty());
        QVERIFY(words.forWord(QStringLiteral("'")).isEmpty());

        // Each language's file, against the emoji page's emoji.
        QSet<QString> page;
        QFile f(QStringLiteral(PRE_DATA_DIR "/emoji.txt"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
            const QStringList parts = line.split(QLatin1Char('\t'));
            if (parts.size() == 3) page.insert(parts.at(2));
        }
        for (const char *id : { "en", "de", "es", "fr", "it" }) {
            const Pre::EmojiWords lang =
                Pre::EmojiWords::load(QStringLiteral(PRE_DATA_DIR "/%1/emoji_words.txt").arg(QLatin1String(id)), page);
            QVERIFY2(lang.size() > 2500, id);
        }
        const Pre::EmojiWords en = Pre::EmojiWords::load(QStringLiteral(PRE_DATA_DIR "/en/emoji_words.txt"), page);
        QCOMPARE(en.forWord(QStringLiteral("pizza")), pizza);
    }

    /*
     * Emoji suggestions on: the word being typed, or swiped, has its emoji in
     * the bar's right-hand place; tapping it puts the emoji in for the word,
     * as it is whatever shift says, and it is a recent emoji. Not for the next
     * words, nor with the setting off.
     */
    void emojiSuggestionsInTheBar()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        c.setDataDir(QStringLiteral(PRE_DATA_DIR));
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        const QString pizza = QStringLiteral("🍕");
        c.setCandidates(QStringLiteral("pizza"), QString(), { QStringLiteral("pizza"), QStringLiteral("pizzas") });
        QCOMPARE(c.m_bar, (QStringList{ QStringLiteral("pizzas"), QStringLiteral("pizza"), QString() }));
        c.setPrefs(QJsonObject{ { QStringLiteral("emojiSuggestions"), true } });
        c.setCandidates(QStringLiteral("pizza"), QString(), { QStringLiteral("pizza"), QStringLiteral("pizzas") });
        QCOMPARE(c.m_bar, (QStringList{ QStringLiteral("pizzas"), QStringLiteral("pizza"), pizza }));
        c.onPick(2, Pre::KeyboardView::CAPS_ALL);
        QCOMPARE(rec.calls.value(rec.calls.size() - 2), QStringLiteral("pick ") + pizza);
        QCOMPARE(Pre::Settings::recentEmoji(rec.saved).value(0), pizza);
        // The correction's emoji, the one space would put in.
        c.setCandidates(QStringLiteral("piza"), QStringLiteral("pizza"), { QStringLiteral("pizza") });
        QCOMPARE(c.m_bar, (QStringList{ QStringLiteral("piza"), QStringLiteral("pizza"), pizza }));
        // Picking a word is as before.
        rec.calls.clear();
        c.onPick(1, Pre::KeyboardView::CAPS_FIRST);
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("pick Pizza") });
        // None for a word without one.
        c.setCandidates(QStringLiteral("the"), QString(), { QStringLiteral("the"), QStringLiteral("then") });
        QCOMPARE(c.m_bar.value(2), QString());
        // Nor for the words that might come next.
        Pre::Bridge b = rec.bridge();
        b.before = [](int) { return QStringLiteral("I like "); };
        b.nextWords = [] { return QStringList{ QStringLiteral("pizza"), QStringLiteral("it") }; };
        c.setBridge(b);
        c.setCandidates(QString(), QString(), {});
        QCOMPARE(c.m_bar.value(1), QStringLiteral("pizza"));
        QCOMPARE(c.m_bar.value(2), QString());
        // A language without them.
        c.setKeyboardState(true, false, { QStringLiteral("nl") }, QStringLiteral("nl"));
        c.setCandidates(QStringLiteral("pizza"), QString(), { QStringLiteral("pizza") });
        QCOMPARE(c.m_bar.value(2), QString());
        // The bar is up for them with suggestions off; not in a field without words.
        c.setKeyboardState(false, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.setCandidates(QStringLiteral("pizza"), QString(), {});
        QCOMPARE(c.m_bar, (QStringList{ QString(), QStringLiteral("pizza"), pizza }));
        c.setWordsAllowed(false);
        QCOMPARE(c.m_bar, (QStringList{ QString(), QString(), QString() }));
    }

    /*
     * A word tapped in the text: the bar offers what else it might be, the
     * likeliest in the middle, and tapping one puts it in its place, with the
     * capital the word had. The cursor inside a word offers them too; just
     * after a word the keyboard typed, it doesn't.
     */
    void fixATappedWord()
    {
        Recorder rec;
        Field field;
        qint64 clock = 10000;
        QStringList asked;
        Pre::Bridge b = field.bridge(rec);
        b.alternatives = [&](const QString &w, const QString &left) {
            asked << left + QLatin1Char('|') + w;
            return QStringList{ QStringLiteral("ball"), QStringLiteral("bell"), QStringLiteral("bill"),
                                QStringLiteral("bull"), QStringLiteral("belt") };
        };
        Pre::Controller c;
        c.setClock([&] { return clock; });
        c.setBridge(b);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        field.set(QStringLiteral("I saw a bell here"), 12);
        c.update();
        QCOMPARE(c.m_bar, (QStringList{ QStringLiteral("bill"), QStringLiteral("ball"), QStringLiteral("bull") }));
        QCOMPARE(asked, QStringList{ QStringLiteral("I saw a |bell") });
        QCOMPARE(c.m_candidates.size(), 3);
        c.update();
        QCOMPARE(asked.size(), 1); // asked once for the same word
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls.last(), QStringLiteral("replace 4 0 ball"));
        QCOMPARE(field.text, QStringLiteral("I saw a ball here"));
        // Still the word to fix: its others now.
        c.update();
        QCOMPARE(c.m_bar.value(1), QStringLiteral("bell"));

        // A capital stays; the cursor in the middle of the word.
        clock += 1000;
        field.set(QStringLiteral("Bell rang"), 2);
        c.update();
        QCOMPARE(c.m_bar.value(1), QStringLiteral("Ball"));
        c.onPick(0, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls.last(), QStringLiteral("replace 2 2 Bill"));
        QCOMPARE(field.text, QStringLiteral("Bill rang"));
        QCOMPARE(field.cursor, 4);

        // The word changed under it: nothing replaced.
        clock += 1000;
        field.set(QStringLiteral("a bell"), 6);
        c.update();
        QCOMPARE(c.m_bar.value(1), QStringLiteral("ball"));
        field.set(QStringLiteral("a belly"), 7);
        rec.calls.clear();
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        QVERIFY(rec.calls.isEmpty());

        // Just after a word the keyboard typed: not tapped, no fixes.
        clock += 1000;
        field.set(QStringLiteral("the bell"), 0);
        c.update();
        c.key(QStringLiteral("x"));
        field.set(QStringLiteral("the bell"));
        clock += 50;
        c.update();
        QCOMPARE(c.m_bar, (QStringList{ QString(), QString(), QString() }));
        // Inside one, though, yes.
        field.set(QStringLiteral("the bell"), 6);
        clock += 50;
        c.update();
        QCOMPARE(c.m_bar.value(1), QStringLiteral("ball"));

        // Not part of an address, a link, a tag or a number; not a word that runs on past what was asked.
        for (const QString &t : { QStringLiteral("me@bell"), QStringLiteral("bell@me"), QStringLiteral("a.bell"),
                                  QStringLiteral("#bell"), QStringLiteral("3rd"), QStringLiteral("bell3"),
                                  QString(70, QLatin1Char('b')) }) {
            clock += 1000;
            field.set(t, int(t.size()) - 1);
            c.update();
            QVERIFY2(c.m_bar.value(1).isEmpty(), qPrintable(t));
        }
        // Not with suggestions off, nor outside prose.
        clock += 1000;
        field.set(QStringLiteral("a bell"), 4);
        c.setKeyboardState(false, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.update();
        QVERIFY(c.m_candidates.isEmpty());
    }

    // Typing over the word tapped: the bar is the typed word's, and a pick is a pick.
    void typingOverATappedWord()
    {
        Recorder rec;
        Field field;
        Pre::Bridge b = field.bridge(rec);
        b.alternatives = [](const QString &, const QString &) { return QStringList{ QStringLiteral("ball") }; };
        Pre::Controller c;
        c.setClock([] { return qint64(10000); });
        c.setBridge(b);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        field.set(QStringLiteral("a bell"));
        c.update();
        QCOMPARE(c.m_bar.value(1), QStringLiteral("ball"));
        c.setCandidates(QStringLiteral("x"), QString(), { QStringLiteral("x"), QStringLiteral("xi") });
        rec.calls.clear();
        c.onPick(1, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("pick x") });
    }

    void wordAround()
    {
        int b = -1, a = -1;
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("say don"), QStringLiteral("'t go"), &b, &a),
                 QStringLiteral("don't"));
        QCOMPARE(b, 3);
        QCOMPARE(a, 2);
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("'quote"), QStringLiteral("' x"), &b, &a),
                 QStringLiteral("quote"));
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("a "), QStringLiteral(" b"), &b, &a), QString());
        QCOMPARE(b, 0);
        QCOMPARE(a, 0);
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("caf\u00e9"), QString(), &b, &a), QStringLiteral("caf\u00e9"));
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("x-ray"), QString(), &b, &a), QString());
        QCOMPARE(Pre::Controller::wordAround(QStringLiteral("bell"), QStringLiteral("/x"), &b, &a), QString());
    }

    /*
     * The very next backspace after an autocorrection puts back what was
     * typed, with what finished the word; after a period it first takes the
     * period's space. Anything typed in between, and it is a backspace again.
     * With vibration on, a correction buzzes.
     */
    void backspaceUndoesACorrection()
    {
        Recorder rec;
        Field field;
        Pre::Controller c;
        field.onKey = [&](const QString &k) {
            if ((k == QLatin1String(" ") || k == QLatin1String(".")) && field.text.endsWith(QStringLiteral("teh") + k)) {
                field.text.replace(field.text.size() - 4, 3, QStringLiteral("the"));
                c.corrected(QStringLiteral("teh"), QStringLiteral("the"));
            }
        };
        c.setBridge(field.bridge(rec));
        c.setPrefs(QJsonObject{ { QStringLiteral("vibrate"), true } });
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral(" "));
        QCOMPARE(field.text, QStringLiteral("I the "));
        QVERIFY(rec.calls.contains(QStringLiteral("luna palm://com.palm.vibrate/vibrate {\"period\":0,\"duration\":28}")));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I teh "));
        QCOMPARE(rec.calls.last(), QStringLiteral("replace 4 0 teh "));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I teh"));

        // A period, and its own space.
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral("."));
        QCOMPARE(field.text, QStringLiteral("I the. "));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I the."));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I teh."));

        // Something typed in between.
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral(" "));
        c.onText(QStringLiteral("x"));
        c.onBackspace(false);
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I the"));
        // Or Return.
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral(" "));
        c.onEnter();
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I the "));
        // The text no longer ending in it (the application changed it).
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral(" "));
        field.set(QStringLiteral("I then "));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I then"));
        // Part of a longer word: "atthe " was never this correction.
        field.set(QStringLiteral("I teh"));
        c.onText(QStringLiteral(" "));
        field.set(QStringLiteral("I atthe "));
        c.onBackspace(false);
        QCOMPARE(field.text, QStringLiteral("I atthe"));
    }

    /*
     * Something copied while the keys are up is offered in the bar straight
     * away; once pasted, not again, till something new is copied.
     */
    void pasteOfferedWhenCopied()
    {
        QClipboard *cb = QGuiApplication::clipboard();
        cb->clear();
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(QStringLiteral("en")), 1080, 1080, false, Pre::Settings::Height::Medium);
        c.setView(&view);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        c.startInput();
        QVERIFY(!view.pasteOffered());
        cb->setText(QStringLiteral("copied"));
        QTRY_VERIFY(view.pasteOffered());
        c.onPaste();
        QVERIFY(rec.calls.contains(QStringLiteral("commit copied")));
        QVERIFY(!view.pasteOffered());
        c.update();
        QVERIFY(!view.pasteOffered());
        cb->setText(QStringLiteral("again"));
        QTRY_VERIFY(view.pasteOffered());
        cb->clear();
    }

    void wordsAreAddedToLuneOSsDictionary()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        QVERIFY(c.canAdd(QStringLiteral("LuneOS")));
        QVERIFY(!c.canAdd(QString()));
        // Unknown: it can be added, not deleted.
        QVERIFY(!c.canRemove(QStringLiteral("LuneOS")));
        c.onAddWord(QStringLiteral("LuneOS"));
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("add LuneOS") });
        rec.unknown = false;
        QVERIFY(!c.canAdd(QStringLiteral("the")));
    }

    // --- Through the event handler to the editor ----------------------------------------------

    // EditorOptions: the first repeat after 350ms, then one every 120ms.
    void backspaceThroughTheEditor()
    {
        CountingEditor editor;
        Logic::EventHandler handler;
        QObject::connect(&handler, &Logic::EventHandler::keyPressed, &editor, &AbstractTextEditor::onKeyPressed);
        QObject::connect(&handler, &Logic::EventHandler::keyReleased, &editor, &AbstractTextEditor::onKeyReleased);
        Pre::Bridge b;
        b.press = [&](const QString &l, const QString &a) { handler.onKeyPressed(l, a); };
        b.release = [&](const QString &l, const QString &a) { handler.onKeyReleased(l, a); };
        b.cancelRepeat = [&] { editor.cancelRepeat(); };
        Pre::Controller c;
        c.setBridge(b);

        // A tap: one.
        c.onBackspaceHeld(true);
        c.onBackspaceHeld(false, true);
        QTest::qWait(600);
        QCOMPARE(editor.backspaces, 1);

        // Held: the editor repeats it, and stops when it comes up.
        c.onBackspaceHeld(true);
        QTest::qWait(700);
        QVERIFY2(editor.backspaces >= 3, qPrintable(QString::number(editor.backspaces)));
        c.onBackspaceHeld(false, true);
        const int up = editor.backspaces;
        QTest::qWait(600);
        QCOMPARE(editor.backspaces, up);

        // Taken away before it repeated: nothing.
        c.onBackspaceHeld(true);
        QTest::qWait(100);
        c.onBackspaceHeld(false, false);
        QTest::qWait(600);
        QCOMPARE(editor.backspaces, up);

        // Two taps through onBackspace (the emoji page's backspace): two.
        c.onBackspace(false);
        c.onBackspace(false);
        QCOMPARE(editor.backspaces, up + 2);
    }

    void barFollowsALanguageSwitch()
    {
        auto *words = new TwoWords;
        CountingEditor editor(words);
        words->setEnabled(true);
        editor.setPreeditEnabled(true);
        Logic::EventHandler handler;
        QObject::connect(&handler, &Logic::EventHandler::keyPressed, &editor, &AbstractTextEditor::onKeyPressed);
        QObject::connect(&handler, &Logic::EventHandler::keyReleased, &editor, &AbstractTextEditor::onKeyReleased);
        QStringList firsts;
        QObject::connect(&editor, &AbstractTextEditor::wordCandidatesChanged,
                         [&](const WordCandidateList &l) { firsts << (l.isEmpty() ? QString() : l.first().word()); });

        handler.onKeyPressed(QStringLiteral("p"), QString());
        handler.onKeyReleased(QStringLiteral("p"), QString());
        QCOMPARE(firsts.last(), QStringLiteral("pa"));

        // The word engine has the new language: the bar is asked again.
        words->first = QStringLiteral("po");
        editor.refreshCandidates();
        QCOMPARE(firsts.last(), QStringLiteral("po"));
        QCOMPARE(editor.text()->preedit(), QStringLiteral("p"));

        // Nothing typed: nothing stays up.
        handler.onKeyPressed(QString(), QStringLiteral("backspace"));
        handler.onKeyReleased(QString(), QStringLiteral("backspace"));
        editor.refreshCandidates();
        QCOMPARE(firsts.last(), QString());
    }

    void barEmptiesWhenTheWordIsBackspacedAway()
    {
        auto *words = new TwoWords;
        CountingEditor editor(words);
        words->setEnabled(true);
        editor.setPreeditEnabled(true);
        Logic::EventHandler handler;
        QObject::connect(&handler, &Logic::EventHandler::keyPressed, &editor, &AbstractTextEditor::onKeyPressed);
        QObject::connect(&handler, &Logic::EventHandler::keyReleased, &editor, &AbstractTextEditor::onKeyReleased);
        QList<int> sizes;
        QObject::connect(&editor, &AbstractTextEditor::wordCandidatesChanged,
                         [&](const WordCandidateList &l) { sizes << int(l.size()); });

        handler.onKeyPressed(QStringLiteral("p"), QString());
        handler.onKeyReleased(QStringLiteral("p"), QString());
        QCOMPARE(editor.text()->preedit(), QStringLiteral("p"));
        QVERIFY(!sizes.isEmpty());
        QCOMPARE(sizes.last(), 2);

        handler.onKeyPressed(QString(), QStringLiteral("backspace"));
        handler.onKeyReleased(QString(), QStringLiteral("backspace"));
        QCOMPARE(editor.text()->preedit(), QString());
        QCOMPARE(sizes.last(), 0);
    }

    // --- Swipe to type ----------------------------------------------------------------------------

    /*
     * A finger drawn through a word's keys, on a language's own keys, is that
     * word: Latin, Cyrillic, Hebrew and Arabic letters, and a letter with no
     * key of its own (ß, ê, a final ם, أ) on the key it is made from. The
     * other words are close calls on purpose: the same first and last letter,
     * as common.
     */
    void swipeSpellsWordsOnAnyKeys_data()
    {
        QTest::addColumn<QString>("language");
        QTest::addColumn<QStringList>("words");
        QTest::newRow("en") << "en" << QStringList{ "hello", "help", "hold", "keyboard", "people", "purple", "would", "world", "together", "the", "there" };
        QTest::newRow("de") << "de" << QStringList{ "straße", "strand", "stunde", "danke", "decke", "zeit", "zieht" };
        QTest::newRow("fr") << "fr" << QStringList{ "être", "entre", "merci", "maison", "matin" };
        QTest::newRow("ru") << "ru" << QStringList{ "привет", "пират", "спасибо", "свобода", "хорошо", "хотя" };
        QTest::newRow("uk") << "uk" << QStringList{ "дякую", "добре", "їжа", "людина" };
        QTest::newRow("he") << "he" << QStringList{ "שלום", "שמים", "תודה", "תורה", "ילד" };
        QTest::newRow("ar") << "ar" << QStringList{ "مرحبا", "مكتبة", "شكرا", "شجرة", "أنا", "كتاب" };
    }

    void swipeSpellsWordsOnAnyKeys()
    {
        QFETCH(QString, language);
        QFETCH(QStringList, words);
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(language), 1080, 1080, false, Pre::Settings::Height::Medium);
        view.show(Pre::KeyboardView::Page::Letters);
        Pre::SwipeDecoder::Layout layout;
        QVERIFY(view.swipeLayout(layout));
        QVector<QPair<QString, int>> entries;
        for (const QString &w : words) entries.append({ w, 12 });
        Pre::SwipeDecoder decoder(entries);
        for (const QString &w : words) {
            const QVector<QPointF> path = swipePath(layout, w, 0, 1);
            QVERIFY2(!path.isEmpty(), qPrintable(w + QStringLiteral(": a letter with no key")));
            QVector<float> xs, ys;
            for (const QPointF &p : path) { xs << float(p.x()); ys << float(p.y()); }
            const QStringList found = decoder.decode(xs, ys, layout, 4);
            QVERIFY2(!found.isEmpty() && found.first() == w,
                     qPrintable(QStringLiteral("%1: %2").arg(w, found.join(QLatin1Char(' ')))));
        }
    }

    //! Words given twice count once, at the higher score; a word added later is swiped, one taken out is not.
    void swipeWordsAreAddedAndTakenOut()
    {
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(QStringLiteral("en")), 1080, 1080, false, Pre::Settings::Height::Medium);
        Pre::SwipeDecoder::Layout layout;
        QVERIFY(view.swipeLayout(layout));
        auto decode = [&](Pre::SwipeDecoder &d, const QString &w) {
            QVector<float> xs, ys;
            for (const QPointF &p : swipePath(layout, w, 0, 1)) { xs << float(p.x()); ys << float(p.y()); }
            return d.decode(xs, ys, layout, 4);
        };
        Pre::SwipeDecoder d({ { QStringLiteral("hello"), 6 }, { QStringLiteral("hello"), 20 }, { QStringLiteral("hullo"), 12 },
                              { QStringLiteral("x"), 20 }, { QStringLiteral("rare"), 2 }, { QStringLiteral("a1b"), 20 } });
        QCOMPARE(d.size(), 2);
        QCOMPARE(decode(d, QStringLiteral("hello")).value(0), QStringLiteral("hello"));
        QVERIFY(decode(d, QStringLiteral("luneos")).isEmpty() || !decode(d, QStringLiteral("luneos")).contains(QStringLiteral("luneos")));
        d.add(QStringLiteral("luneos"), 14);
        QCOMPARE(decode(d, QStringLiteral("luneos")).value(0), QStringLiteral("luneos"));
        d.remove(QStringLiteral("luneos"));
        QVERIFY(!decode(d, QStringLiteral("luneos")).contains(QStringLiteral("luneos")));
        // Only a learned word goes with remove(); forget() takes any.
        d.remove(QStringLiteral("hello"));
        QVERIFY(decode(d, QStringLiteral("hello")).contains(QStringLiteral("hello")));
        d.forget(QStringLiteral("hello"));
        QVERIFY(!decode(d, QStringLiteral("hello")).contains(QStringLiteral("hello")));
    }

    void swipeWordsComeFromTheWordEngine()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writePresage(dir.filePath(QStringLiteral("lib/en/database_en.db")),
                     { { QStringLiteral("the"), 400 }, { QStringLiteral("hello"), 3 }, { QStringLiteral("—"), 50 } });
        writeFile(dir.filePath(QStringLiteral("dict/en_US.dic")),
                  "5\nhello/MS\nworld/S\tpo:noun\nParis/M\nNASA\nand\\/or/X\n");
        writeFile(dir.filePath(QStringLiteral("dict/en_US.aff")), "SET UTF-8\n");
        writeFile(dir.filePath(QStringLiteral("dict/nb.dic")), "0\n");

        const auto presage = Pre::SwipeWords::fromPresage(dir.filePath(QStringLiteral("lib/en/database_en.db")));
        QHash<QString, int> p(presage.begin(), presage.end());
        QCOMPARE(p.value(QStringLiteral("the")), 25);
        QVERIFY(p.value(QStringLiteral("hello")) > Pre::SwipeWords::HUNSPELL_SCORE);
        QVERIFY(p.value(QStringLiteral("hello")) < 25);

        QCOMPARE(Pre::SwipeWords::hunspellFile(QStringLiteral("en"), dir.filePath(QStringLiteral("dict"))),
                 dir.filePath(QStringLiteral("dict/en_US.dic")));
        // Norwegian's keyboard is "no", its dictionary Bokmål's.
        QCOMPARE(Pre::SwipeWords::hunspellFile(QStringLiteral("no"), dir.filePath(QStringLiteral("dict"))),
                 dir.filePath(QStringLiteral("dict/nb.dic")));
        QCOMPARE(Pre::SwipeWords::hunspellFile(QStringLiteral("zh"), dir.filePath(QStringLiteral("dict"))), QString());

        const auto hun = Pre::SwipeWords::fromHunspell(dir.filePath(QStringLiteral("dict/en_US.dic")));
        QHash<QString, int> h(hun.begin(), hun.end());
        QCOMPARE(h.value(QStringLiteral("hello")), Pre::SwipeWords::HUNSPELL_SCORE);
        QCOMPARE(h.value(QStringLiteral("world")), Pre::SwipeWords::HUNSPELL_SCORE);
        QCOMPARE(h.value(QStringLiteral("Paris")), Pre::SwipeWords::HUNSPELL_NAME_SCORE);
        QVERIFY(!h.contains(QStringLiteral("NASA")));
        QVERIFY(h.contains(QStringLiteral("and/or")));
        QCOMPARE(h.size(), 4);

        // Both: presage's score wins where both have a word.
        const auto all = Pre::SwipeWords::load(QStringLiteral("en"), dir.filePath(QStringLiteral("lib")),
                                               dir.filePath(QStringLiteral("dict")));
        Pre::SwipeDecoder d(all);
        QCOMPARE(d.size(), 4);   // the, hello, world, Paris (not "—", "and/or")
        QVERIFY(Pre::SwipeWords::load(QStringLiteral("zh"), dir.filePath(QStringLiteral("lib")),
                                      dir.filePath(QStringLiteral("dict"))).isEmpty());
        QVERIFY(Pre::SwipeWords::presageScore(1, 1000) >= 10);
        QCOMPARE(Pre::SwipeWords::presageScore(0, 1000), 0);
    }

    /*
     * Through the controller: swiping is off till the language's words are in;
     * a swiped word stands as the word being typed with the other guesses
     * either side of it in the bar; space or the next swipe puts it in as it
     * is (not autocorrected), a guess from the bar in its place, and a word
     * the cursor is right after gets a space before it. Shift goes with it.
     */
    void swipeThroughTheController()
    {
        QTemporaryDir dir;
        writePresage(dir.filePath(QStringLiteral("lib/en/database_en.db")),
                     { { QStringLiteral("hello"), 50 }, { QStringLiteral("hell"), 40 }, { QStringLiteral("hollo"), 2 },
                       { QStringLiteral("world"), 50 }, { QStringLiteral("would"), 60 }, { QStringLiteral("the"), 400 } });
        Recorder rec;
        Pre::Controller c;
        Pre::Bridge b = rec.bridge();
        bool atWordEnd = false;
        // As the editor does: what it is given to type comes back as the word being typed.
        b.setWord = [&](const QString &w) { rec.calls << QStringLiteral("set ") + w; c.setCandidates(w, QString(), { w }); };
        b.pick = [&](const QString &w) { rec.calls << QStringLiteral("pick ") + w; c.setCandidates(QString(), QString(), {}); };
        b.atWordEnd = [&] { return atWordEnd; };
        c.setBridge(b);
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(QStringLiteral("en")), 1080, 1080, false, Pre::Settings::Height::Medium);
        c.setView(&view);
        c.setKeyboardState(true, false, { QStringLiteral("en") }, QStringLiteral("en"));
        QVERIFY(!view.m_settings.swipe);
        QSignalSpy loaded(&c, &Pre::Controller::swipeWordsLoaded);
        c.setWordDirs(dir.filePath(QStringLiteral("lib")), dir.filePath(QStringLiteral("dict")));
        QVERIFY(loaded.wait(5000));
        QVERIFY(view.m_settings.swipe);

        Pre::SwipeDecoder::Layout layout;
        QVERIFY(view.swipeLayout(layout));
        auto swipe = [&](const QString &w, int caps) {
            QVector<float> xs, ys;
            for (const QPointF &p : swipePath(layout, w, 0, 1)) { xs << float(p.x()); ys << float(p.y()); }
            const int before = int(rec.calls.size());
            c.onSwipe(xs, ys, caps);
            // Worked out off the keyboard's thread: in once the word is set.
            QTRY_VERIFY(rec.calls.size() > before && rec.calls.last().startsWith(QStringLiteral("set ")));
        };
        swipe(QStringLiteral("hello"), Pre::KeyboardView::CAPS_FIRST);
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("set Hello") });
        QCOMPARE(view.m_words.value(1), QStringLiteral("Hello"));
        QVERIFY(view.m_words.contains(QStringLiteral("Hell")));
        // The next swipe: the first goes in as it was.
        rec.calls.clear();
        swipe(QStringLiteral("world"), Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("pick Hello"), QStringLiteral("set world") }));
        // Another guess from the bar takes its place.
        rec.calls.clear();
        const int other = view.m_words.indexOf(QStringLiteral("would"));
        QVERIFY(other == 0 || other == 2);
        c.onPick(other, Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, QStringList{ QStringLiteral("pick would") });
        // Space after a swiped word: it, as it is.
        rec.calls.clear();
        swipe(QStringLiteral("the"), Pre::KeyboardView::CAPS_ALL);
        c.onText(QStringLiteral(" "));
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("set THE"), QStringLiteral("pick THE") }));
        // Right after a word: a space first.
        rec.calls.clear();
        atWordEnd = true;
        swipe(QStringLiteral("hello"), Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("press  |space"), QStringLiteral("release  |space"),
                                          QStringLiteral("set hello") }));
        // A word tapped out before a swipe goes in as space would put it.
        rec.calls.clear();
        atWordEnd = false;
        c.setCandidates(QStringLiteral("teh"), QStringLiteral("the"), { QStringLiteral("the") });
        swipe(QStringLiteral("world"), Pre::KeyboardView::CAPS_NONE);
        QCOMPARE(rec.calls, (QStringList{ QStringLiteral("press  |space"), QStringLiteral("release  |space"),
                                          QStringLiteral("set world") }));
        // Swiping turned off: off in the view too.
        c.setPrefs(QJsonObject{ { QStringLiteral("swipe"), false } });
        QVERIFY(!view.m_settings.swipe);
    }

    /*
     * The language plugins' own words (PRE_SWIPE_LIB, PRE_SWIPE_DICT; the
     * device's paths by default): common words swiped a little off their keys,
     * as a finger does, come out first nearly always. Prints how often.
     */
    void swipeOnTheLanguagesWords_data()
    {
        QTest::addColumn<QString>("language");
        QTest::addColumn<QStringList>("words");
        QTest::newRow("en") << "en" << QStringList{ "the", "and", "you", "that", "have", "with", "this", "from", "they", "would",
                                                    "there", "their", "what", "about", "which", "when", "make", "like", "time", "just",
                                                    "know", "people", "into", "year", "good", "some", "could", "them", "other", "than",
                                                    "then", "look", "only", "come", "over", "think", "also", "back", "after", "work",
                                                    "first", "well", "even", "want", "because", "these", "give", "most", "phone", "hello" };
        QTest::newRow("de") << "de" << QStringList{ "und", "nicht", "eine", "habe", "auch", "noch", "wenn", "schon", "heute", "morgen",
                                                    "danke", "bitte", "gut", "zeit", "haus", "kommen", "gehen", "wieder", "immer", "machen" };
        QTest::newRow("nl") << "nl" << QStringList{ "niet", "het", "een", "maar", "voor", "zijn", "hebben", "goed", "morgen", "dank",
                                                    "graag", "huis", "tijd", "komen", "gaan", "weer", "altijd", "maken", "vandaag", "werk" };
        QTest::newRow("ru") << "ru" << QStringList{ "что", "это", "как", "она", "только", "когда", "очень", "спасибо", "хорошо", "сегодня",
                                                    "завтра", "время", "дом", "работа", "человек", "знаю", "могу", "хочу", "тебя", "меня" };
        QTest::newRow("he") << "he" << QStringList{ "אני", "הוא", "היא", "לא", "של", "את", "מה", "זה", "שלום", "תודה",
                                                    "בית", "היום", "מחר", "אוהב", "רוצה", "יודע", "עכשיו", "טוב", "אתה", "אנחנו" };
    }

    void swipeOnTheLanguagesWords()
    {
        QFETCH(QString, language);
        QFETCH(QStringList, words);
        const QString lib = qEnvironmentVariable("PRE_SWIPE_LIB", QStringLiteral("/usr/share/maliit/plugins/org/luneos/lib"));
        const QString dict = qEnvironmentVariable("PRE_SWIPE_DICT", QStringLiteral("/usr/share/hunspell"));
        QElapsedTimer t;
        t.start();
        const auto entries = Pre::SwipeWords::load(language, lib, dict);
        if (entries.isEmpty()) QSKIP("No words: set PRE_SWIPE_LIB and PRE_SWIPE_DICT");
        Pre::SwipeDecoder decoder(entries);
        const qint64 loadMs = t.elapsed();
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(language), 1080, 1080, false, Pre::Settings::Height::Medium);
        Pre::SwipeDecoder::Layout layout;
        QVERIFY(view.swipeLayout(layout));
        int first = 0, inBar = 0, tries = 0;
        qint64 worst = 0;
        QStringList missed;
        for (const QString &w : words) {
            for (int seed = 1; seed <= 3; ++seed) {
                QVector<float> xs, ys;
                for (const QPointF &p : swipePath(layout, w, 0.15, quint32(seed))) { xs << float(p.x()); ys << float(p.y()); }
                t.restart();
                const QStringList found = decoder.decode(xs, ys, layout, 3);
                worst = std::max(worst, t.elapsed());
                ++tries;
                if (found.value(0) == w) ++first;
                else missed << QStringLiteral("%1→%2").arg(w, found.value(0));
                if (found.contains(w)) ++inBar;
            }
        }
        qInfo("%s: %d words, loaded in %lld ms; first %d/%d, in the bar %d/%d; slowest swipe %lld ms; missed %s",
              qPrintable(language), decoder.size(), loadMs, first, tries, inBar, tries, worst,
              qPrintable(missed.join(QLatin1Char(' '))));
        QVERIFY2(first * 100 >= tries * 75, "fewer than three in four swiped words came out first");
        QVERIFY2(inBar * 100 >= tries * 90, "fewer than nine in ten swiped words were in the bar");
    }

    // --- Printing ---------------------------------------------------------------------------------

    /*
     * In every language, on the letters and the symbols, at every height, on
     * every screen of screens(): the keys tile the keyboard, a row's letters
     * stand on one line, every letter and alternate is inside its key's face
     * and clear of the other, and the letters are as large as that allows.
     */
    /*
     * The devices come from luneos-components (see profilesFile()). Without
     * them only the sweep runs, which knows no real phone: say so rather than
     * pass on less.
     */
    void devicesAreKnown()
    {
        if (profilesFile().isEmpty())
            QSKIP("luneos-components not found: set LUNEOS_COMPONENTS to test on its devices' screens");
        int devices = 0;
        for (const Screen &sc : screens()) devices += sc.measured;
        QVERIFY2(devices > 0, qPrintable(profilesFile() + QStringLiteral(" gave no phones")));
    }

    /*
     * Small, Medium and Large are three heights on every screen, upright and
     * on its side: in landscape all three were held to one share of the
     * screen, so the choice did nothing there.
     */
    void heightsDiffer()
    {
        QStringList same;
        const Pre::Language en = Pre::Language::byId(QStringLiteral("en"));
        for (const Screen &sc : screens()) {
            qreal last = 0;
            for (auto height : { Pre::Settings::Height::Small, Pre::Settings::Height::Medium,
                                 Pre::Settings::Height::Large }) {
                Pre::KeyboardView view;
                showKeys(view, en, sc.shortSide, sc.width, sc.landscape, height);
                if (view.keysHeight() <= last + 1)
                    same << QStringLiteral("%1: height %2 is %3 px, not above %4")
                                .arg(sc.name).arg(int(height)).arg(view.keysHeight()).arg(last);
                last = view.keysHeight();
            }
        }
        for (const QString &s : same.mid(0, 20)) qWarning().noquote() << s;
        QVERIFY2(same.isEmpty(), qPrintable(QStringLiteral("%1 screens").arg(same.size())));
    }

    void keysFitEveryScreen_data()
    {
        QTest::addColumn<int>("shortSide");
        QTest::addColumn<int>("width");
        QTest::addColumn<bool>("landscape");
        for (const Screen &sc : screens())
            QTest::newRow(qPrintable(sc.name)) << sc.shortSide << sc.width << sc.landscape;
    }

    void keysFitEveryScreen()
    {
        QFETCH(int, shortSide);
        QFETCH(int, width);
        QFETCH(bool, landscape);
        QStringList wrong;
        for (auto height : { Pre::Settings::Height::Small, Pre::Settings::Height::Medium,
                             Pre::Settings::Height::Large }) {
            for (const Pre::Language &l : Pre::Language::all()) {
                Pre::KeyboardView view;
                showKeys(view, l, shortSide, width, landscape, height);
                for (auto page : { Pre::KeyboardView::Page::Letters, Pre::KeyboardView::Page::Symbols }) {
                    view.show(page);
                    const QString where = QStringLiteral("%1 %2, height %3: ")
                                              .arg(l.id, page == Pre::KeyboardView::Page::Letters
                                                             ? QStringLiteral("letters") : QStringLiteral("symbols"))
                                              .arg(int(height));
                    wrong << layoutProblems(view, where) << printingProblems(view, where) << centringProblems(view, where);
                }
            }
        }
        for (const QString &w : wrong.mid(0, 30)) qWarning().noquote() << w;
        QVERIFY2(wrong.isEmpty(), qPrintable(QStringLiteral("%1 problems").arg(wrong.size())));
    }

    /*
     * What laying a page's letters out costs the first time it is shown, on
     * the screen it takes longest on: Russian, twelve keys a row, on a Sargo.
     * Run on a device (or with -callgrind) to see what a phone pays when the
     * language or the page changes; after the first time it is cached.
     */
    void firstLayoutCost()
    {
        Pre::KeyboardView view;
        showKeys(view, Pre::Language::byId(QStringLiteral("ru")), 1080, 1080, false,
                 Pre::Settings::Height::Medium);
        QBENCHMARK {
            view.m_fits.clear();
            view.fitLetters();
        }
    }

    /*
     * Letters that follow Shift: small with it off, capitals with it on, and
     * in the same place at the same size either way, on every phone and in
     * every language, with nothing out of its key in either.
     */
    void followShiftKeepsPlace()
    {
        QStringList problems;
        for (const Screen &sc : screens()) {
            if (!sc.measured) continue;
            for (const Pre::Language &l : Pre::Language::all()) {
                Pre::KeyboardView view;
                Pre::Settings s;
                s.language = l;
                s.languages = { l };
                s.followShift = true;
                view.setDp(sc.shortSide / 411.43);
                view.setLandscape(sc.landscape);
                view.applySettings(s);
                view.setWidth(sc.width);
                view.setHeight(view.headroom() + view.keysHeight());
                const QString where = sc.name + QLatin1Char(' ') + l.id + QStringLiteral(": ");
                QVector<QPair<qreal, qreal>> off;
                for (const auto &z : view.m_zones) off << qMakePair(z.letterPx, z.baseline);
                bool small = false;
                for (const auto &z : view.m_zones)
                    if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char)
                        small = small || view.capText(*z.slot.key) != view.capText(*z.slot.key).toUpper();
                if (l.id != QLatin1String("ar") && l.id != QLatin1String("he") && l.id != QLatin1String("zh") && !small)
                    problems << where + QStringLiteral("no small letters with shift off");
                problems << printingProblems(view, where + QStringLiteral("shift off: "));
                // Laid out afresh with shift on, it must come out the same.
                view.m_shift = Pre::KeyboardView::Shift::Once;
                view.m_fits.clear();
                view.layoutKeys();
                for (int k = 0; k < view.m_zones.size(); ++k) {
                    const auto &z = view.m_zones[k];
                    if (!z.slot.drawn || z.slot.key->kind != Pre::Kind::Char) continue;
                    if (qMakePair(z.letterPx, z.baseline) != off[k])
                        problems << where + view.capText(*z.slot.key) + QStringLiteral(": moves with shift");
                    if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char
                        && view.capText(*z.slot.key) != z.slot.key->text.toUpper())
                        problems << where + view.capText(*z.slot.key) + QStringLiteral(": not a capital with shift on");
                }
                problems << printingProblems(view, where + QStringLiteral("shift on: "));
            }
        }
        if (!problems.isEmpty()) qWarning().noquote() << problems.mid(0, 40).join(QLatin1Char('\n'));
        QCOMPARE(problems.size(), 0);
    }

    void numberFields_data()
    {
        QTest::addColumn<QString>("allowed");
        // InputMethod::digitsForContentType's.
        QTest::newRow("number") << QStringLiteral("0123456789+-.,");
        QTest::newRow("phone") << QStringLiteral("0123456789*#+");
    }

    /*
     * A number field, on every phone either way up: the keys show only what it
     * takes, each alone and big, and type just that; every digit is there, in
     * every language; the rest are blank and type nothing; the Sym page is
     * as it was.
     */
    void numberFields()
    {
        QFETCH(QString, allowed);
        QStringList problems;
        for (const Screen &sc : screens())
        for (const Pre::Language &l : Pre::Language::all()) {
            if (!sc.measured) continue;
            Pre::KeyboardView view;
            showKeys(view, l, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
            qreal letters = 0;
            for (const auto &z : view.m_zones)
                if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char) letters = z.letterPx;
            view.setDigits(allowed);
            const QString where = sc.name + QLatin1Char(' ') + l.id + QStringLiteral(": ");
            // A digit is no wider than the letters were: as big as them, at least.
            for (const auto &z : view.m_zones)
                if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char && z.letterPx < letters) {
                    problems << where + QStringLiteral("%1 px, smaller than the letters' %2").arg(z.letterPx).arg(letters);
                    break;
                }
            QSet<QChar> shown;
            int blank = 0;
            for (const auto &z : view.m_zones) {
                if (!z.slot.drawn || z.slot.key->kind != Pre::Kind::Char) continue;
                const QString s = view.capText(*z.slot.key);
                if (s != view.output(*z.slot.key)) problems << where + s + QStringLiteral(": types ") + view.output(*z.slot.key);
                if (!view.altText(*z.slot.key).isEmpty()) problems << where + s + QStringLiteral(": an alternate still printed");
                if (s.isEmpty()) ++blank;
                for (QChar c : s) {
                    if (!allowed.contains(c)) problems << where + s + QStringLiteral(": not for this field");
                    shown.insert(c);
                }
            }
            for (QChar d : QStringLiteral("0123456789"))
                if (!shown.contains(d)) problems << where + QStringLiteral("no ") + d;
            if (!blank) problems << where + QStringLiteral("no key went blank");
            problems << printingProblems(view, where) << centringProblems(view, where);
            view.show(Pre::KeyboardView::Page::Symbols);
            for (const auto &z : view.m_zones)
                if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char && view.capText(*z.slot.key) != z.slot.key->text)
                    problems << where + QStringLiteral("Sym page changed: ") + view.capText(*z.slot.key);
            view.setDigits(QString());
            view.show(Pre::KeyboardView::Page::Letters);
            for (const auto &z : view.m_zones)
                if (z.slot.drawn && z.slot.key->kind == Pre::Kind::Char && view.capText(*z.slot.key).isEmpty())
                    problems << where + QStringLiteral("a key still blank after the number field");
        }
        if (!problems.isEmpty()) qWarning().noquote() << problems.mid(0, 40).join(QLatin1Char('\n'));
        QCOMPARE(problems.size(), 0);
    }

    /*
     * Every language's letters and symbols on the measured screens of
     * screens(), at Medium height: PRE_SHOTS=<dir> saves them as pictures, to
     * look at with the phone's fonts (FONTCONFIG_FILE pointing at a copy of
     * its /usr/share/fonts and /etc/fonts).
     */
    void shots()
    {
        const QString dir = qEnvironmentVariable("PRE_SHOTS");
        if (dir.isEmpty()) QSKIP("PRE_SHOTS is not set");
        QDir().mkpath(dir);
        for (const Screen &sc : screens()) {
            if (!sc.measured) continue;
            QString name = sc.name;
            name.replace(QLatin1Char(' '), QLatin1Char('-'));
            // The keys only; [whole] keeps the room above them, where popups and menus go.
            auto save = [&](Pre::KeyboardView &view, const QString &what, bool whole = false) {
                QImage img(int(view.width()), int(view.height()), QImage::Format_ARGB32_Premultiplied);
                img.fill(Qt::black);
                QPainter p(&img);
                view.paint(&p);
                p.end();
                const int top = whole ? 0 : int(view.height() - view.keysHeight());
                img.copy(0, top, img.width(), img.height() - top)
                    .save(QStringLiteral("%1/%2-%3.png").arg(dir, name, what));
            };
            for (const Pre::Language &l : Pre::Language::all()) {
                Pre::KeyboardView view;
                showKeys(view, l, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
                for (auto page : { Pre::KeyboardView::Page::Letters, Pre::KeyboardView::Page::Symbols }) {
                    view.show(page);
                    save(view, l.id + (page == Pre::KeyboardView::Page::Letters ? QStringLiteral("-abc") : QStringLiteral("-sym")));
                }
            }
            // The other looks, in English: letters following shift, the White
            // style with shift on, and number and phone fields.
            const Pre::Language en = Pre::Language::byId(QStringLiteral("en"));
            // The language menu: from the space bar held still, and from the globe on the Sym page.
            {
                Pre::KeyboardView menu;
                showKeys(menu, en, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
                Pre::Settings ms = menu.settings();
                ms.languages = { en, Pre::Language::byId(QStringLiteral("de")), Pre::Language::byId(QStringLiteral("fr")) };
                menu.applySettings(ms);
                for (int i = 0; i < menu.m_zones.size(); ++i)
                    if (menu.m_zones[i].slot.key->kind == Pre::Kind::Space) { menu.showMenu(i); break; }
                save(menu, QStringLiteral("en-languages-space"), true);
                menu.m_menu.clear();
                menu.show(Pre::KeyboardView::Page::Symbols);
                menu.showMenu();
                save(menu, QStringLiteral("en-languages-globe"), true);
            }
            // The three heights, letters and symbols.
            for (auto height : { Pre::Settings::Height::Small, Pre::Settings::Height::Large }) {
                Pre::KeyboardView sized;
                showKeys(sized, en, sc.shortSide, sc.width, sc.landscape, height);
                const QString h = height == Pre::Settings::Height::Small ? QStringLiteral("small")
                                                                         : QStringLiteral("large");
                for (auto page : { Pre::KeyboardView::Page::Letters, Pre::KeyboardView::Page::Symbols }) {
                    sized.show(page);
                    save(sized, QStringLiteral("en-height-%1-%2")
                                    .arg(h, page == Pre::KeyboardView::Page::Letters ? QStringLiteral("abc")
                                                                                    : QStringLiteral("sym")));
                }
            }
            Pre::KeyboardView view;
            showKeys(view, en, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
            Pre::Settings s = view.settings();
            s.followShift = true;
            view.applySettings(s);
            save(view, QStringLiteral("en-small"));
            s.followShift = false;
            s.style = Pre::Style::White;
            view.applySettings(s);
            save(view, QStringLiteral("en-white"));
            view.m_shift = Pre::KeyboardView::Shift::Once;
            save(view, QStringLiteral("en-white-shift"));
            view.m_shift = Pre::KeyboardView::Shift::Off;
            s.style = Pre::Style::Orange;
            view.applySettings(s);
            view.setDigits(QStringLiteral("0123456789+-.,"));
            save(view, QStringLiteral("en-number"));
            view.setDigits(QStringLiteral("0123456789*#+"));
            save(view, QStringLiteral("en-phone"));
            // The bar: the next words before a word, and a word with its emoji.
            {
                Pre::KeyboardView bar;
                showKeys(bar, en, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
                Pre::Settings bs = bar.settings();
                bs.suggestions = true;
                bs.emojiSuggestions = true;
                bar.applySettings(bs);
                bar.setSuggestions({ QStringLiteral("don't"), QStringLiteral("think"), QStringLiteral("know") });
                save(bar, QStringLiteral("en-bar-next"));
                bar.setSuggestions({ QStringLiteral("pizzas"), QStringLiteral("pizza"), QStringLiteral("🍕") }, true);
                save(bar, QStringLiteral("en-bar-emoji"));
            }
        }
    }

private:
    /*!
     * A finger's path through [word]'s keys on [layout]: straight from key to
     * key, a point every sixth of the way, each [noise] key widths off at
     * random (seeded with [seed]). Empty if a letter has no key.
     */
    static QVector<QPointF> swipePath(const Pre::SwipeDecoder::Layout &layout, const QString &word, double noise,
                                      quint32 seed)
    {
        static const QHash<QChar, QChar> finals = { { QChar(0x05DA), QChar(0x05DB) }, { QChar(0x05DD), QChar(0x05DE) },
                                                    { QChar(0x05DF), QChar(0x05E0) }, { QChar(0x05E3), QChar(0x05E4) },
                                                    { QChar(0x05E5), QChar(0x05E6) } };
        QVector<int> keys;
        for (QChar c : word.toLower()) {
            if (c.isMark()) continue;
            int k = int(layout.keys.indexOf(c));
            QString spelt = QString(c);
            if (k < 0) spelt = Pre::Language::fold(QString(finals.value(c, c)));
            for (QChar f : spelt) {
                k = int(layout.keys.indexOf(f));
                if (k < 0) return {};
                if (keys.isEmpty() || keys.last() != k) keys << k;
            }
        }
        QRandomGenerator rng(seed);
        auto gauss = [&] {
            const double u = std::max(1e-9, rng.generateDouble()), v = rng.generateDouble();
            return std::sqrt(-2 * std::log(u)) * std::cos(2 * M_PI * v);
        };
        const double sd = noise * layout.keyWidth;
        QVector<QPointF> out;
        auto at = [&](int k) { return QPointF(layout.x[k], layout.y[k]); };
        out << at(keys.first()) + QPointF(gauss() * sd, gauss() * sd);
        for (int i = 1; i < keys.size(); ++i) {
            const QPointF a = at(keys[i - 1]), b = at(keys[i]);
            for (int j = 1; j <= 6; ++j) out << a + (b - a) * (j / 6.0) + QPointF(gauss() * sd, gauss() * sd);
        }
        if (keys.size() == 1) out << out.first() + QPointF(1, 0);
        return out;
    }

    static void writeFile(const QString &path, const QByteArray &bytes)
    {
        QDir().mkpath(QFileInfo(path).path());
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(bytes);
    }

    //! A presage database with only its 1-grams, as text2ngram writes them.
    static void writePresage(const QString &path, const QList<QPair<QString, int>> &words)
    {
        QDir().mkpath(QFileInfo(path).path());
        {
            QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("writer"));
            db.setDatabaseName(path);
            QVERIFY(db.open());
            QSqlQuery q(db);
            QVERIFY(q.exec(QStringLiteral("CREATE TABLE _1_gram (word TEXT, count INTEGER, UNIQUE(word))")));
            for (const auto &w : words) {
                q.prepare(QStringLiteral("INSERT INTO _1_gram VALUES (?, ?)"));
                q.addBindValue(w.first);
                q.addBindValue(w.second);
                QVERIFY(q.exec());
            }
            db.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("writer"));
    }

    //! What is wrong with the printing on [view]'s keys now, each line starting [where].
    static QStringList printingProblems(const Pre::KeyboardView &view, const QString &where)
    {
        QStringList out;
        auto outline = [](const QString &s, const QFont &f, const QPointF &at) {
            QPainterPath path;
            path.addText(at, f, s);
            return path;
        };
        auto letters = [](const QString &s) {
            for (QChar ch : s) if (!ch.isLetter() && !ch.isMark()) return false;
            return !s.isEmpty();
        };
        QHash<qreal, qreal> lineOfRow;
        qreal px = -1;
        for (const auto &z : view.m_zones) {
            if (!z.slot.drawn || z.slot.key->kind != Pre::Kind::Char) continue;
            const Pre::Key &key = *z.slot.key;
            const QString s = view.capText(key);
            // A key with nothing on it (a number field's letters) has nothing to be out of place.
            if (s.isEmpty()) continue;
            if (px < 0) px = z.letterPx;
            if (z.letterPx != px) out << where + s + QStringLiteral(": a size of its own");
            if (letters(s)) {
                const qreal line = lineOfRow.value(z.rect.top(), z.baseline);
                lineOfRow.insert(z.rect.top(), line);
                if (z.baseline != line)
                    out << where + s + QStringLiteral(": on a line %1 px off its row's").arg(z.baseline - line);
            }
            QFont f = view.m_bold;
            f.setPixelSize(int(z.letterPx));
            const QFontMetricsF fm(f);
            const QPainterPath ink = outline(s, f, view.letterAt(z, z.rect, fm));
            const QPainterPath face = view.m_painter.capShape(z.rect);
            if (!face.contains(ink)) out << where + s + QStringLiteral(": out of its key");
            const QString a = view.altText(key);
            if (a.isEmpty()) continue;
            f.setPixelSize(int(z.letterPx * 0.7));
            const QFontMetricsF am(f);
            const QPainterPath alt = outline(a, f, view.altAt(z, z.rect, am));
            if (!face.contains(alt)) out << where + s + QStringLiteral(": its ") + a + QStringLiteral(" out of the key");
            if (alt.intersects(ink)) out << where + s + QStringLiteral(": into its ") + a;
        }
        // As large as fits: a pixel more and something would not.
        if (px >= 0 && px < view.letterSize()) {
            Pre::KeyboardView &v = const_cast<Pre::KeyboardView &>(view);
            if (v.placeLetters(px + 1))
                out << where + QStringLiteral("shrunk to %1 px where %2 fits").arg(px).arg(px + 1);
            v.fitLetters();
        }
        return out;
    }

    /*!
     * A screen the keys are laid out for: its short side, which sets dp
     * (Keyboard.qml: dp = short side / 411.43), and the keyboard's width, the
     * short side in portrait and the long one in landscape.
     */
    struct Screen
    {
        QString name;
        int shortSide;
        int width;
        bool landscape;
        //! A device's, from luneos-components, rather than one of the sweep's.
        bool measured;
    };

    /*!
     * The SettingsStub.js of luneos-components: the LuneOS devices its tests
     * pretend to be, with their screens. $LUNEOS_COMPONENTS names the
     * checkout; without it, one beside this one is looked for. Empty if there
     * is none.
     */
    static QString profilesFile()
    {
        const QString stub = QStringLiteral("/test/imports/LunaNext/Common/SettingsStub.js");
        QStringList dirs;
        if (qEnvironmentVariableIsSet("LUNEOS_COMPONENTS")) dirs << qEnvironmentVariable("LUNEOS_COMPONENTS");
        dirs << QStringLiteral(PLUGINS_DIR "/../../luneos-components");
        for (const QString &d : dirs)
            if (QFileInfo::exists(d + stub)) return QDir::cleanPath(d + stub);
        return QString();
    }

    /*!
     * The screens to lay the keys out for.
     *
     * First every phone in luneos-components' profiles (a tablet keeps the
     * LuneOS keys), each way up: the Pre Keyboard turns with the phone. Then a
     * sweep of short sides from 240 to 1440 px, in portrait and in landscape at
     * 4:3, 16:9 and 2:1, for the screens no profile has yet. The sweep steps
     * 120 px; PRE_SCREENS=full makes it 8, for the odd sizes and fractional dp
     * in between (minutes rather than seconds).
     */
    static QList<Screen> screens()
    {
        QList<Screen> out;
        const QString file = profilesFile();
        QFile f(file);
        if (!file.isEmpty() && f.open(QIODevice::ReadOnly)) {
            QString js = QString::fromUtf8(f.readAll());
            js.replace(QStringLiteral(".pragma library"), QString());
            QJSEngine engine;
            const QJSValue result = engine.evaluate(js, file);
            const QJSValue profiles = engine.globalObject().property(QStringLiteral("profiles"));
            if (result.isError() || !profiles.isObject())
                qWarning() << "Cannot read the profiles in" << file << result.toString();
            QJSValueIterator it(profiles);
            QSet<QString> seen;
            while (it.hasNext()) {
                it.next();
                const QJSValue p = it.value();
                if (p.property(QStringLiteral("tabletUi")).toBool()) continue;
                const int w = p.property(QStringLiteral("displayWidth")).toInt();
                const int h = p.property(QStringLiteral("displayHeight")).toInt();
                if (w <= 0 || h <= 0) continue;
                const int shortSide = std::min(w, h), longSide = std::max(w, h);
                for (bool landscape : { false, true }) {
                    const int width = landscape ? longSide : shortSide;
                    const QString key = QStringLiteral("%1x%2").arg(width).arg(landscape ? shortSide : longSide);
                    if (seen.contains(key)) continue;
                    seen.insert(key);
                    out << Screen{ QStringLiteral("%1 %2%3").arg(it.name(), key,
                                                             landscape ? QStringLiteral(" landscape") : QString()),
                                   shortSide, width, landscape, true };
                }
            }
        }
        const int step = qEnvironmentVariable("PRE_SCREENS") == QLatin1String("full") ? 8 : 120;
        for (int side = 240; side <= 1440; side += step) {
            out << Screen{ QStringLiteral("%1 portrait").arg(side), side, side, false, false };
            for (const auto &r : { std::make_pair(4, 3), std::make_pair(16, 9), std::make_pair(2, 1) })
                out << Screen{ QStringLiteral("%1 landscape %2:%3").arg(side).arg(r.first).arg(r.second),
                               side, side * r.first / r.second, true, false };
        }
        return out;
    }

    //! What is wrong with where [view]'s keys are, each line starting [where].
    static QStringList layoutProblems(const Pre::KeyboardView &view, const QString &where)
    {
        QStringList out;
        if (view.m_zones.isEmpty()) return { where + QStringLiteral("no keys") };
        const QRectF keys(0, view.height() - view.keysHeight(), view.width(), view.keysHeight());
        for (int i = 0; i < view.m_zones.size(); ++i) {
            const QRectF &r = view.m_zones[i].rect;
            const QString key = view.m_zones[i].slot.key->text + view.m_zones[i].slot.key->label;
            if (r.width() < 1 || r.height() < 1) out << where + key + QStringLiteral(": no room");
            if (!keys.adjusted(-0.5, -0.5, 0.5, 0.5).contains(r))
                out << where + key + QStringLiteral(": off the keyboard");
            for (int j = i + 1; j < view.m_zones.size(); ++j)
                if (r.intersected(view.m_zones[j].rect).width() > 0.5
                    && r.intersected(view.m_zones[j].rect).height() > 0.5)
                    out << where + key + QStringLiteral(": on top of another key");
            // In landscape the screen is twice as wide as ten keys of the
            // portrait shape: a key may be a little wider, not stretched.
            if (view.m_zones[i].slot.key->kind == Pre::Kind::Char
                && r.width() > view.m_zones[i].slot.weight * 1.35 * view.rowHeight() + 1)
                out << where + key + QStringLiteral(": stretched, %1 wide in a row %2 tall")
                                         .arg(r.width()).arg(view.rowHeight());
        }
        // The keys in the middle, and a touch beside them goes to the key at
        // that end of its row.
        qreal left = view.width(), right = 0;
        for (const auto &z : view.m_zones) { left = std::min(left, z.rect.left()); right = std::max(right, z.rect.right()); }
        if (std::abs(left - (view.width() - right)) > 1)
            out << where + QStringLiteral("keys not in the middle: %1 px left of them, %2 right").arg(left).arg(view.width() - right);
        for (int i = 0; i < view.m_zones.size(); ++i) {
            const QRectF &r = view.m_zones[i].rect;
            const bool first = i == 0 || view.m_zones[i - 1].rect.top() != r.top();
            const bool last = i + 1 == view.m_zones.size() || view.m_zones[i + 1].rect.top() != r.top();
            if (first && view.zoneAt(0, r.center().y()) != i)
                out << where + QStringLiteral("a touch at the left edge misses the row's first key");
            if (last && view.zoneAt(view.width() - 0.5, r.center().y()) != i)
                out << where + QStringLiteral("a touch at the right edge misses the row's last key");
        }
        return out;
    }

    /*!
     * In a row without alternates (the Sym page, a number field's keypad) a
     * symbol's ink is centred on its key's face, so < > ~ { } and the digits
     * line up by their middles. A mark whose height is its meaning (. , ' ^)
     * keeps its place on the line.
     */
    static QStringList centringProblems(const Pre::KeyboardView &view, const QString &where)
    {
        QStringList out;
        for (int a = 0; a < view.m_zones.size();) {
            int b = a;
            while (b < view.m_zones.size() && view.m_zones[b].rect.top() == view.m_zones[a].rect.top()) ++b;
            bool alts = false;
            for (int k = a; k < b; ++k)
                if (view.m_zones[k].slot.drawn && view.m_zones[k].slot.key->kind == Pre::Kind::Char)
                    alts = alts || !view.altText(*view.m_zones[k].slot.key).isEmpty();
            for (int k = a; !alts && k < b; ++k) {
                const auto &z = view.m_zones[k];
                if (!z.slot.drawn || z.slot.key->kind != Pre::Kind::Char) continue;
                const QString t = view.capText(*z.slot.key);
                bool letters = !t.isEmpty();
                for (QChar c : t) letters = letters && (c.isLetter() || c.isMark());
                if (t.isEmpty() || letters || Pre::KeyboardView::placedMark(t)) continue;
                QFont f = view.m_bold;
                f.setPixelSize(std::max(1, int(z.letterPx)));
                const QRectF ink = QFontMetricsF(f).tightBoundingRect(t);
                const QRectF face = view.m_painter.capFace(QRectF(0, 0, z.rect.width(), z.rect.height()));
                const qreal off = z.baseline + (ink.top() + ink.bottom()) / 2 - face.center().y();
                if (std::abs(off) > 1)
                    out << where + t + QStringLiteral(": %1 px off the middle of its key").arg(off);
            }
            a = b;
        }
        return out;
    }

    //! [view] as a screen with [shortSide] shows it, [width] px wide, in [language], with [height] rows.
    static void showKeys(Pre::KeyboardView &view, const Pre::Language &language, int shortSide, int width,
                         bool landscape, Pre::Settings::Height height)
    {
        Pre::Settings s;
        s.language = language;
        s.languages = { language };
        s.height = height;
        view.setDp(shortSide / 411.43);
        view.setLandscape(landscape);
        view.applySettings(s);
        view.setWidth(width);
        view.setHeight(view.headroom() + view.keysHeight());
    }
};

QTEST_MAIN(TestPreKeyboard)

#include "ut_prekeyboard.moc"
