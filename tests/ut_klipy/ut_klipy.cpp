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

// The Pre Keyboard's GIFs and stickers from KLIPY, without the network: what
// KLIPY's answers are read as, the link a GIF goes in as, the hourly limit on
// calls, the hour answers are kept, and the grid not fetching a picture it
// could not show over and over.

#include "preklipy.h"
#include "premediapanel.h"

#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using Pre::Klipy;
using Pre::MediaPanel;

namespace {

//! A file of one size, in KLIPY's shape.
QString file(const QString &url, int w, int h, qint64 size)
{
    return QStringLiteral(R"({"url":"%1","width":%2,"height":%3,"size":%4})").arg(url).arg(w).arg(h).arg(size);
}

//! A trending page shaped as docs.klipy.com's example: one GIF, then an ad (no file).
QByteArray trendingPage()
{
    const QString hd = QStringLiteral(R"("hd":{"gif":%1,"webp":%2,"mp4":%3})")
            .arg(file(QStringLiteral("https://static.klipy.com/ii/a/hd.gif"), 498, 498, 4001918),
                 file(QStringLiteral("https://static.klipy.com/ii/a/hd.webp"), 498, 498, 285228),
                 file(QStringLiteral("https://static.klipy.com/ii/a/hd.mp4"), 498, 498, 119294));
    const QString md = QStringLiteral(R"("md":{"gif":%1,"jpg":%2})")
            .arg(file(QStringLiteral("https://static.klipy.com/ii/a/md.gif"), 498, 249, 3721260),
                 file(QStringLiteral("https://static.klipy.com/ii/a/md.jpg"), 498, 249, 20086));
    const QString sm = QStringLiteral(R"("sm":{"webp":%1,"empty":{"url":""}})")
            .arg(file(QStringLiteral("https://static.klipy.com/ii/a/sm.webp"), 220, 110, 80118));
    return QStringLiteral(R"({"result":true,"data":{"data":[)"
                          R"({"id":9007199254740993,"slug":"hello-hi-662","title":"Hello","file":{%1,%2,%3},)"
                          R"("type":"gif","blur_preview":"data:image/jpeg;base64,AAAA"},)"
                          R"({"type":"ad","id":1,"content":"<div></div>"},)"
                          R"({"id":"s-2","slug":"no-files","file":{"hd":{}}})"
                          R"(],"current_page":3,"per_page":24,"has_next":true}})")
            .arg(hd, md, sm).toUtf8();
}

} // namespace

