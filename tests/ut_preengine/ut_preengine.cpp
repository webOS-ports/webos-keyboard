/*
 * The Pre Keyboard's engine, on the build machine: the tests of the Android Pre
 * Keyboard (github.com/alan-morford/prekeyboard), ported with the code they test.
 * They read the dictionaries that ship, in data/pre.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
 * Licensed under the Apache License, Version 2.0.
 */

#include "precomposer.h"
#include "precorrector.h"
#include "preklipy.h"
#include "prekeymap.h"
#include "prelanguage.h"
#include "preswipedecoder.h"
#include "prewords.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QSet>
#include <QStringList>
#include <QTextStream>

#include <cstdio>
#include <random>

using namespace Pre;

static QString g_data;
static int g_failed = 0, g_passed = 0;
static QString g_test;

#define CHECK(cond) do { if (cond) ++g_passed; else { ++g_failed; \
    fprintf(stderr, "FAIL %s:%d [%s] %s\n", __FILE__, __LINE__, qPrintable(g_test), #cond); } } while (0)
#define EQ(a, b) do { const auto _a = (a); const auto _b = (b); if (_a == _b) ++g_passed; else { ++g_failed; \
    fprintf(stderr, "FAIL %s:%d [%s] %s == %s\n   got: %s\n  want: %s\n", __FILE__, __LINE__, qPrintable(g_test), \
            #a, #b, qPrintable(show(_a)), qPrintable(show(_b))); } } while (0)

static QString show(const QString &s) { return s.isNull() ? QStringLiteral("<null>") : QLatin1Char('"') + s + QLatin1Char('"'); }
static QString show(const QStringList &l) { return QLatin1Char('[') + l.join(QStringLiteral(", ")) + QLatin1Char(']'); }
static QString show(bool b) { return b ? QStringLiteral("true") : QStringLiteral("false"); }
static QString show(int i) { return QString::number(i); }
static QString show(qsizetype i) { return QString::number(i); }
static QString show(const QPair<QString, QString> &p) { return show(p.first) + QStringLiteral(" to ") + show(p.second); }
static QString show(const QPair<QString, int> &p) { return show(p.first) + QStringLiteral(" to ") + show(p.second); }

static QStringList lines(const QString &name)
{
    QFile f(g_data + QLatin1Char('/') + name);
    if (!f.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "cannot read %s\n", qPrintable(f.fileName()));
        return {};
    }
    return QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
}

static QSharedPointer<Corrector> corrector(const Language &lang)
{
    QStringList all;
    for (const QString &d : lang.dictionaries())
        all += lines(d);
    return QSharedPointer<Corrector>(new Corrector(all, lang));
}

// --- Composer -----------------------------------------------------------------------------

struct StringField : Composer::Field
{
    QString text;
    QString before(int n) override { return text.right(n); }
    void remove(int n) override { text.chop(qMin(n, text.length())); }
    void commit(const QString &s) override { text += s; }
};

struct ComposerFixture
{
    StringField field;
    qint64 now = 10000;
    Composer composer;
    Composer::Rules on, off;
    QStringList learned;
    QList<QPair<QString, int>> seen;

    ComposerFixture(const QSharedPointer<Corrector> &c) : composer(&field, [this] { return now; })
    {
        off.autocorrect = false;
        off.doubleSpacePeriod = false;
        composer.corrector = c;
        composer.onLearned = [this](const QString &w) { learned << w; };
    }
    void type(const QString &s) { type(s, on); }
    void type(const QString &s, const Composer::Rules &r) { for (QChar ch : s) { composer.type(QString(ch), r); now += 300; } }
    void clear() { field.text.clear(); composer.reset(); }
};

static void composerTests(const QSharedPointer<Corrector> &c)
{
    {
        g_test = "correctsWhenTheWordEnds"; ComposerFixture t(c);
        t.type("i dont think teh cat is here.");
        EQ(t.field.text, QString("I don't think the cat is here. "));
    }
    {
        g_test = "nothingWhenSwitchedOff"; ComposerFixture t(c);
        t.type("i dont think teh cat", t.off);
        EQ(t.field.text, QString("i dont think teh cat"));
    }
    {
        g_test = "backspaceUndoesWithoutLearning"; ComposerFixture t(c);
        t.type("teh ");
        EQ(t.field.text, QString("the "));
        EQ(t.composer.backspace(), true);
        EQ(t.field.text, QString("teh "));
        EQ(t.learned, QStringList());
        t.type("teh ");
        EQ(t.field.text, QString("teh the "));
        EQ(t.composer.backspace(), true);
        EQ(t.field.text, QString("teh teh "));
        EQ(t.composer.backspace(), false);
    }
    {
        g_test = "contractionsFromContext"; ComposerFixture t(c);
        t.type("I think ill take a nap. ill be back, its fine. The dog wagged its tail ");
        EQ(t.field.text, QString("I think I'll take a nap. I'll be back, it's fine. The dog wagged its tail "));
    }
    {
        g_test = "doubleSpacePeriod"; ComposerFixture t(c);
        t.type("Hello");
        t.composer.type(" ", t.on); t.now += 200; t.composer.type(" ", t.on);
        EQ(t.field.text, QString("Hello. "));
        t.field.text.clear(); t.type("Hi"); t.composer.type(" ", t.on); t.now += 1500; t.composer.type(" ", t.on);
        EQ(t.field.text, QString("Hi  "));
        t.field.text.clear(); t.type("Hi."); t.composer.type(" ", t.on); t.now += 200; t.composer.type(" ", t.on);
        EQ(t.field.text, QString("Hi.  "));
        t.field.text.clear(); t.type("Hello", t.off); t.composer.type(" ", t.off); t.now += 200; t.composer.type(" ", t.off);
        EQ(t.field.text, QString("Hello  "));
    }
    {
        g_test = "leavesAddressesAndNumbersAlone"; ComposerFixture t(c);
        t.type("mail teh@exmaple.com 3teh ");
        EQ(t.field.text, QString("mail teh@exmaple.com 3teh "));
    }
    {
        g_test = "enterFinishesTheWord"; ComposerFixture t(c);
        t.type("teh"); t.composer.beforeEnter(t.on);
        EQ(t.field.text, QString("the"));
    }
    {
        g_test = "swipedWordsSpaceThemselvesAndBackspaceTakesTheWord"; ComposerFixture t(c);
        t.type("Hello");
        t.composer.swipe("world", t.on); t.composer.swipe("again", t.on);
        EQ(t.field.text, QString("Hello world again "));
        EQ(t.composer.backspace(), true);
        EQ(t.field.text, QString("Hello world "));
        t.type("!");
        EQ(t.field.text, QString("Hello world!"));
        t.clear();
        t.composer.swipe("one", t.on); t.type("("); t.composer.swipe("two", t.on);
        EQ(t.field.text, QString("one (two "));
    }
    {
        g_test = "typingAndSwipingMixWithoutLosingSpaces"; ComposerFixture t(c);
        t.composer.swipe("Going", t.on); t.type("to"); t.composer.swipe("town", t.on);
        EQ(t.field.text, QString("Going to town "));
        t.clear();
        t.composer.swipe("Going", t.on); t.type(" to");
        EQ(t.field.text, QString("Going to"));
    }
    {
        g_test = "aPeriodAfterASwipeLandsOnTheWord"; ComposerFixture t(c);
        t.type("See you"); t.composer.swipe("soon", t.on); t.type(".");
        EQ(t.field.text, QString("See you soon. "));
        t.type("?");
        EQ(t.field.text, QString("See you soon.?"));
    }
    {
        g_test = "backspaceAfterASwipeTakesItsLeadingSpaceToo"; ComposerFixture t(c);
        t.type("Hello"); t.composer.swipe("world", t.on);
        EQ(t.field.text, QString("Hello world "));
        EQ(t.composer.backspace(), true);
        EQ(t.field.text, QString("Hello"));
    }
    {
        g_test = "aLoneIIsCapitalisedWhenASwipeFollowsIt"; ComposerFixture t(c);
        t.type("i"); t.composer.swipe("think", t.on);
        EQ(t.field.text, QString("I think "));
        t.clear();
        t.type("so i"); t.composer.swipe("went", t.on);
        EQ(t.field.text, QString("so I went "));
    }
    {
        g_test = "aPeriodBringsASpaceThatBackspaceTakesAlone"; ComposerFixture t(c);
        t.type("The end.");
        EQ(t.field.text, QString("The end. "));
        EQ(t.composer.backspace(), false);
        t.field.text.chop(1);
        EQ(t.field.text, QString("The end."));
        EQ(t.composer.backspace(), false);
        t.clear();
        t.type("pi is 3.14 .net");
        EQ(t.field.text, QString("pi is 3.14 .net"));
        t.clear();
        Composer::Rules url; url.autocorrect = false; url.doubleSpacePeriod = false; url.prose = false;
        t.type("example.com", url);
        EQ(t.field.text, QString("example.com"));
        t.clear();
        t.type("mail me@site.com or www.site.org/a.b ");
        EQ(t.field.text, QString("mail me@site.com or www.site.org/a.b "));
        t.clear();
        t.type("Wait..."); EQ(t.field.text, QString("Wait... "));
        t.composer.beforeEnter(t.on); EQ(t.field.text, QString("Wait..."));
    }
    {
        g_test = "punctuationAfterATypedSpaceSnapsToTheWord"; ComposerFixture t(c);
        t.type("Wait ,");
        EQ(t.field.text, QString("Wait, "));
        t.type(" so");
        EQ(t.field.text, QString("Wait, so"));
        t.clear(); t.type("Really ?"); EQ(t.field.text, QString("Really? "));
        t.clear(); t.type("The end ."); EQ(t.field.text, QString("The end. "));
        t.clear(); t.type("teh !"); EQ(t.field.text, QString("the! "));
        t.clear(); t.type("I like teh"); t.composer.pick("the", false); t.type(",");
        EQ(t.field.text, QString("I like the, "));
        t.clear(); t.type("a "); t.composer.forget(); t.type(",");
        EQ(t.field.text, QString("a ,"));
        t.clear(); t.type("so "); t.now += 1500; t.type(" ,"); EQ(t.field.text, QString("so  ,"));
        t.clear(); t.type("3 ."); EQ(t.field.text, QString("3 ."));
    }
    {
        g_test = "pickingFromTheBar"; ComposerFixture t(c);
        t.type("I like teh");
        EQ(t.composer.currentWord(), QString("teh"));
        t.composer.pick("the", false);
        EQ(t.field.text, QString("I like the "));
        t.composer.type(" ", t.on);
        EQ(t.field.text, QString("I like the. "));
        t.clear();
        t.composer.swipe("in", t.on); t.composer.pick("on", false);
        EQ(t.field.text, QString("on "));
        EQ(t.composer.swiped(), QString("on"));
        // The word as typed, picked over its correction, is learned and kept from then on.
        ComposerFixture u(corrector(Language::enUS()));
        u.type("Zorp"); u.composer.pick("Zorp", true);
        EQ(u.learned, QStringList({ "Zorp" }));
        u.type("zorp ");
        EQ(u.field.text, QString("Zorp zorp "));
    }
    {
        g_test = "currentWordOnlyAtAWordsEnd"; ComposerFixture t(c);
        t.type("say hel");
        EQ(t.composer.currentWord(), QString("hel"));
        t.type(" ");
        EQ(t.composer.currentWord(), QString(""));
        t.type("mail@exa");
        EQ(t.composer.currentWord(), QString(""));
    }
    {
        g_test = "whatTheNextWordFollows"; ComposerFixture t(c);
        EQ(t.composer.nextContext(), NextWords::START);
        t.type("I want ");
        EQ(t.composer.nextContext(), QString("want"));
        t.field.text = "I ";
        EQ(t.composer.finishedPair(), (QPair<QString, QString>(NextWords::START, "I")));
        t.field.text = "I want";
        EQ(t.composer.nextContext(), QString());
        t.field.text = "Hi there. ";
        EQ(t.composer.nextContext(), NextWords::START);
        EQ(t.composer.finishedPair(), (QPair<QString, QString>("hi", "there")));
        t.field.text = "Well, ";
        EQ(t.composer.nextContext(), NextWords::COMMA);
        t.field.text = "see www.site.com ";
        EQ(t.composer.nextContext(), QString());
        EQ(t.composer.finishedPair(), (QPair<QString, QString>()));
        t.field.text = "I want";
        EQ(t.composer.finishedPair(), (QPair<QString, QString>()));
    }
    {
        g_test = "nextWords";
        NextWords nw(lines("next_words.txt"));
        EQ(nw.predict("want").value(0), QString("to"));
        CHECK(nw.predict(NextWords::START).contains("I"));
        nw.learn("want", "pizza"); CHECK(nw.predict("want").value(0) != "pizza");
        nw.learn("want", "pizza"); EQ(nw.predict("want").value(0), QString("pizza"));
        NextWords again(QStringList{});
        again.load(nw.save().split('\n'));
        EQ(again.predict("want"), QStringList({ "pizza" }));
    }
    {
        g_test = "saysWhatItCorrectedAndWhere"; ComposerFixture t(c);
        t.composer.onCorrected = [&t](const QString &, const QString &w, int tail) { t.seen << QPair<QString, int>(w, tail); };
        t.type("teh ");
        EQ(t.seen.size(), 1); EQ(t.seen.value(0), (QPair<QString, int>("the", 1)));
        t.type("cta.");
        EQ(t.field.text, QString("the cat. "));
        EQ(t.seen.last(), (QPair<QString, int>("cat", 2)));
        t.type("dont");
        t.composer.swipe("know", t.on);
        EQ(t.field.text, QString("the cat. don't know "));
        EQ(t.seen.last(), (QPair<QString, int>("don't", 6)));
        t.type("thsi");
        t.composer.beforeEnter(t.on);
        EQ(t.seen.last(), (QPair<QString, int>("this", 0)));
        t.type(" fine ");
        EQ(t.seen.size(), 4);
    }
}

// --- Corrector ----------------------------------------------------------------------------


static void correctorTests(const QSharedPointer<Corrector> &c)
{
    auto fixes = [&](const QString &typed, const QString &want) {
        const QString got = c->correct(typed);
        if (got != want) { ++g_failed; fprintf(stderr, "FAIL [%s] correct(%s) = %s, want %s\n", qPrintable(g_test),
                                                qPrintable(typed), qPrintable(show(got)), qPrintable(want)); }
        else ++g_passed;
    };
    auto keeps = [&](const QString &typed) { fixes(typed, QString()); };

    g_test = "contractions";
    fixes("dont", "don't"); fixes("cant", "can't"); fixes("wont", "won't"); fixes("im", "I'm");
    fixes("Dont", "Don't"); fixes("youre", "you're"); fixes("didnt", "didn't"); fixes("thats", "that's");
    for (const char *w : { "well", "were", "its", "ill", "hell", "shell", "lets", "wed", "shed" }) keeps(w);

    g_test = "contractionsThatAreAlsoWordsAreOfferedNotForced";
    const QList<QPair<QString, QString>> amb = { { "ill", "I'll" }, { "its", "it's" }, { "well", "we'll" },
        { "were", "we're" }, { "lets", "let's" }, { "wed", "we'd" }, { "hell", "he'll" }, { "shell", "she'll" },
        { "shed", "she'd" } };
    for (const auto &p : amb) {
        keeps(p.first);
        EQ(c->suggestions(p.first).mid(0, 2), QStringList({ p.first, p.second }));
        EQ(c->alternatives(p.first).value(0), p.second);
    }
    EQ(c->suggestions("Its").mid(0, 2), QStringList({ "Its", "It's" }));
    EQ(c->correct("ill", "think"), QString("I'll")); EQ(c->correct("Ill", NextWords::START), QString("I'll"));
    EQ(c->correct("its", "that"), QString("it's")); EQ(c->correct("were", "think"), QString("we're"));
    EQ(c->suggestions("ill", "think").mid(0, 2), QStringList({ "I'll", "ill" }));
    EQ(c->correct("its", "of"), QString()); EQ(c->correct("ill", "was"), QString());
    EQ(c->correct("Well", NextWords::START), QString()); EQ(c->correct("Were", NextWords::START), QString());
    EQ(c->correct("ill", "zebra"), QString());
    fixes("hed", "he'd"); fixes("wholl", "who'll");

    g_test = "capitals";
    fixes("i", "I"); keeps("I"); keeps("NASA"); keeps("OK");

    g_test = "typos";
    fixes("teh", "the"); fixes("hte", "the"); fixes("recieve", "receive"); fixes("adress", "address");
    fixes("becuase", "because"); fixes("tomorow", "tomorrow"); fixes("thsi", "this");
    fixes("Teh", "The"); fixes("keybaord", "keyboard"); fixes("helo", "hello");

    g_test = "runTogetherWords";
    fixes("theyare", "they are"); fixes("theynare", "they are"); fixes("inthe", "in the");
    fixes("iwent", "I went"); fixes("thebdog", "the dog"); fixes("sonmuch", "so much");
    fixes("Ifnyou", "If you"); fixes("alot", "a lot");
    fixes("nownin", "now in"); fixes("onnit", "on it"); fixes("wasnit", "was it");
    fixes("asnit", "as it"); fixes("sonit", "so it"); fixes("tobme", "to me");
    fixes("basrball", "baseball"); fixes("toon", "too"); fixes("abvout", "about");
    for (const char *w : { "welcome", "ashore", "nowhere", "together", "inbox", "notice" }) keeps(w);

    g_test = "knownWordsAndOddities";
    for (const char *w : { "hello", "the", "keyboard", "a", "ten", "receive", "don't" }) keeps(w);
    keeps("x2"); keeps("http"); keeps("qzxj");

    g_test = "learnedWordsStay";
    {
        QSharedPointer<Corrector> local = corrector(Language::enUS());
        EQ(local->correct("teh"), QString("the"));
        local->learn("teh");
        EQ(local->correct("teh"), QString());
    }

    g_test = "suggestions";
    EQ(c->suggestions("teh").value(0), QString("the"));
    EQ(c->suggestions("hello").value(0), QString("hello"));
    CHECK(c->suggestions("keyboa").contains("keyboard"));
    EQ(c->suggestions("Hello").value(0), QString("Hello"));
    EQ(c->suggestions("th").size(), 3);
    EQ(c->suggestions(""), QStringList());

    g_test = "alternativesForATappedWord";
    const QStringList ball = c->alternatives("ball");
    CHECK(ball.contains("bell"));
    CHECK(!ball.contains("ball"));
    EQ(ball.size(), 3);
    bool allCaps = true;
    for (const QString &w : c->alternatives("Ball")) if (!w[0].isUpper()) allCaps = false;
    CHECK(allCaps);
    EQ(c->alternatives("dont").value(0), QString("don't"));
}

// --- Languages and the keymap -------------------------------------------------------------

static void languageTests()
{
    QHash<QString, QSharedPointer<Corrector>> c;
    QList<QSharedPointer<NextWords>> keep;
    for (const Language &lang : Language::all()) {
        QSharedPointer<Corrector> corr = corrector(lang);
        QSharedPointer<NextWords> nw(new NextWords(lines(lang.asset(Language::NEXT_WORDS))));
        keep << nw;
        NextWords *n = nw.data();
        corr->follows = [n](const QString &a, const QString &b) { return n->follows(a, b); };
        c.insert(lang.id, corr);
    }
    auto fixes = [&](const QString &lang, const QString &typed, const QString &want) {
        const QString got = c[lang]->correct(typed);
        if (got != want) { ++g_failed; fprintf(stderr, "FAIL [%s] %s correct(%s) = %s, want %s\n", qPrintable(g_test),
                                                qPrintable(lang), qPrintable(typed), qPrintable(show(got)), qPrintable(show(want))); }
        else ++g_passed;
    };
    auto keeps = [&](const QString &lang, const QString &typed) { fixes(lang, typed, QString()); };

    g_test = "everyLanguageHasItsFiles";
    for (const Language &lang : Language::all())
        for (const QString &name : { Language::DICTIONARY, Language::NEXT_WORDS, Language::EMOJI_WORDS })
            CHECK(QFile::exists(g_data + '/' + lang.asset(name)));
    for (const auto &corr : c) CHECK(corr->size() > 50000);
    CHECK(c["en_GB"]->isKnown("colour")); CHECK(!c["en_GB"]->isKnown("color"));

    g_test = "regionalEnglishSpells";
    keeps("en_GB", "colour"); keeps("en_GB", "organise"); fixes("en_GB", "colur", "colour");
    keeps("en_CA", "colour"); keeps("en_CA", "organize");
    keeps("en_AU", "colour"); keeps("en_US", "color");
    fixes("en_GB", "dont", "don't");

    g_test = "accentsGoIn";
    fixes("fr", "etre", "être"); fixes("fr", "deja", "déjà"); fixes("fr", "francais", "français");
    fixes("fr", "coeur", "cœur"); fixes("fr", "Etre", "Être");
    fixes("de", "fur", "für"); fixes("de", "strasse", "Straße"); fixes("de", "konnen", "können"); fixes("de", "uber", "über");
    fixes("es", "espanol", "español"); fixes("es", "manana", "mañana"); fixes("es", "tambien", "también");
    fixes("it", "perche", "perché"); fixes("it", "citta", "città"); fixes("it", "piu", "più");

    g_test = "wordsThatDifferOnlyByAccentAreBothKept";
    keeps("fr", "a"); keeps("fr", "à"); keeps("fr", "ou"); keeps("fr", "où");
    keeps("es", "si"); keeps("es", "sí"); keeps("es", "el"); keeps("es", "él");
    keeps("it", "e"); keeps("it", "è"); keeps("it", "si"); keeps("it", "sì");

    g_test = "germanNounsGetTheirCapital";
    fixes("de", "haus", "Haus"); fixes("de", "zeit", "Zeit");
    keeps("de", "sie"); keeps("de", "Sie");

    g_test = "elisions";
    fixes("fr", "cest", "c'est"); fixes("fr", "jai", "j'ai"); fixes("fr", "lhomme", "l'homme");
    fixes("fr", "l'hommme", "l'homme");
    keeps("fr", "l'homme"); keeps("fr", "qu'il");
    CHECK(c["fr"]->isKnown("l'homme"));
    EQ(c["fr"]->spelling("l'homme"), QString("homme"));
    fixes("it", "lanno", "l'anno"); keeps("it", "dell'anno");
    bool allElided = true;
    for (const QString &s : c["fr"]->suggestions("l'hom")) if (!s.startsWith("l'")) allElided = false;
    CHECK(allElided);

    g_test = "typosNearTheirKeys";
    fixes("fr", "qvec", "avec");
    fixes("de", "nicjt", "nicht");
    fixes("es", "qie", "que"); fixes("it", "cpme", "come");
    CHECK(c["fr"]->adjacent('a', 'z')); CHECK(c["fr"]->adjacent('q', 's'));
    CHECK(c["de"]->adjacent('z', 't')); CHECK(c["de"]->adjacent('y', 's'));
    CHECK(!c["en_US"]->adjacent('z', 't'));

    g_test = "noEnglishContractionsElsewhere";
    keeps("fr", "dont");

    g_test = "layoutsHaveEveryLetterOnce";
    for (const Language &lang : Language::all()) {
        QStringList letters;
        for (const auto &row : Keymap::letters(lang))
            for (const Slot &s : row)
                if (s.drawn && s.key->kind == Kind::Char && s.key->text.length() == 1 &&
                    s.key->text[0] >= 'a' && s.key->text[0] <= 'z')
                    letters << s.key->text;
        letters.sort();
        EQ(letters.join(""), QString("abcdefghijklmnopqrstuvwxyz"));
    }
    auto topRow = [](const QString &id) {
        QString s; for (const Slot &slot : Keymap::letters(Language::byId(id))[0]) s += slot.key->text; return s;
    };
    EQ(topRow("fr"), QString("azertyuiop"));
    EQ(topRow("de"), QString("qwertzuiop"));

    g_test = "alternatesStayInPlace";
    const Rows &us = Keymap::letters(Language::enUS());
    for (const Language &lang : Language::all()) {
        const Rows &rows = Keymap::letters(lang);
        for (int r = 0; r < rows.size(); ++r) {
            QStringList a, b;
            for (const Slot &s : us[r]) a << s.key->alt;
            for (const Slot &s : rows[r]) b << s.key->alt;
            EQ(b, a);
        }
    }

    g_test = "ownAccentsLeadTheHolds";
    auto hold = [](const QString &lang, const QString &letter) {
        for (const auto &row : Keymap::letters(Language::byId(lang)))
            for (const Slot &s : row)
                if (s.key->text == letter) return s.key->extended;
        return QStringList();
    };
    EQ(hold("fr", "e").mid(0, 5), QStringList({ "e", "é", "è", "ê", "ë" }));
    EQ(hold("de", "s").mid(0, 2), QStringList({ "s", "ß" }));
    EQ(hold("es", "n").mid(0, 2), QStringList({ "n", "ñ" }));
    EQ(hold("it", "e").mid(0, 3), QStringList({ "e", "è", "é" }));
    EQ(hold("en_US", "e").mid(0, 3), QStringList({ "e", "è", "é" }));

    g_test = "symPageRepeatsNoLanguagesLetters";
    QStringList sym;
    for (const auto &row : Keymap::symbols())
        for (const Slot &s : row)
            if (s.drawn && s.key->kind == Kind::Char) { sym << s.key->text; sym += s.key->extended; }
    for (const Language &lang : Language::all()) {
        QSet<QString> letters;
        for (const auto &row : Keymap::letters(lang))
            for (const Slot &s : row)
                if (s.drawn && s.key->kind == Kind::Char) {
                    letters.insert(s.key->text);
                    if (!s.key->alt.isEmpty()) letters.insert(s.key->alt);
                    for (const QString &e : s.key->extended) letters.insert(e);
                }
        QStringList repeated;
        for (const QString &x : sym)
            if (letters.contains(x) && x != "." && !(x.length() == 1 && x[0].isDigit())) repeated << x;
        EQ(repeated, QStringList());
    }

    g_test = "symPageHasTheAsciiSymbolsTheLettersPageLacks";
    for (QChar ch : QString("^~`|\\<>[]{}")) CHECK(sym.contains(QString(ch)));

    g_test = "everyHoldListStartsWithItsKey";
    QList<Rows> pages = { Keymap::letters(Language::enUS()), Keymap::symbols() };
    for (const Rows &page : pages)
        for (const auto &row : page)
            for (const Slot &s : row)
                if (s.key->kind == Kind::Char && !s.key->extended.isEmpty()) EQ(s.key->extended.first(), s.key->text);

    g_test = "swipesSpellAccentedWordsWithPlainLetters";
    EQ(Language::fold("être"), QString("etre"));
    EQ(Language::fold("straße"), QString("strasse"));
    EQ(Language::fold("cœur"), QString("coeur"));
    EQ(Language::fold("École"), QString("Ecole"));
}

// --- Emoji for words ----------------------------------------------------------------------

static void emojiTests()
{
    g_test = "commonWordsGetTheUsualEmoji";
    EmojiWords w(lines("emoji_words.txt"));
    EQ(w.forWord("pizza"), QString("🍕"));
    EQ(w.forWord("Love"), QString("❤️"));
    EQ(w.forWord("tacos"), QString("🌮"));
    EQ(w.forWord("pizza's"), QString("🍕"));
    EQ(w.forWord("France"), QString("🇫🇷"));
    EQ(w.forWord("the"), QString());
    EQ(w.forWord(""), QString());

    g_test = "onlyEmojiTheDeviceCanDraw";
    EQ(EmojiWords(lines("emoji_words.txt"), [](const QString &e) { return e != "🐱"; }).forWord("cat"), QString("🐈"));
    EQ(EmojiWords(lines("emoji_words.txt"), [](const QString &) { return false; }).forWord("pizza"), QString());
}

// --- Swipe --------------------------------------------------------------------------------

static const float W = 100, H = 130;

static SwipeDecoder::Layout swipeLayout()
{
    SwipeDecoder::Layout l;
    auto row = [&](const char *s, float col0, int r) {
        for (int i = 0; s[i]; ++i) { l.x[s[i] - 'a'] = (col0 + i + 0.5f) * W; l.y[s[i] - 'a'] = (r + 0.5f) * H; }
    };
    row("qwertyuiop", 0, 0); row("asdfghjkl", 0, 1); row("zxcvbnm", 1, 2);
    l.keyWidth = W; l.keyHeight = H;
    return l;
}

//! A path through [word]'s keys, each corner off by up to [wobble] keys, sampled every 8 px.
static QPair<QVector<float>, QVector<float>> swipePath(const QString &word, float wobble, std::mt19937 &rnd, float smooth = 0)
{
    const SwipeDecoder::Layout l = swipeLayout();
    std::normal_distribution<float> gauss(0, 1);
    QVector<float> cx, cy;
    for (QChar ch : word.toLower()) {
        if (ch < 'a' || ch > 'z') continue;
        cx << l.x[ch.unicode() - 'a'] + gauss(rnd) * wobble * W;
        cy << l.y[ch.unicode() - 'a'] + gauss(rnd) * wobble * H;
    }
    QVector<float> xs, ys;
    xs << cx[0]; ys << cy[0];
    for (int i = 1; i < cx.size(); ++i) {
        const float dx = cx[i] - cx[i - 1], dy = cy[i] - cy[i - 1];
        const int steps = qMax(1, int(std::hypot(dx, dy) / 8));
        for (int s = 1; s <= steps; ++s) { xs << cx[i - 1] + dx * s / steps; ys << cy[i - 1] + dy * s / steps; }
    }
    const int k = int(smooth / 8);
    auto avg = [k](const QVector<float> &v) {
        QVector<float> out(v.size());
        for (int i = 0; i < v.size(); ++i) {
            const int lo = qMax(0, i - k), hi = qMin(v.size() - 1, i + k);
            float t = 0; for (int j = lo; j <= hi; ++j) t += v[j];
            out[i] = t / (hi - lo + 1);
        }
        return out;
    };
    return { avg(xs), avg(ys) };
}

static void swipeTests(const QSharedPointer<Corrector> &c)
{
    SwipeDecoder d(c->entries());
    const SwipeDecoder::Layout layout = swipeLayout();
    auto read = [&](const QString &word, float wobble = 0, unsigned seed = 1, float smooth = 0) {
        std::mt19937 rnd(seed);
        const auto p = swipePath(word, wobble, rnd, smooth);
        return d.decode(p.first, p.second, layout);
    };
    const QStringList words = QString("the and hello keyboard orange palm great this working quick brown jumps over lazy "
        "swipe typing android phone thanks what where people little about would could because something together "
        "tomorrow morning coffee please today time really think know want going good love white black music family "
        "friend message later").split(' ');

    g_test = "cleanSwipesReadExactly";
    QStringList wrong;
    for (const QString &w : words) { const QStringList got = read(w); if (got.value(0) != w) wrong << w + " -> " + show(got); }
    EQ(wrong, QStringList());

    g_test = "shakyHandsMostlyRight";
    int right = 0, total = 0;
    QStringList misses;
    for (unsigned seed = 1; seed <= 5; ++seed)
        for (const QString &w : words) {
            const QStringList got = read(w, 0.22f, seed, 50);
            total++;
            if (got.value(0) == w) right++;
            else if (!got.contains(w)) misses << w + " -> " + show(got);
        }
    printf("swipe, shaky: %d / %d first; not offered at all: %s\n", right, total, qPrintable(show(misses)));
    CHECK(right >= total * 0.85);
    CHECK(misses.size() <= total * 0.05);

    g_test = "doubleLettersAndShortWords";
    EQ(read("helo").value(0), QString("hello"));
    EQ(read("in").value(0), QString("in"));
    CHECK(read("on").contains("on"));

    g_test = "aTapIsNotASwipe";
    EQ(d.decode({ 50, 55 }, { 65, 70 }, layout), QStringList());

    g_test = "learnedWordsCanBeSwiped";
    CHECK(!read("luneos").contains("luneos"));
    d.add("LuneOS", 10);
    EQ(read("luneos").value(0), QString("LuneOS"));

    g_test = "quickEnough";
    std::mt19937 rnd(3);
    const auto p = swipePath("something", 0.2f, rnd);
    d.decode(p.first, p.second, layout);
    QElapsedTimer t; t.start();
    for (int i = 0; i < 20; ++i) d.decode(p.first, p.second, layout);
    printf("swipe decode: %.1f ms\n", t.nsecsElapsed() / 20 / 1e6);
}

//! Its own dictionary, since it deletes words from it.
static void removedTests()
{
    g_test = "aDeletedWordIsCorrectedAwayFromAndCanBePutBack";
    QSharedPointer<Corrector> d = corrector(Language::enUS());
    EQ(d->isKnown("om"), true);
    EQ(d->correct("om"), QString());
    d->learn("yest");
    EQ(d->suggestions("yest").value(0), QString("yest"));
    d->remove("om"); d->remove("yest");
    EQ(d->isKnown("om"), false);
    EQ(d->isKnown("yest"), false);
    CHECK(!d->correct("om").isEmpty());
    CHECK(d->suggestions("om").contains("on"));
    CHECK(d->suggestions("yest").contains("test") || d->suggestions("yest").contains("yes"));
    bool listed = false;
    for (const auto &e : d->entries()) if (e.first == QLatin1String("om")) listed = true;
    EQ(listed, false);
    d->restore("om");
    EQ(d->isKnown("om"), true);

    g_test = "aDeletedWordIsNoLongerSwiped";
    SwipeDecoder s(d->entries());
    const SwipeDecoder::Layout layout = swipeLayout();
    std::mt19937 rnd(1);
    const auto p = swipePath("hello", 0, rnd, 0);
    EQ(s.decode(p.first, p.second, layout).value(0), QString("hello"));
    s.forget("hello");
    CHECK(!s.decode(p.first, p.second, layout).contains("hello"));
}

//! KLIPY's answers read (no network: only what came back).
static void klipyTests()
{
    const QByteArray json = R"({"result":true,"data":{"data":[
      {"id":8041071659142944,"slug":"hello-hi-662","title":"Hello","type":"gif",
       "blur_preview":"data:image/jpeg;base64,AAAA",
       "file":{
         "hd":{"gif":{"url":"https://static.klipy.com/hd.gif","width":498,"height":498,"size":4001918}},
         "md":{"gif":{"url":"https://static.klipy.com/md.gif","width":320,"height":320,"size":2001918},
               "webp":{"url":"https://static.klipy.com/md.webp","width":320,"height":320,"size":200000}},
         "sm":{"gif":{"url":"https://static.klipy.com/sm.gif","width":220,"height":110,"size":300000},
               "jpg":{"url":"https://static.klipy.com/sm.jpg","width":220,"height":110,"size":7000}}}},
      {"type":"ad","content":"<div></div>"},
      {"id":2,"slug":"grrr","title":"Grrr Sticker","type":"sticker",
       "file":{"md":{"png":{"url":"https://static.klipy.com/g.png","width":189,"height":200,"size":3178},
                     "webp":{"url":"https://static.klipy.com/g.webp","width":189,"height":200,"size":16260}}}}
    ],"current_page":1,"per_page":24,"has_next":true}})";

    g_test = "readsAPage";
    bool ok = false;
    QString refusal;
    const Klipy::Page p = Klipy::parse(json, &ok, &refusal);
    EQ(ok, true);
    EQ(int(p.items.size()), 2);                  // the ad, with no file, is left out
    EQ(p.page, 1);
    EQ(p.hasNext, true);
    const Klipy::Item &gif = p.items.value(0);
    EQ(gif.id, QString("8041071659142944"));
    EQ(gif.slug, QString("hello-hi-662"));
    const Klipy::File *jpg = gif.first("jpg", { "md", "sm" });
    EQ(jpg ? jpg->url : QString(), QString("https://static.klipy.com/sm.jpg"));
    CHECK(qFuzzyCompare(gif.aspect(), 1.0));
    EQ(gif.blur, QString("data:image/jpeg;base64,AAAA"));
    // LuneOS's applications take a GIF as its link: the medium GIF.
    EQ(Klipy::link(gif), QString("https://static.klipy.com/md.gif"));
    // A sticker with no GIF: its WebP.
    EQ(Klipy::link(p.items.value(1)), QString("https://static.klipy.com/g.webp"));

    g_test = "refusesAnError";
    Klipy::parse(R"({"result":false,"message":"bad key"})", &ok, &refusal);
    EQ(ok, false);
    EQ(refusal, QString("bad key"));
    Klipy::parse(R"({"result":false,"errors":{"message":["The provided API key is invalid."]}})", &ok, &refusal);
    EQ(refusal, QString("The provided API key is invalid."));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    g_data = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(PRE_DATA_DIR);
    QElapsedTimer t; t.start();
    QSharedPointer<Corrector> c = corrector(Language::enUS());
    QSharedPointer<NextWords> nw(new NextWords(lines("next_words.txt")));
    c->follows = [nw](const QString &a, const QString &b) { return nw->follows(a, b); };
    c->loadContractions(lines("contractions.txt"));
    printf("dictionary: %d words in %lld ms\n", c->size(), t.elapsed());
    composerTests(c);
    correctorTests(c);
    languageTests();
    emojiTests();
    swipeTests(c);
    removedTests();
    klipyTests();
    printf("%d passed, %d failed\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
