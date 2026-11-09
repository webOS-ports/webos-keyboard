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

#include "premediapanel.h"

#include <QBuffer>
#include <QElapsedTimer>
#include <QFontMetricsF>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace Pre {
namespace {

//! Pictures kept in memory, per tab.
const int CACHED = 60;
//! What showed stays as it was for this long between openings (as Klipy keeps answers).
const qint64 FRESH_FOR = 60 * 60000LL;

qint64 clockMs()
{
    static QElapsedTimer clock;
    if (!clock.isValid()) clock.start();
    return clock.elapsed();
}

} // namespace

MediaPanel::MediaPanel(Klipy::Kind kind, const std::function<void()> &update,
                       const std::function<void(const Klipy::Item &, const QString &)> &pick)
    : m_kind(kind), m_update(update), m_pick(pick)
{
}

MediaPanel::~MediaPanel()
{
    for (const Picture &pic : std::as_const(m_cache)) delete pic.movie;
}

void MediaPanel::open()
{
    if (!Klipy::instance().configured()) return;
    if (!m_items.isEmpty() && clockMs() - m_loadedAt < FRESH_FOR) return;
    if (m_loading) return;
    search(m_query);
}

void MediaPanel::search(const QString &q)
{
    m_query = q.trimmed();
    m_items.clear();
    m_page = 0;
    m_hasNext = false;
    m_failed = false;
    m_refusal.clear();
    m_scrollY = 0;
    m_loading = false;
    ++m_generation;
    relayout();
    if (Klipy::instance().configured()) more();
    if (m_update) m_update();
}

void MediaPanel::more()
{
    if (m_loading) return;
    m_loading = true;
    const int gen = ++m_generation;
    Klipy::instance().fetch(m_kind, m_query, m_page + 1, this, [this, gen](const Klipy::Answer &a) {
        if (gen != m_generation) return;
        m_loading = false;
        if (a.ok) {
            m_items += a.page.items;
            m_page = a.page.page;
            m_hasNext = a.page.hasNext && !a.page.items.isEmpty();
            m_loadedAt = clockMs();
            relayout();
        } else {
            m_failed = true;
            m_refusal = a.refusal;
        }
        if (m_update) m_update();
    });
}

void MediaPanel::close()
{
    for (const QString &url : std::as_const(m_running)) {
        auto it = m_cache.find(url);
        if (it != m_cache.end() && it->movie) it->movie->setPaused(true);
    }
    m_running.clear();
}

// --- Geometry ---------------------------------------------------------------------------------

void MediaPanel::layout(const QRectF &grid)
{
    m_grid = grid;
    relayout();
}

qreal MediaPanel::maxScroll() const { return std::max<qreal>(0, m_contentHeight - m_grid.height()); }

void MediaPanel::setScrollY(qreal y) { m_scrollY = std::max<qreal>(0, std::min(y, maxScroll())); }

//! Shortest column first, in KLIPY's order; GIFs keep their shape, stickers are squares.
void MediaPanel::relayout()
{
    if (m_grid.width() <= 0) return;
    const qreal gap = 3 * m_dp;
    // Big enough to see: two GIFs across a phone, three stickers.
    const qreal target = (m_kind == Klipy::Kind::Gif ? 170 : 110) * m_dp;
    const int cols = std::max(2, int(m_grid.width() / target));
    const qreal colW = (m_grid.width() - gap * (cols + 1)) / cols;
    QVector<qreal> heights(cols, gap);
    m_rects.clear();
    for (const Klipy::Item &item : std::as_const(m_items)) {
        int c = 0;
        for (int i = 1; i < cols; ++i)
            if (heights[i] < heights[c] - 0.5) c = i;
        const qreal h = m_kind == Klipy::Kind::Gif ? colW / std::clamp<qreal>(item.aspect(), 0.6, 2.4) : colW;
        const qreal x = gap + c * (colW + gap);
        m_rects << QRectF(x, heights[c], colW, h);
        heights[c] += h + gap;
    }
    m_contentHeight = m_rects.isEmpty() ? 0 : *std::max_element(heights.cbegin(), heights.cend());
    setScrollY(m_scrollY);
}

// --- Pictures ---------------------------------------------------------------------------------

//! The preview to show: a small animated WebP, else a small GIF, else a still.
QString MediaPanel::previewUrl(const Klipy::Item &item) const
{
    const QStringList small{ QStringLiteral("sm"), QStringLiteral("xs"), QStringLiteral("md") };
    const QStringList any{ QStringLiteral("sm"), QStringLiteral("xs"), QStringLiteral("md"), QStringLiteral("hd") };
    const Klipy::File *f = item.first(QStringLiteral("webp"), small);
    if (!f) f = item.first(QStringLiteral("gif"), { QStringLiteral("sm"), QStringLiteral("xs") });
    if (!f) f = item.first(QStringLiteral("jpg"), any);
    if (!f) f = item.first(QStringLiteral("png"), any);
    return f ? f->url : QString();
}

