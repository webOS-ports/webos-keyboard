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

#include <QFile>
#include <QFont>
#include <QFontMetricsF>
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
    return f;
}

} // namespace

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
}

void EmojiPanel::layout(const QRectF &grid, const QRectF &bar)
{
    m_grid = grid;
    m_bar = bar;
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

// The bottom row: ABC, the tabs, backspace.
QRectF EmojiPanel::abcRect() const { return QRectF(m_bar.left(), m_bar.top(), m_bar.width() * 0.15, m_bar.height()); }
QRectF EmojiPanel::backRect() const
{
    return QRectF(m_bar.right() - m_bar.width() * 0.15, m_bar.top(), m_bar.width() * 0.15, m_bar.height());
}
QRectF EmojiPanel::tabRect(int i) const
{
    const qreal l = abcRect().right(), r = backRect().left();
    const qreal w = (r - l) / std::max(1, int(m_categories.size()));
    return QRectF(l + i * w, m_bar.top(), w, m_bar.height());
}

void EmojiPanel::draw(QPainter &p, Painter &painter, Style style,
                      const std::function<void(QPainter &, const QString &, const QRectF &)> &label)
{
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
    const QRectF abc = abcRect(), back = backRect();
    p.drawImage(abc.topLeft(), painter.cap(int(abc.width()), int(abc.height()), m_pressed == Abc));
    label(p, QStringLiteral("ABC"), abc);
    p.drawImage(back.topLeft(), painter.cap(int(back.width()), int(back.height()), m_pressed == Back));
    painter.icon(p, Painter::Icon::Backspace, back.center().x(), back.center().y(),
                 std::min(back.width(), back.height()) * 0.42);
    const int current = currentCategory();
    const QRectF tabs(abc.right(), m_bar.top(), back.left() - abc.right(), m_bar.height());
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
    m_downIn = m_grid.contains(pt) ? Grid : abcRect().contains(pt) ? Abc : backRect().contains(pt) ? Back
             : m_bar.contains(pt) ? Tabs : None;
    m_pressed = m_downIn;
    if (m_downIn == Back && m_actions.backspace) m_actions.backspace();
    if (m_actions.update) m_actions.update();
}

void EmojiPanel::move(qreal x, qreal y)
{
    Q_UNUSED(x);
    if (m_downIn == Grid) {
        if (!m_dragging && std::abs(y - m_downY) > SLOP_DP * m_dp) m_dragging = true;
        if (m_dragging) {
            const qreal ms = std::max<qreal>(1, m_moveClock.restart());
            // Smoothed: the last few moves count, not just the very last one.
            m_velocity = 0.6 * ((m_lastY - y) / ms) + 0.4 * m_velocity;
            m_scrollY = std::max<qreal>(0, std::min(maxScroll(), m_scrollY - (y - m_lastY)));
            if (m_actions.update) m_actions.update();
        }
    }
    m_lastY = y;
}

void EmojiPanel::up(qreal x, qreal y)
{
    const QPointF pt(x, y);
    switch (m_downIn) {
    case Grid:
        if (m_dragging) {
            if (std::abs(m_velocity) > 0.15) { m_flinging = true; m_flingClock.start(); }
        } else {
            tapGrid(m_downX, m_downY);
        }
        break;
    case Abc:
        if (abcRect().contains(pt) && m_actions.letters) m_actions.letters();
        break;
    case Tabs:
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
    if (m_actions.update) m_actions.update();
}

bool EmojiPanel::animate()
{
    if (!m_flinging) return false;
    const qreal ms = std::max<qreal>(1, m_flingClock.restart());
    m_scrollY += m_velocity * ms;
    m_velocity *= std::pow(1 - FRICTION, ms);
    if (m_scrollY <= 0 || m_scrollY >= maxScroll() || std::abs(m_velocity) < 0.02) {
        m_scrollY = std::max<qreal>(0, std::min(maxScroll(), m_scrollY));
        m_flinging = false;
    }
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
