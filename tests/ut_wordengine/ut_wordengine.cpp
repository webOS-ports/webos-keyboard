/*
 * This file is part of the LuneOS keyboard
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 */

// The word engine between the Text Assist settings and the language plugin's
// checker, and what the editor does with it when a word is finished.
//
// "Autocorrect" and "Spell checking" are two settings, but the checker used to
// be loaded for spell checking alone: with autocorrect on and spell checking
// off no word was ever wrong, so none was corrected and no word of the owner's
// could be saved. And the language's correction (dont -> don't, i -> I) only
// ran when presage had a word or the checker said the word was wrong, which a
// dictionary that knows "cant" and "i" never does.
//
// The plugin here stands in for the western plugins' SpellChecker: spell() is
// true for everything and suggest() empty while it is not loaded, and a word
// is only saved while it is.

#include "abstracttexteditor.h"
#include "logic/abstractlanguagefeatures.h"
#include "logic/languageplugininterface.h"
#include "logic/wordengine.h"
#include "models/key.h"
#include "models/text.h"

#include <QtTest>

using namespace MaliitKeyboard;

namespace {

class SpaceAfterReplaced : public AbstractLanguageFeatures
{
public:
    bool activateAutoCaps(const QString &) const override { return false; }
    // As the western plugins: a word space put in is followed by a space.
    QString appendixForReplacedPreedit(const QString &preedit) const override
    {
        return preedit.isEmpty() ? QString() : QStringLiteral(" ");
    }
};

class FakePlugin : public LanguagePluginInterface
{
public:
    QSet<QString> dictionary;
    QHash<QString, QStringList> suggestions;
    QStringList predictions;
    QStringList saved;
    bool loaded = false;
    int loads = 0;

    void parse(const QString &, const QString &) override {}
    QStringList getWordCandidates() override { return predictions; }
    void wordCandidateSelected(QString) override {}
    AbstractLanguageFeatures *languageFeature() override { return &features; }

    bool spellCheckerEnabled() override { return loaded; }
    bool setSpellCheckerEnabled(bool enabled) override
    {
        if (enabled && !loaded)
            ++loads;
        loaded = enabled;
        return true;
    }
    bool spell(const QString &word) override
    {
        return !loaded || dictionary.contains(word) || saved.contains(word);
    }
    QStringList spellCheckerSuggest(const QString &word, int limit) override
    {
        if (!loaded)
            return QStringList();
        const QStringList all = suggestions.value(word);
        return limit < 0 ? all : all.mid(0, limit);
    }
    void addToSpellCheckerUserWordList(const QString &word) override
    {
        if (loaded && !saved.contains(word))
            saved << word;
    }
    bool setSpellCheckerLanguage(const QString &) override { return true; }
    void removeFromSpellCheckerUserWordList(const QString &word) override { saved.removeAll(word); }
    QStringList spellCheckerUserWords() override { return saved; }

private:
    SpaceAfterReplaced features;
};

class Engine : public Logic::WordEngine
{
public:
    using Logic::WordEngine::useLanguagePlugin;

    explicit Engine(FakePlugin *plugin)
    {
        // No plugin can be loaded here: the language is set first, then the
        // stand-in takes the place of the one that did not load.
        onLanguageChanged(QStringLiteral("en"));
        useLanguagePlugin(plugin);
    }
};

//! Collects what the editor sends to the application.
class Editor : public AbstractTextEditor
{
public:
    explicit Editor(Logic::AbstractWordEngine *engine)
        : AbstractTextEditor(EditorOptions(), new Model::Text, engine)
    {}

    QString committed;

    void type(const QString &keys)
    {
        for (const QChar c : keys) {
            Key key;
            if (c == QLatin1Char(' ')) {
                key.setAction(Key::ActionSpace);
            } else {
                key.setAction(Key::ActionInsert);
                key.setLabel(QString(c));
            }
            onKeyPressed(key);
            onKeyReleased(key);
        }
    }

private:
    void sendPreeditString(const QString &, Model::Text::PreeditFace, const Replacement &) override {}
    void sendCommitString(const QString &text) override { committed += text; }
    void sendKeyEvent(const QKeyEvent &) override {}
    void invokeAction(const QString &, const QKeySequence &) override {}
};

void englishWords(FakePlugin *plugin)
{
    // As en_US.dic: "cant", "wont" and the letter "i" are words of their own.
    plugin->dictionary = { QStringLiteral("the"), QStringLiteral("cant"), QStringLiteral("wont"),
                           QStringLiteral("i"), QStringLiteral("I"), QStringLiteral("can't") };
    plugin->suggestions.insert(QStringLiteral("teh"), { QStringLiteral("the"), QStringLiteral("ten") });
    plugin->suggestions.insert(QStringLiteral("webos"), { QStringLiteral("webs") });
}

struct Settings
{
    bool spellChecking;
    bool autoCorrection;
};

} // namespace

Q_DECLARE_METATYPE(Settings)

class TestWordEngine : public QObject
{
    Q_OBJECT

