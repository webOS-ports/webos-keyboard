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

#include "preemojipanel.h"
#include "premediapanel.h"

#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QMutex>
#include <QHash>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace Pre {
namespace {

const qreal SLOP_DP = 8;
//! A fling slows by this share of its speed every millisecond.
const qreal FRICTION = 0.0028;

QFont emojiFont(qreal px)
{
    QFont f(QStringLiteral("Noto Color Emoji"));
    f.setPixelSize(std::max(1, int(px)));
    f.setStyleStrategy(QFont::NoFontMerging);
    return f;
}

} // namespace

EmojiPanel::Tab EmojiPanel::s_lastTab = EmojiPanel::Tab::Emoji;

EmojiPanel::EmojiPanel(qreal dp, const Actions &actions)
    : m_dp(dp), m_actions(actions), m_tab(s_lastTab)
{
    auto update = [this] { if (m_actions.update) m_actions.update(); };
    m_gifs.reset(new MediaPanel(Klipy::Kind::Gif, update, [this](const Klipy::Item &item, const QString &q) {
        if (m_actions.media) m_actions.media(Klipy::Kind::Gif, item, q);
    }));
    m_stickers.reset(new MediaPanel(Klipy::Kind::Sticker, update, [this](const Klipy::Item &item, const QString &q) {
        if (m_actions.media) m_actions.media(Klipy::Kind::Sticker, item, q);
    }));
    m_gifs->setDp(dp);
    m_stickers->setDp(dp);
}

EmojiPanel::~EmojiPanel() = default;

void EmojiPanel::setDp(qreal dp)
{
    m_dp = dp;
    m_gifs->setDp(dp);
    m_stickers->setDp(dp);
}

MediaPanel *EmojiPanel::media() const
{
    return m_tab == Tab::Gif ? m_gifs.get() : m_tab == Tab::Sticker ? m_stickers.get() : nullptr;
}

MediaPanel *EmojiPanel::panel(Klipy::Kind kind) const
{
    return kind == Klipy::Kind::Gif ? m_gifs.get() : m_stickers.get();
}

void EmojiPanel::select(Tab t)
{
    if (t == m_tab) return;
    if (MediaPanel *m = media()) m->close();
    m_tab = t;
    s_lastTab = t;
    m_flinging = false;
    if (MediaPanel *m = media()) m->open();
    if (m_actions.update) m_actions.update();
}

void EmojiPanel::searched(Klipy::Kind kind, const QString &q)
{
    select(kind == Klipy::Kind::Gif ? Tab::Gif : Tab::Sticker);
    if (MediaPanel *m = media()) m->search(q);
}

void EmojiPanel::close()
{
    m_gifs->close();
    m_stickers->close();
}

void EmojiPanel::say(Klipy::Kind kind, const QString &s, int ms)
{
    panel(kind)->say(s, ms);
}

qreal EmojiPanel::scroll() const
{
    if (MediaPanel *m = media()) return m->scrollY();
    return m_scrollY;
}

void EmojiPanel::setScroll(qreal y)
{
    if (MediaPanel *m = media()) m->setScrollY(y);
    else m_scrollY = std::max<qreal>(0, std::min(y, maxScroll()));
}

qreal EmojiPanel::maxScrollNow() const
{
    if (MediaPanel *m = media()) return m->maxScroll();
    return maxScroll();
}

void EmojiPanel::open(const QString &emojiFile, const QStringList &recent)
{
    if (m_all.isEmpty()) {
        static const QHash<QString, QString> icons = {
            { "smileys", "😀" }, { "people", "👋" }, { "nature", "🐻" }, { "food", "🍔" }, { "travel", "🚗" },
            { "activities", "⚽" }, { "objects", "💡" }, { "symbols", "❤" }, { "flags", "🏁" },
        };
        QFile f(emojiFile);
        if (f.open(QIODevice::ReadOnly)) {
            QList<QPair<QString, QStringList>> groups;
            for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'))) {
                if (line.startsWith(QLatin1Char('#'))) continue;
                const QStringList parts = line.split(QLatin1Char('\t'));
                if (parts.size() != 3) continue;
                if (!canDraw(parts[2])) continue;
                if (groups.isEmpty() || groups.last().first != parts[0])
                    groups.append({ parts[0], {} });
                groups.last().second << parts[2];
            }
            for (const auto &g : groups)
                if (!g.second.isEmpty())
                    m_all << Category{ g.first, icons.value(g.first, g.second.first()), g.second };
        }
    }
    m_categories = QList<Category>{ Category{ QStringLiteral("recent"), QStringLiteral("🕘"), recent } } + m_all;
    m_flinging = false;
    m_scrollY = 0;
    relayout();
    if (MediaPanel *m = media()) m->open();
}

