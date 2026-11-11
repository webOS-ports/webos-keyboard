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
#include "prekeyboardview.h"
#include "prekeymap.h"
#include "prelanguage.h"
#include "presettings.h"

#include "abstracttexteditor.h"
#include "logic/abstractlanguagefeatures.h"
#include "logic/eventhandler.h"
#include "models/text.h"

#include <QDir>
#include <QFileInfo>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QJsonArray>
#include <QSet>
#include <QPainter>
#include <QPainterPath>
#include <QtTest>

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
        b.savePrefs = [this](const QJsonObject &) { calls << QStringLiteral("save"); };
        b.hide = [this] { calls << QStringLiteral("hide"); };
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
        s.suggestions = true;
        const QJsonObject pre = s.toJson(QJsonObject{ { QStringLiteral("recentEmoji"), QJsonArray{ QStringLiteral("🙂") } } });
        const QStringList keys = pre.keys();
        QCOMPARE(QSet<QString>(keys.begin(), keys.end()),
                 (QSet<QString>{ QStringLiteral("style"), QStringLiteral("height"), QStringLiteral("holdForMore"),
                                 QStringLiteral("vibrate"), QStringLiteral("recentEmoji") }));
        const Pre::Settings back = Pre::Settings::fromJson(pre);
        QCOMPARE(back.style, Pre::Style::White);
        QCOMPARE(back.height, Pre::Settings::Height::Large);
        QVERIFY(back.holdForMore && back.vibrate);
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

    void wordsAreAddedToLuneOSsDictionary()
    {
        Recorder rec;
        Pre::Controller c;
        c.setBridge(rec.bridge());
        QVERIFY(c.canAdd(QStringLiteral("LuneOS")));
        QVERIFY(!c.canAdd(QString()));
        QVERIFY(!c.canRemove(QStringLiteral("the")));
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
                    wrong << layoutProblems(view, where) << printingProblems(view, where);
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
            for (const Pre::Language &l : Pre::Language::all()) {
                Pre::KeyboardView view;
                showKeys(view, l, sc.shortSide, sc.width, sc.landscape, Pre::Settings::Height::Medium);
                for (auto page : { Pre::KeyboardView::Page::Letters, Pre::KeyboardView::Page::Symbols }) {
                    view.show(page);
                    QImage img(int(view.width()), int(view.height()), QImage::Format_ARGB32_Premultiplied);
                    img.fill(Qt::black);
                    QPainter p(&img);
                    view.paint(&p);
                    p.end();
                    const int top = int(view.height() - view.keysHeight());
                    QString name = sc.name;
                    name.replace(QLatin1Char(' '), QLatin1Char('-'));
                    img.copy(0, top, img.width(), img.height() - top)
                        .save(QStringLiteral("%1/%2-%3-%4.png").arg(dir, name, l.id,
                              page == Pre::KeyboardView::Page::Letters ? QStringLiteral("abc") : QStringLiteral("sym")));
                }
            }
        }
    }

private:
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
            if (key.alt.isEmpty()) continue;
            f.setPixelSize(int(z.letterPx * 0.7));
            const QFontMetricsF am(f);
            const QPainterPath alt = outline(key.alt, f, view.altAt(z, z.rect, am));
            if (!face.contains(alt)) out << where + s + QStringLiteral(": its ") + key.alt + QStringLiteral(" out of the key");
            if (alt.intersects(ink)) out << where + s + QStringLiteral(": into its ") + key.alt;
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
