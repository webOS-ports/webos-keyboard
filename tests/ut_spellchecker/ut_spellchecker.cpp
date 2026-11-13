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

// Taking a word out of the owner's dictionary, which lives in db8, where Text
// Assist lists it. A deleted word is blocked without regard to case, but the
// delete matched the word exactly: holding "webos" in the bar left the
// "webOS" Text Assist had saved where it was, in the list and in the checker.
//
// luna-service2 is a stand-in (stubs/): each call is recorded here, and a db8
// find is answered at once with the words the test saved.

#include "spellchecker.h"
#include "userwords.h"

#include <QtTest>

namespace {

struct Call
{
    QString uri;
    QJsonObject payload;
};

QList<Call> calls;
QStringList saved;

} // namespace

struct LSHandle {};
struct LSMessage { QByteArray payload; };

bool LSErrorInit(LSError *error)
{
    error->error_code = 0;
    error->message = nullptr;
    return true;
}

void LSErrorFree(LSError *) {}

bool LSRegister(const char *, LSHandle **sh, LSError *)
{
    static LSHandle handle;
    *sh = &handle;
    return true;
}

bool LSUnregister(LSHandle *, LSError *) { return true; }
bool LSGmainAttach(LSHandle *, GMainLoop *, LSError *) { return true; }

bool LSCall(LSHandle *sh, const char *uri, const char *payload, LSFilterFunc callback, void *ctx,
            LSMessageToken *, LSError *)
{
    const QString u = QString::fromUtf8(uri);
    calls << Call{ u, QJsonDocument::fromJson(QByteArray(payload)).object() };
    if (u == QLatin1String("luna://com.palm.db/find")) {
        QJsonArray results;
        for (const QString &w : saved)
            results << QJsonObject{ { QStringLiteral("word"), w } };
        LSMessage reply{ QJsonDocument(QJsonObject{ { QStringLiteral("returnValue"), true },
                                                    { QStringLiteral("results"), results } })
                             .toJson(QJsonDocument::Compact) };
        callback(sh, &reply, ctx);
    }
    return true;
}

const char *LSMessageGetPayload(LSMessage *message)
{
    return message->payload.constData();
}

class TestSpellChecker : public QObject
{
    Q_OBJECT

    static QStringList deletedWords()
    {
        QStringList out;
        for (const Call &c : calls) {
            if (c.uri != QLatin1String("luna://com.palm.db/del"))
                continue;
            const QJsonObject query = c.payload.value(QStringLiteral("query")).toObject();
            for (const QJsonValue where : query.value(QStringLiteral("where")).toArray()) {
                const QJsonObject w = where.toObject();
                if (w.value(QStringLiteral("prop")).toString() == QLatin1String("word")
                    && w.value(QStringLiteral("op")).toString() == QLatin1String("="))
                    out << w.value(QStringLiteral("val")).toString();
            }
        }
        return out;
    }

private Q_SLOTS:
    void init()
    {
        calls.clear();
        saved.clear();
    }

    // ---- savedSpellings ----

    void savedSpellings_data()
    {
        QTest::addColumn<QStringList>("stored");
        QTest::addColumn<QString>("word");
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("as saved") << QStringList{ "webOS" } << "webOS" << QStringList{ "webOS" };
        QTest::newRow("other case") << QStringList{ "webOS" } << "webos" << QStringList{ "webOS" };
        QTest::newRow("all caps") << QStringList{ "webOS" } << "WEBOS" << QStringList{ "webOS" };
        QTest::newRow("two spellings") << QStringList{ "webOS", "the", "WebOS" } << "webos"
                                       << QStringList{ "webOS", "WebOS" };
        QTest::newRow("twice") << QStringList{ "webOS", "webOS" } << "webos" << QStringList{ "webOS" };
        QTest::newRow("not there") << QStringList{ "webOS" } << "webs" << QStringList{};
        QTest::newRow("a part of one") << QStringList{ "webOS" } << "web" << QStringList{};
        QTest::newRow("accent") << QStringList{ "Café" } << "CAFÉ" << QStringList{ "Café" };
        QTest::newRow("empty") << QStringList{ "webOS" } << "" << QStringList{};
    }

    void savedSpellings()
    {
        QFETCH(QStringList, stored);
        QFETCH(QString, word);
        QFETCH(QStringList, expected);
        QCOMPARE(MaliitKeyboard::Logic::savedSpellings(stored, word), expected);
    }

    // ---- removeFromUserWordlist ----

    void removeInAnotherCase()
    {
        saved = QStringList{ QStringLiteral("webOS"), QStringLiteral("LuneOS") };
        SpellChecker checker;
        QVERIFY(checker.setLanguage(QStringLiteral("en_US")));
        QVERIFY(checker.setEnabled(true));
        QCOMPARE(checker.userWords(), saved);
        QVERIFY(checker.spell(QStringLiteral("webOS")));

        checker.removeFromUserWordlist(QStringLiteral("webos"));

        QCOMPARE(checker.userWords(), QStringList{ QStringLiteral("LuneOS") });
        QVERIFY(!checker.spell(QStringLiteral("webOS")));
        QVERIFY(checker.spell(QStringLiteral("LuneOS")));
        // db8 matches the spelling exactly: deleted as it was saved.
        QCOMPARE(deletedWords(), QStringList{ QStringLiteral("webOS") });
    }

    void removeAsSaved()
    {
        saved = QStringList{ QStringLiteral("webOS") };
        SpellChecker checker;
        checker.removeFromUserWordlist(QStringLiteral("webOS"));
        QVERIFY(checker.userWords().isEmpty());
        QCOMPARE(deletedWords(), QStringList{ QStringLiteral("webOS") });
    }

    // Saved twice, in two cases: both go, each by its own spelling.
    void removeEverySpelling()
    {
        saved = QStringList{ QStringLiteral("webOS"), QStringLiteral("WebOS") };
        SpellChecker checker;
        checker.removeFromUserWordlist(QStringLiteral("WEBOS"));
        QVERIFY(checker.userWords().isEmpty());
        QCOMPARE(deletedWords(), (QStringList{ QStringLiteral("webOS"), QStringLiteral("WebOS") }));
    }

    void removeWhatIsNotThere()
    {
        saved = QStringList{ QStringLiteral("webOS") };
        SpellChecker checker;
        checker.removeFromUserWordlist(QStringLiteral("webs"));
        QCOMPARE(checker.userWords(), saved);
        QVERIFY(deletedWords().isEmpty());
    }

    // The delete is the db8 kind's, by its "word" property.
    void deleteQuery()
    {
        saved = QStringList{ QStringLiteral("webOS") };
        SpellChecker checker(QStringLiteral("test.dictionary:1"));
        checker.removeFromUserWordlist(QStringLiteral("webos"));
        QCOMPARE(calls.last().uri, QStringLiteral("luna://com.palm.db/del"));
        QCOMPARE(calls.last().payload.value(QStringLiteral("query")).toObject()
                     .value(QStringLiteral("from")).toString(),
                 QStringLiteral("test.dictionary:1"));
    }
};

QTEST_GUILESS_MAIN(TestSpellChecker)
#include "ut_spellchecker.moc"