//! The picture for [url] if it is in; else null, and it is asked for after this paint.
const MediaPanel::Picture *MediaPanel::picture(const QString &url)
{
    auto it = m_cache.constFind(url);
    if (it != m_cache.cend()) return &it.value();
    return nullptr;
}

void MediaPanel::decode(const QString &url, const QByteArray &bytes, int width)
{
    if (bytes.isEmpty()) return;
    auto *movie = new QMovie(this);
    auto *buffer = new QBuffer(movie);
    buffer->setData(bytes);
    buffer->open(QIODevice::ReadOnly);
    movie->setDevice(buffer);
    movie->setCacheMode(QMovie::CacheNone);
    if (!movie->isValid() || !movie->jumpToFrame(0)) { delete movie; return; }
    // No bigger than the cell needs: a sm GIF is a few hundred pixels across.
    const QSize size = movie->currentImage().size();
    Picture pic;
    if (size.width() > width * 1.3 && width > 0) {
        const qreal s = qreal(width) / size.width();
        movie->setScaledSize(QSize(width, std::max(1, int(std::round(size.height() * s)))));
    }
    if (movie->frameCount() == 1) {
        // A still: kept as an image, no timer.
        pic.still = movie->scaledSize().isValid() ? movie->currentImage().scaled(movie->scaledSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                                  : movie->currentImage();
        delete movie;
    } else {
        movie->jumpToFrame(0);
        connect(movie, &QMovie::frameChanged, this, [this] { if (m_update) m_update(); });
        pic.movie = movie;
    }
    m_cache.insert(url, pic);
    m_cacheOrder << url;
    while (m_cacheOrder.size() > CACHED) drop(m_cacheOrder.takeFirst());
}

void MediaPanel::drop(const QString &url)
{
    auto it = m_cache.find(url);
    if (it == m_cache.end()) return;
    delete it->movie;
    m_cache.erase(it);
    m_running.remove(url);
}

//! Back on the keyboard's thread after a paint: fetch, start and stop what it found.
void MediaPanel::afterPaint()
{
    m_afterPaintQueued = false;
    for (auto it = m_wanted.cbegin(); it != m_wanted.cend(); ++it) {
        const QString url = it.key();
        const int width = it.value();
        if (m_cache.contains(url) || m_pending.contains(url)) continue;
        m_pending.insert(url);
        Klipy::instance().load(url, this, [this, url, width](const QByteArray &bytes) {
            m_pending.remove(url);
            decode(url, bytes, width);
            if (m_update) m_update();
        });
    }
    m_wanted.clear();
    // Only what's on screen moves.
    for (const QString &url : std::as_const(m_drawn)) {
        if (m_running.contains(url)) continue;
        auto it = m_cache.find(url);
        if (it == m_cache.end() || !it->movie) continue;
        if (it->movie->state() == QMovie::NotRunning) it->movie->start();
        else it->movie->setPaused(false);
        m_running.insert(url);
    }
    for (const QString &url : QSet<QString>(m_running).subtract(m_drawn)) {
        auto it = m_cache.find(url);
        if (it != m_cache.end() && it->movie) it->movie->setPaused(true);
        m_running.remove(url);
    }
    // Recently used: what was just drawn goes to the back of the queue.
    for (const QString &url : std::as_const(m_drawn)) {
        m_cacheOrder.removeOne(url);
        m_cacheOrder << url;
    }
    if (m_wantMore) {
        m_wantMore = false;
        more();
    }
}

// --- Drawing ----------------------------------------------------------------------------------

void MediaPanel::say(const QString &s, int ms)
{
    m_toast = s;
    m_toastUntil = clockMs() + ms;
    if (m_update) m_update();
    QTimer::singleShot(ms + 20, this, [this] { if (m_update) m_update(); });
}

void MediaPanel::draw(QPainter &p, const QFont &font)
{
    p.save();
    p.setClipRect(m_grid);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    m_drawn.clear();
    const qreal r = 6 * m_dp;
    for (int i = 0; i < m_rects.size() && i < m_items.size(); ++i) {
        const QRectF &box = m_rects[i];
        const qreal top = m_grid.top() + box.top() - m_scrollY;
        if (top > m_grid.bottom() || top + box.height() < m_grid.top()) continue;
        const QRectF cell(m_grid.left() + box.left(), top, box.width(), box.height());
        const Klipy::Item &item = m_items[i];
        const QString url = previewUrl(item);
        const Picture *pic = url.isEmpty() ? nullptr : picture(url);
        if (!url.isEmpty() && !pic) m_wanted.insert(url, int(cell.width()));
        p.save();
        if (m_kind == Klipy::Kind::Gif) {
            QPainterPath clip;
            clip.addRoundedRect(cell, r, r);
            p.setClipPath(clip, Qt::IntersectClip);
        }
        const QImage img = !pic ? QImage() : pic->movie ? pic->movie->currentImage() : pic->still;
        if (img.isNull()) {
            QImage blur;
            if (m_kind == Klipy::Kind::Gif && !item.blur.isEmpty()) {
                auto b = m_blurs.constFind(item.id);
                if (b != m_blurs.cend()) {
                    blur = b.value();
                } else {
                    blur = QImage::fromData(QByteArray::fromBase64(item.blur.section(QLatin1Char(','), 1).toLatin1()));
                    m_blurs.insert(item.id, blur);
                }
            }
            if (!blur.isNull()) {
                p.drawImage(cell, blur);
            } else {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(255, 255, 255, 28));
                p.drawRoundedRect(cell, r, r);
            }
        } else {
            const qreal w = img.width(), h = img.height();
            // GIFs fill their cell (it has their shape); stickers fit inside theirs.
            const qreal s = m_kind == Klipy::Kind::Gif ? std::max(cell.width() / w, cell.height() / h)
                                                       : std::min(cell.width() / w, cell.height() / h) * 0.92;
            const qreal dw = w * s, dh = h * s;
            p.drawImage(QRectF(cell.center().x() - dw / 2, cell.center().y() - dh / 2, dw, dh), img);
            if (pic->movie) m_drawn.insert(url);
        }
        if (i == pressed) p.fillRect(cell, QColor(255, 255, 255, 70));
        p.restore();
    }

    QString message;
    const QString noun = Klipy::noun(m_kind);
    if (!Klipy::instance().configured())
        message = noun.left(1).toUpper() + noun.mid(1) + QStringLiteral(" need a KLIPY key in this build");
    else if (m_items.isEmpty() && m_loading) message = QStringLiteral("Loading…");
    else if (m_items.isEmpty() && !m_refusal.isEmpty()) message = QStringLiteral("KLIPY: ") + m_refusal;
    else if (m_items.isEmpty() && m_failed) message = QStringLiteral("Can't reach KLIPY. Tap to try again.");
    else if (m_items.isEmpty() && !m_query.isEmpty()) message = QStringLiteral("No %1 for “%2”").arg(noun, m_query);
    else if (m_items.isEmpty()) message = QStringLiteral("No %1 right now").arg(noun);
    if (!message.isEmpty()) {
        QFont f = font;
        qreal size = 15 * m_dp;
        f.setPixelSize(std::max(1, int(size)));
        const qreal w = QFontMetricsF(f).horizontalAdvance(message);
        if (w > m_grid.width() - 24 * m_dp) f.setPixelSize(std::max(1, int(size * (m_grid.width() - 24 * m_dp) / w)));
        p.setFont(f);
        p.setPen(QColor(255, 255, 255, 170));
        p.drawText(QPointF(m_grid.center().x() - QFontMetricsF(f).horizontalAdvance(message) / 2, m_grid.center().y()), message);
    }
    // Scrolled to within a screen of the end: the next page.
    if (m_hasNext && !m_loading && !m_failed && m_scrollY > 0 && m_scrollY + m_grid.height() * 2 > m_contentHeight)
        m_wantMore = true;
    p.restore();

    if (!m_toast.isEmpty() && clockMs() < m_toastUntil) {
        QFont f = font;
        f.setPixelSize(std::max(1, int(14 * m_dp)));
        const QFontMetricsF fm(f);
        const qreal w = fm.horizontalAdvance(m_toast) + 28 * m_dp;
        const QRectF box(m_grid.center().x() - w / 2, m_grid.bottom() - 40 * m_dp, w, 30 * m_dp);
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(20, 22, 24, 225));
        p.drawRoundedRect(box, box.height() / 2, box.height() / 2);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(box, Qt::AlignCenter, m_toast);
        p.restore();
    }

    if (!m_afterPaintQueued && (!m_wanted.isEmpty() || m_wantMore || m_drawn != m_running)) {
        m_afterPaintQueued = true;
        QMetaObject::invokeMethod(this, &MediaPanel::afterPaint, Qt::QueuedConnection);
    }
}

// --- Touch ------------------------------------------------------------------------------------

int MediaPanel::itemAt(qreal x, qreal y) const
{
    if (!m_grid.contains(QPointF(x, y))) return -1;
    const QPointF c(x - m_grid.left(), y - m_grid.top() + m_scrollY);
    for (int i = 0; i < m_rects.size(); ++i)
        if (m_rects[i].contains(c)) return i;
    return -1;
}

void MediaPanel::tap(qreal x, qreal y)
{
    if (m_items.isEmpty()) {
        if (m_failed && !m_loading) search(m_query);
        return;
    }
    const int i = itemAt(x, y);
    if (i >= 0 && i < m_items.size() && m_pick) m_pick(m_items[i], m_query);
}

} // namespace Pre
