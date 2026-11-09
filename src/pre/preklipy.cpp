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

#include "preklipy.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

#ifndef PRE_KLIPY_KEY
#define PRE_KLIPY_KEY ""
#endif

namespace Pre {
namespace {

const char API[] = "https://api.klipy.com/api/v1/";
const qint64 KEEP_ANSWERS = 60 * 60000LL;
const int ANSWERS_KEPT = 40;
const int TIMEOUT_MS = 20000;

qint64 monotonicMs()
{
    static QElapsedTimer clock;
    if (!clock.isValid()) clock.start();
    return clock.elapsed();
}

QNetworkRequest request(const QUrl &url)
{
    QNetworkRequest r(url);
    r.setTransferTimeout(TIMEOUT_MS);
    r.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PreKeyboard-LuneOS"));
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return r;
}

QString enc(const QString &s) { return QString::fromLatin1(QUrl::toPercentEncoding(s)); }

} // namespace

// --- Items ------------------------------------------------------------------------------------

const Klipy::File *Klipy::Item::file(const QString &size, const QString &format) const
{
    auto s = files.constFind(size);
    if (s == files.cend()) return nullptr;
    auto f = s->constFind(format);
    return f == s->cend() ? nullptr : &f.value();
}

const Klipy::File *Klipy::Item::first(const QString &format, const QStringList &sizes) const
{
    for (const QString &s : sizes)
        if (const File *f = file(s, format)) return f;
    return nullptr;
}

qreal Klipy::Item::aspect() const
{
    for (const auto &m : files)
        for (const File &f : m)
            if (f.width > 0 && f.height > 0) return qreal(f.width) / f.height;
    return 1;
}

// --- The client -------------------------------------------------------------------------------

Klipy::Klipy() : m_key(QString::fromUtf8(PRE_KLIPY_KEY).trimmed()) {}

Klipy &Klipy::instance()
{
    static Klipy *k = new Klipy;
    return *k;
}

QNetworkAccessManager *Klipy::net()
{
    if (!m_net) m_net = new QNetworkAccessManager(this);
    return m_net;
}

QString Klipy::stateFile() const
{
    return (m_stateDir.isEmpty() ? QDir::tempPath() : m_stateDir) + QStringLiteral("/klipy.ini");
}

/*!
 * Whether another call may go out, counting it if so. The times of the last hour's
 * calls are kept, so the count holds across the keyboard being restarted.
 */
bool Klipy::allowCall()
{
    QSettings s(stateFile(), QSettings::IniFormat);
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QStringList recent;
    for (const QString &t : s.value(QStringLiteral("calls")).toString().split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const qint64 at = t.toLongLong();
        if (at >= now - 3600000LL && at <= now) recent << t;
    }
    if (recent.size() >= HOURLY) return false;
    recent << QString::number(now);
    s.setValue(QStringLiteral("calls"), recent.join(QLatin1Char(',')));
    return true;
}

//! A stable, anonymous id for this phone's keyboard, as KLIPY asks: a random one, made once.
QString Klipy::customerId()
{
    QSettings s(stateFile(), QSettings::IniFormat);
    QString id = s.value(QStringLiteral("customer")).toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        s.setValue(QStringLiteral("customer"), id);
    }
    return id;
}

void Klipy::fetch(Kind kind, const QString &query, int page, QObject *context,
                  const std::function<void(const Answer &)> &done)
{
    const QString q = query.trimmed().toLower();
    const QString cacheKey = path(kind) + QLatin1Char('|') + q + QLatin1Char('|') + QString::number(page);
    QPointer<QObject> ctx(context);
    auto answer = [ctx, done](const Answer &a) { if (ctx) done(a); };
    auto had = m_answers.constFind(cacheKey);
    if (had != m_answers.cend()) {
        if (monotonicMs() - had->first < KEEP_ANSWERS) {
            const Answer a{ true, had->second, QString() };
            QMetaObject::invokeMethod(this, [answer, a] { answer(a); }, Qt::QueuedConnection);
            return;
        }
        m_answers.remove(cacheKey);
        m_answerOrder.removeAll(cacheKey);
    }
    if (!allowCall()) {
        const Answer a{ false, Page(), QStringLiteral("that's enough for this hour. Try again later.") };
        QMetaObject::invokeMethod(this, [answer, a] { answer(a); }, Qt::QueuedConnection);
        return;
    }
    QString url = QLatin1String(API) + enc(m_key) + QLatin1Char('/') + path(kind)
            + (q.isEmpty() ? QStringLiteral("/trending") : QStringLiteral("/search"))
            + QStringLiteral("?page=") + QString::number(page) + QStringLiteral("&per_page=") + QString::number(PER_PAGE);
    if (!q.isEmpty()) url += QStringLiteral("&q=") + enc(query.trimmed());
    url += QStringLiteral("&customer_id=") + enc(customerId());
    const QString country = QLocale::territoryToCode(QLocale::system().territory()).toLower();
    if (country.length() == 2) url += QStringLiteral("&locale=") + country;
    url += QStringLiteral("&content_filter=medium");

    QNetworkReply *reply = net()->get(request(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, cacheKey, answer] {
        reply->deleteLater();
        const QByteArray body = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        Answer a;
        if (reply->error() != QNetworkReply::NoError && status == 0) {
            a.ok = false;          // no answer at all: offline, or KLIPY unreachable
        } else if (status < 200 || status > 299) {
            // KLIPY says what was wrong ("The provided API key is invalid."): that, not a number.
            a.refusal = message(body);
            if (a.refusal.isEmpty()) a.refusal = QStringLiteral("HTTP %1").arg(status);
        } else {
            a.page = parse(body, &a.ok, &a.refusal);
        }
        if (a.ok) {
            m_answers.insert(cacheKey, { monotonicMs(), a.page });
            m_answerOrder.removeAll(cacheKey);
            m_answerOrder << cacheKey;
            while (m_answerOrder.size() > ANSWERS_KEPT) m_answers.remove(m_answerOrder.takeFirst());
        }
        answer(a);
    });
}