bool EmojiPanel::canDraw(const QString &emoji)
{
    // One font, so one QFontMetrics for every thread that asks.
    static QMutex lock;
    QMutexLocker locker(&lock);
    static const QFontMetrics metrics(emojiFont(32));
    for (const uint cp : emoji.toUcs4()) {
        // Joiners, variation selectors, skin tones and keycaps: not glyphs of their own.
        if (cp == 0x200D || cp == 0xFE0F || cp == 0xFE0E || cp == 0x20E3 || (cp >= 0x1F3FB && cp <= 0x1F3FF)
                || (cp >= 0xE0020 && cp <= 0xE007F))
            continue;
        if (!metrics.inFontUcs4(cp))
            return false;
    }
    return true;
}

void EmojiPanel::layout(qreal top, const QRectF &rows, const QRectF &bar)
{
    const qreal h = std::round(rows.height() * 0.2);
    m_strip = QRectF(rows.left(), top, rows.width(), h);
    m_grid = QRectF(rows.left(), m_strip.bottom(), rows.width(), rows.bottom() - m_strip.bottom());
    m_mediaGrid = QRectF(rows.left(), m_strip.bottom(), rows.width(), bar.bottom() - m_strip.bottom());
    m_bar = bar;
    m_gifs->layout(m_mediaGrid);
    m_stickers->layout(m_mediaGrid);
    relayout();
}

void EmojiPanel::relayout()
{
    if (m_grid.width() <= 0) return;
    m_cols = std::max(6, int(m_grid.width() / (m_grid.height() / 3.2)));
    m_cell = m_grid.width() / m_cols;
    const qreal lineH = cellHeight();
    m_starts.resize(m_categories.size());
    qreal y = 0;
    for (int i = 0; i < m_categories.size(); ++i) {
        m_starts[i] = y;
        y += ((m_categories[i].emoji.size() + m_cols - 1) / m_cols) * lineH;
    }
    m_contentHeight = y;
    m_scrollY = std::max<qreal>(0, std::min(m_scrollY, maxScroll()));
}

qreal EmojiPanel::cellHeight() const { return std::min(m_cell, m_grid.height() / 3); }
qreal EmojiPanel::maxScroll() const { return std::max<qreal>(0, m_contentHeight - m_grid.height()); }

// The strip: ABC, three tabs, then the search field.
QRectF EmojiPanel::abcRect() const
{
    const qreal pad = 2 * m_dp;
    return QRectF(m_strip.left() + pad, m_strip.top() + pad,
                  std::min(m_strip.width() * 0.15, 72 * m_dp) - pad, m_strip.height() - 2 * pad);
}
qreal EmojiPanel::tabWidth() const { return std::min(m_strip.width() * 0.15, 70 * m_dp); }
QRectF EmojiPanel::topTab(int i) const
{
    const qreal l = abcRect().right() + 4 * m_dp;
    return QRectF(l + i * tabWidth(), m_strip.top(), tabWidth(), m_strip.height());
}
QRectF EmojiPanel::searchRect() const
{
    const qreal pad = m_strip.height() * 0.16;
    const qreal l = topTab(2).right() + 8 * m_dp;
    return QRectF(l, m_strip.top() + pad, m_strip.right() - 8 * m_dp - l, m_strip.height() - 2 * pad);
}
QRectF EmojiPanel::clearRect() const
{
    const QRectF s = searchRect();
    return QRectF(s.right() - s.height() * 1.4, s.top(), s.height() * 1.4, s.height());
}