    static void apply(Logic::WordEngine *engine, AbstractTextEditor *editor, const Settings &s)
    {
        // As InputMethodPrivate::registerWordEngineSetting and registerAutoCorrectSetting.
        engine->setSpellcheckerEnabled(s.spellChecking);
        engine->setAutoCorrectEnabled(s.autoCorrection);
        if (editor)
            editor->setAutoCorrectEnabled(s.autoCorrection);
    }

private Q_SLOTS:
    // ---- The checker follows either setting ----

    void checkerLoaded_data()
    {
        QTest::addColumn<Settings>("settings");
        QTest::addColumn<bool>("loaded");
        QTest::addColumn<bool>("misspelled");
        QTest::addColumn<bool>("marked");

        QTest::newRow("both on") << Settings{ true, true } << true << true << true;
        QTest::newRow("autocorrect only") << Settings{ false, true } << true << true << false;
        QTest::newRow("spell checking only") << Settings{ true, false } << true << true << true;
        QTest::newRow("both off") << Settings{ false, false } << false << false << false;
    }

    void checkerLoaded()
    {
        QFETCH(Settings, settings);
        QFETCH(bool, loaded);
        QFETCH(bool, misspelled);
        QFETCH(bool, marked);

        FakePlugin plugin;
        englishWords(&plugin);
        Engine engine(&plugin);
        apply(&engine, nullptr, settings);

        QCOMPARE(plugin.loaded, loaded);
        QCOMPARE(engine.isMisspelled(QStringLiteral("teh")), misspelled);
        QVERIFY(!engine.isMisspelled(QStringLiteral("the")));
        QCOMPARE(engine.marksMisspellings(), marked);
        QCOMPARE(engine.spellingSuggestions(QStringLiteral("teh"), 4).isEmpty(), !loaded);
    }

    // Spell checking switched off under autocorrect leaves the checker loaded;
    // autocorrect switched off then unloads it.
    void eitherSettingKeepsTheCheckerLoaded()
    {
        FakePlugin plugin;
        Engine engine(&plugin);
        apply(&engine, nullptr, Settings{ true, true });
        QVERIFY(plugin.loaded);

        engine.setSpellcheckerEnabled(false);
        QVERIFY(plugin.loaded);
        QCOMPARE(plugin.loads, 1);

        engine.setAutoCorrectEnabled(false);
        QVERIFY(!plugin.loaded);

        engine.setAutoCorrectEnabled(true);
        QVERIFY(plugin.loaded);
        engine.setSpellcheckerEnabled(true);
        engine.setAutoCorrectEnabled(false);
        QVERIFY(plugin.loaded);
    }

    // The engine is on for autocorrect alone, so a word is corrected without
    // predictions, as it is for spell checking alone.
    void enabledForAutocorrectAlone()
    {
        FakePlugin plugin;
        Engine engine(&plugin);
        engine.setEnabled(true);
        engine.setWordPredictionEnabled(false);
        QVERIFY(!engine.isEnabled());
        QSignalSpy spy(&engine, &Logic::AbstractWordEngine::enabledChanged);
        engine.setAutoCorrectEnabled(true);
        QVERIFY(engine.isEnabled());
        QCOMPARE(spy.count(), 1);
    }

    void userWordsWithAutocorrectAlone()
    {
        FakePlugin plugin;
        Engine engine(&plugin);
        apply(&engine, nullptr, Settings{ false, true });

        engine.addToUserDictionary(QStringLiteral("webOS"));
        QCOMPARE(plugin.saved, QStringList{ QStringLiteral("webOS") });
        QCOMPARE(engine.userWords(), QStringList{ QStringLiteral("webOS") });
        QVERIFY(!engine.isMisspelled(QStringLiteral("webOS")));
    }

    void noUserWordsWithBothOff()
    {
        FakePlugin plugin;
        plugin.saved << QStringLiteral("webOS");
        Engine engine(&plugin);
        apply(&engine, nullptr, Settings{ false, false });
        QVERIFY(engine.userWords().isEmpty());
    }

    // A saved word typed in another case is put in as saved, autocorrect alone.
    void userWordCaseWithAutocorrectAlone()
    {
        FakePlugin plugin;
        englishWords(&plugin);
        plugin.saved << QStringLiteral("webOS");
        Engine *engine = new Engine(&plugin);
        Editor editor(engine);
        engine->setEnabled(true);
        apply(engine, &editor, Settings{ false, true });

        editor.type(QStringLiteral("webos "));
        QCOMPARE(editor.committed, QStringLiteral("webOS "));
    }

    // ---- What is put in ----

    void spaceCorrects_data()
    {
        QTest::addColumn<Settings>("settings");
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QString>("committed");

        QTest::newRow("teh, both on") << Settings{ true, true } << "teh " << "the ";
        QTest::newRow("teh, autocorrect only") << Settings{ false, true } << "teh " << "the ";
        QTest::newRow("teh, spell checking only") << Settings{ true, false } << "teh " << "teh ";
        QTest::newRow("teh, both off") << Settings{ false, false } << "teh " << "teh ";
        // s8-6: the period ends the word, corrected, and stays.
        QTest::newRow("teh., both on") << Settings{ true, true } << "teh. " << "the. ";
        QTest::newRow("teh., autocorrect only") << Settings{ false, true } << "teh. " << "the. ";
        QTest::newRow("teh., spell checking only") << Settings{ true, false } << "teh. " << "teh. ";
        QTest::newRow("the., autocorrect only") << Settings{ false, true } << "the. " << "the. ";
    }

