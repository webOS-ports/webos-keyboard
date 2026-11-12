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

#include "correction.h"

#include <QHash>
#include <QSet>
#include <QtTest>

using MaliitKeyboard::Logic::Correction;

namespace {

/*!
 * A dictionary as hunspell is to the word engine: the words it knows, and for
 * a word it doesn't, its suggestions in its order. The suggestions here are
 * hunspell's own for these words (en_AU, de, fr, es, it as LuneOS ships them).
 */
struct FakeDictionary
{
    QSet<QString> words;
    QHash<QString, QStringList> suggestions;

    Correction::Dictionary dictionary() const
    {
        Correction::Dictionary d;
        d.spell = [this](const QString &w) { return words.contains(w); };
        d.suggest = [this](const QString &w) { return suggestions.value(w); };
        return d;
    }
};

} // namespace

class TestCorrection : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void data_fold()
    {
        QCOMPARE(Correction::fold(QStringLiteral("Don't")), QStringLiteral("dont"));
        QCOMPARE(Correction::fold(QStringLiteral("c\u2019est")), QStringLiteral("cest"));
        QCOMPARE(Correction::fold(QStringLiteral("peut-être")), QStringLiteral("peutetre"));
        QCOMPARE(Correction::fold(QStringLiteral("of the")), QStringLiteral("ofthe"));
        QCOMPARE(Correction::fold(QStringLiteral("Straße")), QStringLiteral("strasse"));
        QCOMPARE(Correction::fold(QStringLiteral("niño")), QStringLiteral("nino"));
        QCOMPARE(Correction::casedLike(QStringLiteral("Dont"), QStringLiteral("don't")), QStringLiteral("Don't"));
        QCOMPARE(Correction::casedLike(QStringLiteral("dont"), QStringLiteral("don't")), QStringLiteral("don't"));
        QCOMPARE(Correction::casedLike(QStringLiteral("Haus"), QStringLiteral("Haus")), QStringLiteral("Haus"));
    }

    void choose_data()
    {
        QTest::addColumn<QString>("language");
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QStringList>("known");
        QTest::addColumn<QStringList>("suggested");
        QTest::addColumn<QStringList>("predicted");
        QTest::addColumn<QString>("expected");

        const QStringList none;
        // English contractions and I, whatever the dictionary says.
        QTest::newRow("dont") << "en" << "dont" << none << QStringList{ "font", "dint", "don", "dot" } << none << "don't";
        QTest::newRow("Dont") << "en" << "Dont" << none << QStringList{ "Font" } << none << "Don't";
        QTest::newRow("im") << "en" << "im" << none << QStringList{ "mi", "um" } << QStringList{ "impossible" } << "I'm";
        QTest::newRow("cant known") << "en" << "cant" << QStringList{ "cant" } << none << none << "can't";
        QTest::newRow("i") << "en" << "i" << QStringList{ "i" } << none << QStringList{ "it" } << "I";
        QTest::newRow("i'll") << "en_GB" << "i'll" << none << none << none << "I'll";
        QTest::newRow("dont in German") << "de" << "dont" << QStringList{ "dont" } << none << none << "dont";
        // Known: kept, not completed.
        QTest::newRow("merci") << "fr" << "merci" << QStringList{ "merci" } << none << QStringList{ "mercier", "merci" } << "merci";
        QTest::newRow("hola") << "es" << "hola" << QStringList{ "hola" } << none << QStringList{ "holanda" } << "hola";
        // Capitals only: a German noun.
        QTest::newRow("haus") << "de" << "haus" << none << QStringList{ "Haus", "hau", "aus", "hause" } << QStringList{ "hause", "haus" } << "Haus";
        QTest::newRow("auto") << "de" << "auto" << none << QStringList{ "Auto", "auto-", "-auto" } << QStringList{ "autorität" } << "Auto";
        QTest::newRow("CA is no ça") << "fr" << "ca" << QStringList{ "ça" } << QStringList{ "CA", "qua" } << QStringList{ "cardinal" } << "ça";
        // Accents and apostrophes the checker has.
        QTest::newRow("ecole") << "fr" << "ecole" << none << QStringList{ "école" } << none << "école";
        QTest::newRow("Ecole") << "fr" << "Ecole" << none << QStringList{ "école" } << none << "École";
        QTest::newRow("tres") << "fr" << "tres" << none << QStringList{ "très", "tees" } << QStringList{ "tressaillit" } << "très";
        QTest::newRow("didnt") << "en" << "didnt" << none << QStringList{ "dint", "didn't" } << none << "didn't";
        QTest::newRow("cest") << "fr" << "cest" << none << QStringList{ "c\u2019est" } << none << "c\u2019est";
        QTest::newRow("cancion") << "es" << "cancion" << none << QStringList{ "canción" } << QStringList{ "canciones" } << "canción";
        QTest::newRow("mussen") << "de" << "mussen" << none << QStringList{ "missen", "mussten", "messen", "müssen" } << none << "müssen";
        QTest::newRow("gross") << "de" << "gross" << none << QStringList{ "groß" } << none << "groß";
        QTest::newRow("peut-etre") << "fr" << "peut-etre" << none << QStringList{ "peut-être" } << none << "peut-être";
        // An accent the checker missed.
        QTest::newRow("fur") << "de" << "fur" << QStringList{ "für" } << QStringList{ "ruf", "fuhr" } << QStringList{ "furcht" } << "für";
        QTest::newRow("nino") << "es" << "nino" << QStringList{ "niño" } << QStringList{ "bino", "mino" } << none << "niño";
        QTest::newRow("strasse") << "de" << "strasse" << QStringList{ "Straße" } << QStringList{ "stresse" } << none << "Straße";
        QTest::newRow("no accents in English") << "en" << "cafx" << QStringList{ "cafè" } << QStringList{ "calf" } << none << "calf";
        // What presage expects, when the checker has it too.
        QTest::newRow("beautif") << "en" << "beautif" << none << QStringList{ "beautify", "beaut if", "beautiful" } << QStringList{ "beautiful" } << "beautiful";
        QTest::newRow("hund") << "de" << "hund" << QStringList{ "Hund" } << QStringList{ "Hund", "und" } << QStringList{ "hundert", "hund" } << "Hund";
        // Swapped letters, and words run together: whichever the checker puts first.
        QTest::newRow("taht") << "en" << "taht" << none << QStringList{ "that", "tat", "ta ht" } << none << "that";
        QTest::newRow("alot") << "en" << "alot" << none << QStringList{ "a lot", "alto" } << none << "a lot";
        QTest::newRow("tothe") << "en" << "tothe" << none << QStringList{ "tithe", "tote", "to the" } << none << "to the";
        QTest::newRow("diff rent") << "en" << "diffrent" << none << QStringList{ "diff rent", "different" } << none << "different";
        QTest::newRow("he lo") << "en" << "helo" << none << QStringList{ "he lo", "hole" } << none << "hole";
        QTest::newRow("ofthe") << "en" << "Ofthe" << none << QStringList{ "of the" } << none << "Of the";
        QTest::newRow("swap first") << "en" << "teh" << none << QStringList{ "eh", "the" } << none << "the";
        QTest::newRow("no swap: the rest differs") << "en" << "abcd" << none << QStringList{ "zzzz", "bacx" } << none << "zzzz";
        QTest::newRow("no split: other letters") << "en" << "tothe" << none << QStringList{ "to them", "tithe" } << none << "tithe";
        QTest::newRow("no part word, hyphen typed") << "fr" << "peut-etrz" << none << QStringList{ "peut-", "peut-être" } << none << "peut-être";
        QTest::newRow("no hyphen not typed") << "en" << "xauto" << none << QStringList{ "x-auto", "Xanto" } << none << "Xanto";
        QTest::newRow("swap not at the end") << "en" << "tah" << none << QStringList{ "hat", "tea" } << none << "hat";
        // Elisions, French and Italian.
        QTest::newRow("lecole") << "fr" << "lecole" << QStringList{ "l\u2019école" } << QStringList{ "recolle" } << none << "l\u2019école";
        QTest::newRow("quil") << "fr" << "quil" << QStringList{ "qu\u2019il", "il" } << QStringList{ "cil", "kil" } << none << "qu\u2019il";
        QTest::newRow("Lhomme") << "fr" << "Lhomme" << QStringList{ "L\u2019homme", "homme" } << none << none << "L\u2019homme";
        QTest::newRow("lacqua") << "it" << "lacqua" << QStringList{ "l'acqua", "acqua" } << QStringList{ "acqua" } << none << "l'acqua";
        QTest::newRow("no elision before a consonant") << "fr" << "lmaison" << QStringList{ "maison", "l\u2019maison" } << QStringList{ "maison" } << none << "maison";
        QTest::newRow("no elisions in Spanish") << "es" << "lagua" << QStringList{ "agua", "l\u2019agua" } << QStringList{ "lagos" } << none << "lagos";
        // The rest: the checker's first, else presage's.
        QTest::newRow("teh") << "en" << "teh" << none << QStringList{ "the", "eh" } << none << "the";
        QTest::newRow("only presage") << "en" << "xyzzq" << none << none << QStringList{ "xyzzqa" } << "xyzzqa";
        QTest::newRow("nothing") << "en" << "xyzzq" << none << none << none << "xyzzq";
        QTest::newRow("a part word") << "de" << "xauto" << none << QStringList{ "-auto", "Xanto" } << none << "Xanto";
    }

    void choose()
    {
        QFETCH(QString, language);
        QFETCH(QString, typed);
        QFETCH(QStringList, known);
        QFETCH(QStringList, suggested);
        QFETCH(QStringList, predicted);
        QFETCH(QString, expected);
        FakeDictionary fake;
        fake.words = QSet<QString>(known.begin(), known.end());
        fake.suggestions.insert(typed, suggested);
        // An elision's word, looked up on its own.
        fake.suggestions.insert(QStringLiteral("ecole"), { QStringLiteral("école") });
        QCOMPARE(Correction::choose(typed, language, fake.dictionary(), predicted), expected);
    }

    void withoutADictionary()
    {
        QCOMPARE(Correction::choose(QStringLiteral("teh"), QStringLiteral("en"), Correction::Dictionary(), {}),
                 QStringLiteral("teh"));
        QCOMPARE(Correction::choose(QString(), QStringLiteral("en"), FakeDictionary().dictionary(), {}), QString());
    }
};

QTEST_GUILESS_MAIN(TestCorrection)
#include "ut_correction.moc"