// The bottom row: the tabs, backspace.
QRectF EmojiPanel::backRect() const
{
    return QRectF(m_bar.right() - m_bar.width() * 0.15, m_bar.top(), m_bar.width() * 0.15, m_bar.height());
}
QRectF EmojiPanel::tabRect(int i) const
{
    const qreal l = m_bar.left(), r = backRect().left();
    const qreal w = (r - l) / std::max(1, int(m_categories.size()));
    return QRectF(l + i * w, m_bar.top(), w, m_bar.height());
}

void EmojiPanel::draw(QPainter &p, Painter &painter, Style style,
                      const std::function<void(QPainter &, const QString &, const QRectF &)> &label)
{
    const QRectF abc = abcRect();
    p.drawImage(abc.topLeft(), painter.cap(int(abc.width()), int(abc.height()), m_pressed == Abc));
    label(p, QStringLiteral("ABC"), abc);
    drawStrip(p, painter, style);
    if (MediaPanel *m = media()) {
        m->draw(p, m_font);
        return;
    }
    // The grid, clipped to its area.
    p.save();
    p.setClipRect(m_grid);
    const qreal lineH = cellHeight();
    p.setFont(emojiFont(lineH * 0.62));
    p.setPen(Qt::white);
    for (int ci = 0; ci < m_categories.size(); ++ci) {
        const Category &cat = m_categories[ci];
        const qreal top = m_grid.top() + m_starts[ci] - m_scrollY;
        const int lines = (cat.emoji.size() + m_cols - 1) / m_cols;
        if (top > m_grid.bottom() || top + lines * lineH < m_grid.top()) continue;
        for (int i = 0; i < cat.emoji.size(); ++i) {
            const qreal y = top + (i / m_cols) * lineH;
            if (y + lineH < m_grid.top() || y > m_grid.bottom()) continue;
            const qreal x = m_grid.left() + (i % m_cols) * m_cell;
            p.drawText(QRectF(x, y, m_cell, lineH), Qt::AlignCenter, cat.emoji[i]);
        }
        // A hairline between categories.
        if (ci > 0)
            p.fillRect(QRectF(m_grid.left() + 8 * m_dp, top - 0.5 * m_dp, m_grid.width() - 16 * m_dp, m_dp),
                       QColor(255, 255, 255, 70));
    }
    p.restore();

    // The bottom row, as keys.
    const QRectF back = backRect();
    p.drawImage(back.topLeft(), painter.cap(int(back.width()), int(back.height()), m_pressed == Back));
    painter.icon(p, Painter::Icon::Backspace, back.center().x(), back.center().y(),
                 std::min(back.width(), back.height()) * 0.42);
    const int current = currentCategory();
    const QRectF tabs(m_bar.left(), m_bar.top(), back.left() - m_bar.left(), m_bar.height());
    p.drawImage(tabs.topLeft(), painter.cap(int(tabs.width()), int(tabs.height()), false));
    if (m_categories.isEmpty()) return;
    p.setFont(emojiFont(std::min(tabRect(0).width(), m_bar.height()) * 0.42));
    for (int i = 0; i < m_categories.size(); ++i) {
        const QRectF r = tabRect(i);
        p.setOpacity(i == current ? 1.0 : 120 / 255.0);
        p.drawText(r, Qt::AlignCenter, m_categories[i].icon);
        p.setOpacity(1.0);
        if (i == current) {
            p.setPen(Qt::NoPen);
            p.setBrush(accent(style));
            p.drawRoundedRect(QRectF(r.center().x() - r.width() * 0.3, r.bottom() - 6.5 * m_dp,
                                     r.width() * 0.6, 2 * m_dp), m_dp, m_dp);
            p.setPen(Qt::white);
        }
    }
}