void Klipy::shared(Kind kind, const Item &item, const QString &query)
{
    if (!configured() || !allowCall()) return;
    const QUrl url(QLatin1String(API) + enc(m_key) + QLatin1Char('/') + path(kind) + QStringLiteral("/share/") + enc(item.slug));
    QJsonObject body{ { QStringLiteral("customer_id"), customerId() } };
    if (!query.trimmed().isEmpty()) body.insert(QStringLiteral("q"), query.trimmed());
    QNetworkRequest r = request(url);
    r.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = net()->post(r, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}

void Klipy::load(const QString &url, QObject *context, const std::function<void(const QByteArray &)> &done)
{
    QPointer<QObject> ctx(context);
    QNetworkReply *reply = net()->get(request(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [reply, ctx, done] {
        reply->deleteLater();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray b = (reply->error() == QNetworkReply::NoError && status >= 200 && status <= 299)
                ? reply->readAll() : QByteArray();
        if (ctx) done(b);
    });
}

// --- KLIPY's answers --------------------------------------------------------------------------

Klipy::Page Klipy::parse(const QByteArray &json, bool *ok, QString *refusal)
{
    Page page;
    if (ok) *ok = false;
    const QJsonObject root = QJsonDocument::fromJson(json).object();
    if (root.isEmpty()) return page;
    if (!root.value(QStringLiteral("result")).toBool(true)) {
        if (refusal) {
            *refusal = message(json);
            if (refusal->isEmpty()) *refusal = QStringLiteral("KLIPY said no");
        }
        return page;
    }
    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    if (data.isEmpty()) return page;
    for (const QJsonValue &v : data.value(QStringLiteral("data")).toArray()) {
        const QJsonObject o = v.toObject();
        // Only pictures: an ad (not asked for) has no file.
        const QJsonObject fileObj = o.value(QStringLiteral("file")).toObject();
        if (fileObj.isEmpty()) continue;
        Item item;
        for (auto s = fileObj.constBegin(); s != fileObj.constEnd(); ++s) {
            const QJsonObject formats = s.value().toObject();
            QHash<QString, File> m;
            for (auto f = formats.constBegin(); f != formats.constEnd(); ++f) {
                const QJsonObject fo = f.value().toObject();
                const QString u = fo.value(QStringLiteral("url")).toString();
                if (u.isEmpty()) continue;
                m.insert(f.key(), File{ u, fo.value(QStringLiteral("width")).toInt(), fo.value(QStringLiteral("height")).toInt(),
                                        qint64(fo.value(QStringLiteral("size")).toDouble()) });
            }
            if (!m.isEmpty()) item.files.insert(s.key(), m);
        }
        if (item.files.isEmpty()) continue;
        const QJsonValue id = o.value(QStringLiteral("id"));
        item.id = id.isString() ? id.toString() : QString::number(qint64(id.toDouble()));
        item.slug = o.value(QStringLiteral("slug")).toString();
        item.title = o.value(QStringLiteral("title")).toString();
        const QString blur = o.value(QStringLiteral("blur_preview")).toString();
        if (blur.startsWith(QStringLiteral("data:"))) item.blur = blur;
        page.items << item;
    }
    page.page = data.value(QStringLiteral("current_page")).toInt(1);
    page.hasNext = data.value(QStringLiteral("has_next")).toBool(false);
    if (ok) *ok = true;
    return page;
}

QString Klipy::message(const QByteArray &json)
{
    const QJsonObject o = QJsonDocument::fromJson(json).object();
    const QJsonValue errors = o.value(QStringLiteral("errors"));
    if (errors.isObject()) {
        const QJsonValue m = errors.toObject().value(QStringLiteral("message"));
        if (m.isArray() && !m.toArray().isEmpty() && !m.toArray().first().toString().isEmpty())
            return m.toArray().first().toString();
        if (m.isString() && !m.toString().isEmpty()) return m.toString();
    }
    return o.value(QStringLiteral("message")).toString();
}

QString Klipy::link(const Item &item)
{
    const QStringList sizes{ QStringLiteral("md"), QStringLiteral("sm"), QStringLiteral("hd"), QStringLiteral("xs") };
    for (const QString &format : { QStringLiteral("gif"), QStringLiteral("webp"), QStringLiteral("png") })
        if (const File *f = item.first(format, sizes)) return f->url;
    return QString();
}

} // namespace Pre
