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

#ifndef PRE_KLIPY_H
#define PRE_KLIPY_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

#include <functional>

class QNetworkAccessManager;
class TestKlipy;

namespace Pre {

/*!
 * GIFs and stickers from KLIPY (docs.klipy.com). The requests go straight from the
 * phone to KLIPY and the pictures are loaded straight from the addresses KLIPY
 * gives, as its terms ask; nothing is kept on disk.
 *
 * The app key is KLIPY's for this keyboard, put in at build time (qmake
 * KLIPY_KEY=..., or the KLIPY_KEY environment variable); without one, the GIF and
 * sticker tabs say so.
 *
 * KLIPY counts calls to its API (a key in testing is allowed 100 an hour), not the
 * pictures, which come from its file servers. So each answer is kept for an hour
 * and asked for again only after that, and no more than HOURLY calls go out in any
 * hour; the count is kept in the keyboard's state directory, so it holds across
 * the keyboard being restarted.
 *
 * One for the process, on the thread that made it (the keyboard's).
 */
class Klipy : public QObject
{
    Q_OBJECT

public:
    enum class Kind { Gif, Sticker };
    static QString path(Kind kind) { return kind == Kind::Gif ? QStringLiteral("gifs") : QStringLiteral("stickers"); }
    static QString noun(Kind kind) { return kind == Kind::Gif ? QStringLiteral("GIFs") : QStringLiteral("stickers"); }

    //! One rendition of an item: a size (hd, md, sm, xs) in one format (gif, webp, jpg, png, mp4, webm).
    struct File { QString url; int width = 0; int height = 0; qint64 size = 0; };

    struct Item
    {
        QString id;
        QString slug;
        QString title;
        //! size -> format -> file.
        QHash<QString, QHash<QString, File>> files;
        //! A tiny blurred picture, as a data: URL, to show till the real one is in.
        QString blur;

        const File *file(const QString &size, const QString &format) const;
        //! The first of [sizes] that has [format].
        const File *first(const QString &format, const QStringList &sizes) const;
        //! Width over height, from whichever rendition says.
        qreal aspect() const;
    };

    struct Page { QList<Item> items; int page = 1; bool hasNext = false; };

    //! What came back: a page, or why not (KLIPY's own reason when it refused, else empty).
    struct Answer { bool ok = false; Page page; QString refusal; };

    static Klipy &instance();

    QString key() const { return m_key; }
    //! The tests blank it, so they never go to the network.
    void setKey(const QString &key) { m_key = key.trimmed(); }
    bool configured() const { return !m_key.isEmpty(); }
    //! Where the call count and the anonymous id are kept.
    void setStateDir(const QString &dir) { m_stateDir = dir; }

    static constexpr int PER_PAGE = 24;
    //! Calls to KLIPY's API allowed in any hour, a little under a testing key's 100.
    static constexpr int HOURLY = 90;

    //! The trending page, or a search when [query] is given. [done] runs unless [context] is gone.
    void fetch(Kind kind, const QString &query, int page, QObject *context, const std::function<void(const Answer &)> &done);
    //! Tells KLIPY [item] was sent (its share trigger), from the search [query] if there was one.
    void shared(Kind kind, const Item &item, const QString &query);
    //! The bytes at [url]; empty if they couldn't be had. [done] runs unless [context] is gone.
    void load(const QString &url, QObject *context, const std::function<void(const QByteArray &)> &done);

    //! KLIPY's answer to a list request; [refusal] gets its reason when it said no.
    static Page parse(const QByteArray &json, bool *ok, QString *refusal);
    //! The reason in an answer of KLIPY's: {"errors":{"message":["..."]}} or {"message":"..."}.
    static QString message(const QByteArray &json);
    /*!
     * Where [item] can be seen. LuneOS's applications take no pictures from a
     * keyboard, so a GIF or sticker goes in as this link: a GIF first, as the
     * likeliest to show moving wherever it is opened.
     */
    static QString link(const Item &item);

private:
    friend class ::TestKlipy;
    Klipy();
    //! Milliseconds on a clock that isn't set back, for how old an answer is.
    static qint64 monotonicMs();
    QNetworkAccessManager *net();
    bool allowCall();
    QString customerId();
    QString stateFile() const;

    QString m_key;
    QString m_stateDir;
    QNetworkAccessManager *m_net = nullptr;
    //! Answers already had, by kind, search and page, with when they came: an hour, the last few dozen.
    QHash<QString, QPair<qint64, Page>> m_answers;
    QStringList m_answerOrder;
};

} // namespace Pre

#endif // PRE_KLIPY_H