void EmojiPanel::drawStrip(QPainter &p, Painter &painter, Style style)
{
    const QColor lit = style == Style::White ? QColor(Qt::white) : accent(style);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    const Tab tabs[] = { Tab::Emoji, Tab::Gif, Tab::Sticker };
    for (int i = 0; i < 3; ++i) {
        const QRectF r = topTab(i);
        const bool on = tabs[i] == m_tab;
        const int alpha = on ? 255 : 130;
        if (tabs[i] == Tab::Sticker) {
            painter.icon(p, Painter::Icon::Sticker, r.center().x(), r.center().y(), r.height() * 0.58,
                         QColor(255, 255, 255, alpha));
        } else {
            const QString s = tabs[i] == Tab::Emoji ? QStringLiteral("Emoji") : QStringLiteral("GIF");
            QFont f = m_font;
            f.setPixelSize(std::max(1, int(std::min(r.height() * 0.42, r.width() * 0.3))));
            p.setFont(f);
            p.setPen(QColor(255, 255, 255, alpha));
            p.drawText(r, Qt::AlignCenter, s);
        }
        if (on) {
            p.setPen(Qt::NoPen);
            p.setBrush(lit);
            p.drawRoundedRect(QRectF(r.center().x() - r.width() * 0.28, r.bottom() - 3.5 * m_dp, r.width() * 0.56, 2 * m_dp),
                              m_dp, m_dp);
        }
    }
    MediaPanel *m = media();
    if (!m) {
        // A hairline under the strip, as between categories.
        p.fillRect(QRectF(m_strip.left() + 8 * m_dp, m_strip.bottom() - 0.5 * m_dp, m_strip.width() - 16 * m_dp, m_dp),
                   QColor(255, 255, 255, 70));
        p.restore();
        return;
    }
    // The search field: "Search KLIPY", as KLIPY asks, or what was searched for and an x.
    const QRectF s = searchRect();
    p.setPen(Qt::NoPen);
    p.setBrush(m_pressed == Search ? QColor(255, 255, 255, 70) : QColor(255, 255, 255, 38));
    p.drawRoundedRect(s, s.height() / 2, s.height() / 2);
    painter.icon(p, Painter::Icon::Search, s.left() + s.height() * 0.6, s.center().y(), s.height() * 0.56,
                 QColor(255, 255, 255, 170));
    const QString q = m->query();
    QFont f = m_font;
    f.setPixelSize(std::max(1, int(s.height() * 0.46)));
    p.setFont(f);
    const QFontMetricsF fm(f);
    const qreal textLeft = s.left() + s.height() * 1.1;
    const qreal room = (q.isEmpty() ? s.right() : clearRect().left()) - textLeft - 4 * m_dp;
    QString t = q.isEmpty() ? QStringLiteral("Search KLIPY") : q;
    while (t.length() > 1 && fm.horizontalAdvance(t) > room) t = t.left(t.length() - 2) + QStringLiteral("…");
    p.setPen(QColor(255, 255, 255, q.isEmpty() ? 140 : 255));
    p.drawText(QPointF(textLeft, s.center().y() + (fm.ascent() - fm.descent()) / 2), t);
    if (!q.isEmpty()) {
        const QRectF x = clearRect();
        painter.icon(p, Painter::Icon::Close, x.center().x(), x.center().y(), s.height() * 0.42, QColor(255, 255, 255, 200));
    }
    p.restore();
}

int EmojiPanel::currentCategory() const
{
    int cur = 0;
    for (int i = 0; i < m_starts.size(); ++i)
        if (m_starts[i] <= m_scrollY + 1 && !m_categories[i].emoji.isEmpty()) cur = i;
    // Scrolled to the very end: the last tab, even if its category is short.
    if (m_scrollY >= maxScroll() - 1 && maxScroll() > 0) cur = m_categories.size() - 1;
    return cur;
}

void EmojiPanel::down(qreal x, qreal y)
{
    m_flinging = false;
    m_downX = x; m_downY = y; m_lastY = y;
    m_dragging = false;
    m_velocity = 0;
    m_moveClock.start();
    const QPointF pt(x, y);
    MediaPanel *m = media();
    if (m_strip.contains(pt)) {
        m_downIn = x < abcRect().right() + 2 * m_dp ? Abc : (m && searchRect().contains(pt)) ? Search : Strip;
    } else if (m) {
        m_downIn = m_mediaGrid.contains(pt) ? Grid : None;
    } else {
        m_downIn = m_grid.contains(pt) ? Grid : backRect().contains(pt) ? Back : m_bar.contains(pt) ? Tabs : None;
    }
    m_pressed = m_downIn;
    if (m_downIn == Grid && m) m->pressed = m->itemAt(x, y);
    if (m_downIn == Back && m_actions.backspace) m_actions.backspace();
    if (m_actions.update) m_actions.update();
}