    void spaceCorrects()
    {
        QFETCH(Settings, settings);
        QFETCH(QString, typed);
        QFETCH(QString, committed);

        FakePlugin plugin;
        englishWords(&plugin);
        Engine *engine = new Engine(&plugin);
        Editor editor(engine);
        engine->setEnabled(true);
        apply(engine, &editor, settings);

        editor.type(typed);
        QCOMPARE(editor.committed, committed);
    }

    // With autocorrect alone a word is corrected, not marked: a misspelling
    // nothing is suggested for keeps the plain face.
    void notMarkedWithAutocorrectAlone_data()
    {
        QTest::addColumn<Settings>("settings");
        QTest::addColumn<int>("face");

        QTest::newRow("autocorrect only") << Settings{ false, true } << int(Model::Text::PreeditDefault);
        QTest::newRow("spell checking only") << Settings{ true, false } << int(Model::Text::PreeditNoCandidates);
    }

    void notMarkedWithAutocorrectAlone()
    {
        QFETCH(Settings, settings);
        QFETCH(int, face);

        FakePlugin plugin;
        Engine engine(&plugin);
        engine.setEnabled(true);
        apply(&engine, nullptr, settings);

        Model::Text text;
        text.setPreedit(QStringLiteral("xqzv"));
        engine.computeCandidates(&text);
        QCOMPARE(int(text.preeditFace()), face);
    }

    // ---- The language's correction without a word from presage ----

    void contractionWithoutPredictions_data()
    {
        QTest::addColumn<Settings>("settings");
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QString>("committed");

        // The dictionary knows "cant", "wont" and "i": the checker has nothing to say.
        QTest::newRow("cant") << Settings{ true, true } << "cant " << "can't ";
        QTest::newRow("wont") << Settings{ true, true } << "wont " << "won't ";
        QTest::newRow("i") << Settings{ true, true } << "i " << "I ";
        QTest::newRow("Cant") << Settings{ true, true } << "Cant " << "Can't ";
        // Not in the dictionary at all.
        QTest::newRow("dont") << Settings{ true, true } << "dont " << "don't ";
        QTest::newRow("im, autocorrect only") << Settings{ false, true } << "im " << "I'm ";
        QTest::newRow("cant, autocorrect only") << Settings{ false, true } << "cant " << "can't ";
        // Autocorrect off: as typed.
        QTest::newRow("cant, spell checking only") << Settings{ true, false } << "cant " << "cant ";
        // A word of the dictionary's own with nothing to change: as typed.
        QTest::newRow("the") << Settings{ true, true } << "the " << "the ";
    }

    void contractionWithoutPredictions()
    {
        QFETCH(Settings, settings);
        QFETCH(QString, typed);
        QFETCH(QString, committed);

        FakePlugin plugin;
        englishWords(&plugin);
        Engine *engine = new Engine(&plugin);
        Editor editor(engine);
        engine->setEnabled(true);
        engine->setWordPredictionEnabled(true);
        apply(engine, &editor, settings);

        editor.type(typed);
        QCOMPARE(editor.committed, committed);
    }

    // A word the dictionary knows and presage has nothing for is offered as
    // nothing, as before: no candidate of its own.
    void knownWordWithoutPredictionsHasNoCandidate()
    {
        FakePlugin plugin;
        englishWords(&plugin);
        Engine engine(&plugin);
        engine.setEnabled(true);
        engine.setWordPredictionEnabled(true);
        apply(&engine, nullptr, Settings{ true, true });

        Model::Text text;
        text.setPreedit(QStringLiteral("the"));
        int changes = 0;
        WordCandidateList offered;
        connect(&engine, &Logic::AbstractWordEngine::candidatesChanged, this,
                [&](const WordCandidateList &candidates) { ++changes; offered = candidates; });
        engine.computeCandidates(&text);
        QCOMPARE(changes, 1);
        QVERIFY(offered.isEmpty());
        QVERIFY(text.primaryCandidate().isEmpty());

        text.setPreedit(QStringLiteral("cant"));
        engine.computeCandidates(&text);
        QCOMPARE(text.primaryCandidate(), QStringLiteral("can't"));
    }

    // A language whose plugin did not load leaves none, with prediction still
    // on: a deleted word typed then has nothing to ask for suggestions.
    void deletedWordWithoutAPlugin()
    {
        FakePlugin plugin;
        Engine engine(&plugin);
        engine.setEnabled(true);
        engine.setWordPredictionEnabled(true);
        engine.useLanguagePlugin(nullptr);
        engine.setBlockedWords({ QStringLiteral("teh") });
        QVERIFY(engine.isEnabled());

        Model::Text text;
        text.setPreedit(QStringLiteral("teh"));
        engine.computeCandidates(&text);
        QVERIFY(text.primaryCandidate().isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestWordEngine)
#include "ut_wordengine.moc"
