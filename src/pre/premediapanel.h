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

#ifndef PRE_MEDIAPANEL_H
#define PRE_MEDIAPANEL_H

#include "preklipy.h"

#include <QFont>
#include <QHash>
#include <QImage>
#include <QList>
#include <QObject>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QStringList>

#include <functional>

class QMovie;
class QPainter;
class TestKlipy;

namespace Pre {

/*!
 * One KLIPY tab of the emoji page: GIFs or stickers, trending or searched for, in
 * columns that scroll like the emoji grid. The pictures move (animated WebP, or
 * GIF) and are kept only in memory; only those scrolled to are loaded, and only
 * those on screen move. The next page is asked for only once the owner scrolls
 * near the end, since KLIPY counts those calls.
 *
 * The emoji page owns it: gives it the grid's area, the scrolling, and the taps.
 *
 * Drawing may happen on the scene graph's thread (with the keyboard's waiting),
 * so draw() only looks: what it found wanting (pictures to fetch, pictures to
 * start or stop, the next page) is done back on the keyboard's thread after it.
 */
class MediaPanel : public QObject
{
    Q_OBJECT

public:
    MediaPanel(Klipy::Kind kind, const std::function<void()> &update,
               const std::function<void(const Klipy::Item &, const QString &)> &pick);
    ~MediaPanel() override;

    Klipy::Kind kind() const { return m_kind; }
    //! What was searched for, or empty for what's trending.
    QString query() const { return m_query; }

    void setDp(qreal dp) { m_dp = dp; relayout(); }
    //! Shows what's trending, unless it (or a search) is already showing and fresh.
    void open();
    //! Starts again from the first page of [q] (trending when blank).
    void search(const QString &q);
    //! Lets the moving pictures rest while the tab isn't showing.
    void close();

    void layout(const QRectF &grid);
    qreal scrollY() const { return m_scrollY; }
    void setScrollY(qreal y);
    qreal maxScroll() const;

    void draw(QPainter &p, const QFont &font);

    //! The item at [x], [y], or -1.
    int itemAt(qreal x, qreal y) const;
    void tap(qreal x, qreal y);
    //! The item under a finger that is down, lit till it lifts.
    int pressed = -1;

    //! A line over the bottom of the grid for a moment ("Sent as a link").
    void say(const QString &s, int ms = 2500);

private:
    friend class ::TestKlipy;
    struct Picture { QMovie *movie = nullptr; QImage still; };

    void more();
    void relayout();
    QString previewUrl(const Klipy::Item &item) const;
    const Picture *picture(const QString &url);
    void decode(const QString &url, const QByteArray &bytes, int width);
    void afterPaint();
    void drop(const QString &url);

    Klipy::Kind m_kind;
    std::function<void()> m_update;
    std::function<void(const Klipy::Item &, const QString &)> m_pick;
    qreal m_dp = 2;

    QString m_query;
    QList<Klipy::Item> m_items;
    int m_page = 0;
    bool m_hasNext = false;
    bool m_loading = false;
    bool m_failed = false;
    //! What KLIPY said when it refused (a bad key, say), rather than not answering.
    QString m_refusal;
    //! Answers to older requests are dropped when this has moved on.
    int m_generation = 0;
    qint64 m_loadedAt = 0;

    QRectF m_grid;
    QList<QRectF> m_rects;
    qreal m_contentHeight = 0;
    qreal m_scrollY = 0;

    // Pictures: by address, the last CACHED used; those waiting to come; those moving.
    QHash<QString, Picture> m_cache;
    QStringList m_cacheOrder;
    QHash<QString, int> m_wanted;      // asked for in the last paint, with the width it is for
    QSet<QString> m_pending;
    //! Pictures that didn't come or couldn't be shown: not asked for again till the next search.
    QSet<QString> m_broken;
    QSet<QString> m_drawn;
    QSet<QString> m_running;
    QHash<QString, QImage> m_blurs;
    bool m_afterPaintQueued = false;
    bool m_wantMore = false;

    QString m_toast;
    qint64 m_toastUntil = 0;
};

} // namespace Pre

#endif // PRE_MEDIAPANEL_H