void EmojiPanel::move(qreal x, qreal y)
{
    Q_UNUSED(x);
    if (m_downIn == Grid) {
        if (!m_dragging && std::abs(y - m_downY) > SLOP_DP * m_dp) {
            m_dragging = true;
            if (MediaPanel *m = media()) m->pressed = -1;
        }
        if (m_dragging) {
            const qreal ms = std::max<qreal>(1, m_moveClock.restart());
            // Smoothed: the last few moves count, not just the very last one.
            m_velocity = 0.6 * ((m_lastY - y) / ms) + 0.4 * m_velocity;
            setScroll(scroll() - (y - m_lastY));
            if (m_actions.update) m_actions.update();
        }
    }
    m_lastY = y;
}

void EmojiPanel::up(qreal x, qreal y)
{
    const QPointF pt(x, y);
    MediaPanel *m = media();
    switch (m_downIn) {
    case Grid:
        if (m_dragging) {
            if (std::abs(m_velocity) > 0.15) { m_flinging = true; m_flingClock.start(); }
        } else if (m) {
            m->tap(m_downX, m_downY);
        } else {
            tapGrid(m_downX, m_downY);
        }
        break;
    case Abc:
        if (m_strip.contains(pt) && x < abcRect().right() + 2 * m_dp && m_actions.letters) m_actions.letters();
        break;
    case Strip:
        for (int i = 0; i < 3; ++i)
            if (topTab(i).contains(pt)) { select(i == 0 ? Tab::Emoji : i == 1 ? Tab::Gif : Tab::Sticker); break; }
        break;
    case Search:
        if (m) {
            if (!m->query().isEmpty() && clearRect().contains(pt)) m->search(QString());
            else if (searchRect().contains(pt) && m_actions.search) m_actions.search(m->kind(), m->query());
        }
        break;
    case Tabs:
        if (m) break;
        for (int i = 0; i < m_categories.size(); ++i)
            if (tabRect(i).contains(pt)) { jumpTo(i); break; }
        break;
    default:
        break;
    }
    cancel();
}

void EmojiPanel::cancel()
{
    m_pressed = None;
    m_dragging = false;
    if (MediaPanel *m = media()) m->pressed = -1;
    if (m_actions.update) m_actions.update();
}

bool EmojiPanel::animate()
{
    if (!m_flinging) return false;
    const qreal ms = std::max<qreal>(1, m_flingClock.restart());
    const qreal next = scroll() + m_velocity * ms;
    m_velocity *= std::pow(1 - FRICTION, ms);
    setScroll(next);
    if (next <= 0 || next >= maxScrollNow() || std::abs(m_velocity) < 0.02)
        m_flinging = false;
    return true;
}

void EmojiPanel::jumpTo(int i)
{
    m_flinging = false;
    m_scrollY = std::max<qreal>(0, std::min(maxScroll(), m_starts.value(i)));
    if (m_actions.update) m_actions.update();
}

void EmojiPanel::tapGrid(qreal x, qreal y)
{
    const qreal lineH = cellHeight();
    const qreal contentY = y - m_grid.top() + m_scrollY;
    for (int ci = 0; ci < m_categories.size(); ++ci) {
        const Category &cat = m_categories[ci];
        const int lines = (cat.emoji.size() + m_cols - 1) / m_cols;
        if (contentY < m_starts[ci] || contentY >= m_starts[ci] + lines * lineH) continue;
        const int i = int((contentY - m_starts[ci]) / lineH) * m_cols + int((x - m_grid.left()) / m_cell);
        if (i >= 0 && i < cat.emoji.size() && m_actions.emoji)
            m_actions.emoji(cat.emoji[i]);
        return;
    }
}

} // namespace Pre