class TestKlipy : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        QVERIFY(m_state.isValid());
        Klipy &k = Klipy::instance();
        // Never the network: no key, and every test's state of its own.
        k.setKey(QString());
        k.setStateDir(m_state.path());
        QFile::remove(k.stateFile());
        k.m_answers.clear();
        k.m_answerOrder.clear();
    }

    void parsesAPage()
    {
        bool ok = false;
        QString refusal;
        const Klipy::Page page = Klipy::parse(trendingPage(), &ok, &refusal);
        QVERIFY(ok);
        QVERIFY(refusal.isEmpty());
        // The ad and the item with no files are not pictures.
        QCOMPARE(page.items.size(), 1);
        const Klipy::Item &item = page.items.first();
        QCOMPARE(item.id, QStringLiteral("9007199254740993"));
        QCOMPARE(item.slug, QStringLiteral("hello-hi-662"));
        QCOMPARE(item.title, QStringLiteral("Hello"));
        QCOMPARE(item.blur, QStringLiteral("data:image/jpeg;base64,AAAA"));
        QCOMPARE(item.files.size(), 3);
        QVERIFY(!item.file(QStringLiteral("sm"), QStringLiteral("empty")));
        const Klipy::File *gif = item.file(QStringLiteral("hd"), QStringLiteral("gif"));
        QVERIFY(gif);
        QCOMPARE(gif->width, 498);
        QCOMPARE(gif->size, qint64(4001918));
        QCOMPARE(page.page, 3);
        QVERIFY(page.hasNext);
    }

    void refusalsSayWhy_data()
    {
        QTest::addColumn<QByteArray>("json");
        QTest::addColumn<QString>("why");
        QTest::newRow("errors.message list")
                << QByteArray(R"({"result":false,"errors":{"message":["The provided API key is invalid."]}})")
                << QStringLiteral("The provided API key is invalid.");
        QTest::newRow("errors.message string")
                << QByteArray(R"({"result":false,"errors":{"message":"Too many requests"}})")
                << QStringLiteral("Too many requests");
        QTest::newRow("message") << QByteArray(R"({"result":false,"message":"Gone"})") << QStringLiteral("Gone");
        QTest::newRow("no reason") << QByteArray(R"({"result":false})") << QStringLiteral("KLIPY said no");
    }

    void refusalsSayWhy()
    {
        QFETCH(QByteArray, json);
        QFETCH(QString, why);
        bool ok = true;
        QString refusal;
        const Klipy::Page page = Klipy::parse(json, &ok, &refusal);
        QVERIFY(!ok);
        QVERIFY(page.items.isEmpty());
        QCOMPARE(refusal, why);
    }

    void notJsonIsNotAPage()
    {
        bool ok = true;
        QString refusal;
        for (const QByteArray &junk : { QByteArray(), QByteArray("<html>502</html>"), QByteArray("[]"),
                                        QByteArray(R"({"result":true})"), QByteArray(R"({"data":"x"})") }) {
            Klipy::parse(junk, &ok, &refusal);
            QVERIFY2(!ok, junk.constData());
        }
        QVERIFY(Klipy::message("not json").isEmpty());
    }

    void linkIsAGifFirst()
    {
        bool ok = false;
        Klipy::Item item = Klipy::parse(trendingPage(), &ok, nullptr).items.first();
        // md before hd: smaller to open, still clear.
        QCOMPARE(Klipy::link(item), QStringLiteral("https://static.klipy.com/ii/a/md.gif"));
        item.files[QStringLiteral("md")].remove(QStringLiteral("gif"));
        QCOMPARE(Klipy::link(item), QStringLiteral("https://static.klipy.com/ii/a/hd.gif"));
        item.files[QStringLiteral("hd")].remove(QStringLiteral("gif"));
        QCOMPARE(Klipy::link(item), QStringLiteral("https://static.klipy.com/ii/a/sm.webp"));
        Klipy::Item still;
        still.files[QStringLiteral("xs")][QStringLiteral("png")] = Klipy::File{ QStringLiteral("https://s/x.png"), 1, 1, 1 };
        QCOMPARE(Klipy::link(still), QStringLiteral("https://s/x.png"));
        QVERIFY(Klipy::link(Klipy::Item()).isEmpty());
    }

    //! A testing key is allowed 100 calls an hour: no more than HOURLY go out.
    void hourlyLimit()
    {
        Klipy &k = Klipy::instance();
        for (int i = 0; i < Klipy::HOURLY; ++i) QVERIFY2(k.allowCall(), qPrintable(QString::number(i)));
        QVERIFY(!k.allowCall());
        // Kept on disk: a restarted keyboard counts on.
        QSettings s(k.stateFile(), QSettings::IniFormat);
        QCOMPARE(s.value(QStringLiteral("calls")).toString().split(QLatin1Char(',')).size(), Klipy::HOURLY);
    }

    //! Calls older than an hour, or stamped in the future (a clock set back), don't count.
    void hourlyLimitForgets()
    {
        Klipy &k = Klipy::instance();
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        QStringList calls;
        for (int i = 0; i < Klipy::HOURLY; ++i) calls << QString::number(now - 3600000LL - 1000 - i);
        for (int i = 0; i < Klipy::HOURLY; ++i) calls << QString::number(now + 600000LL + i);
        {
            QSettings s(k.stateFile(), QSettings::IniFormat);
            s.setValue(QStringLiteral("calls"), calls.join(QLatin1Char(',')));
        }
        QVERIFY(k.allowCall());
        QSettings s(k.stateFile(), QSettings::IniFormat);
        QCOMPARE(s.value(QStringLiteral("calls")).toString().split(QLatin1Char(',')).size(), 1);
    }

    void customerIdStaysTheSame()
    {
        Klipy &k = Klipy::instance();
        const QString id = k.customerId();
        QVERIFY(!id.isEmpty());
        QCOMPARE(k.customerId(), id);
    }

    //! An answer is used again for an hour; after that it is asked for anew.
    void answersKeptForAnHour()
    {
        Klipy &k = Klipy::instance();
        // The hour's calls used up, so an answer not kept can't go to the network either.
        for (int i = 0; i < Klipy::HOURLY; ++i) k.allowCall();
        bool ok = false;
        const Klipy::Page page = Klipy::parse(trendingPage(), &ok, nullptr);
        QVERIFY(ok);

        Klipy::Answer a;
        bool came = false;
        QObject context;
        auto got = [&](const Klipy::Answer &answer) { a = answer; came = true; };

        k.m_answers.insert(QStringLiteral("gifs|cats|1"), { Klipy::monotonicMs() - 1000, page });
        k.fetch(Klipy::Kind::Gif, QStringLiteral("  Cats "), 1, &context, got);
        QTRY_VERIFY_WITH_TIMEOUT(came, 2000);
        QVERIFY(a.ok);
        QCOMPARE(a.page.items.size(), 1);

        came = false;
        k.m_answers.insert(QStringLiteral("gifs|cats|1"), { Klipy::monotonicMs() - 60 * 60000LL - 1, page });
        k.fetch(Klipy::Kind::Gif, QStringLiteral("cats"), 1, &context, got);
        QTRY_VERIFY_WITH_TIMEOUT(came, 2000);
        QVERIFY(!a.ok);
        QVERIFY(a.refusal.contains(QStringLiteral("enough for this hour")));
        QVERIFY(!k.m_answers.contains(QStringLiteral("gifs|cats|1")));
    }

    //! A picture that can't be shown is drawn blank, not fetched again with every paint.
    void brokenPictureNotFetchedAgain()
    {
        MediaPanel panel(Klipy::Kind::Gif, [] {}, [](const Klipy::Item &, const QString &) {});
        panel.setDp(1);
        panel.layout(QRectF(0, 0, 400, 400));
        bool ok = false;
        panel.m_items = Klipy::parse(trendingPage(), &ok, nullptr).items;
        panel.relayout();
        const QString url = panel.previewUrl(panel.m_items.first());
        QVERIFY(!url.isEmpty());

        QImage img(400, 400, QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        panel.draw(p, QFont());
        QVERIFY(panel.m_wanted.contains(url));
        panel.m_wanted.clear();

        panel.decode(url, QByteArray("not a picture"), 200);
        QVERIFY(!panel.m_cache.contains(url));
        panel.draw(p, QFont());
        QVERIFY(!panel.m_wanted.contains(url));

        panel.decode(QStringLiteral("https://x/none"), QByteArray(), 200);
        QVERIFY(panel.m_broken.contains(QStringLiteral("https://x/none")));

        // A new search tries again.
        panel.m_broken.insert(url);
        panel.search(QStringLiteral("dogs"));
        QVERIFY(panel.m_broken.isEmpty());
        QVERIFY(panel.m_blurs.isEmpty());
        p.end();
    }

    void unconfiguredSaysSo()
    {
        QVERIFY(!Klipy::instance().configured());
        MediaPanel panel(Klipy::Kind::Sticker, [] {}, [](const Klipy::Item &, const QString &) {});
        panel.open();
        QVERIFY(!panel.m_loading);
        QVERIFY(panel.m_items.isEmpty());
    }

private:
    QTemporaryDir m_state;
};

QTEST_MAIN(TestKlipy)

#include "ut_klipy.moc"
