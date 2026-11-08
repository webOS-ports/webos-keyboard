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

#ifndef PRE_EMOJIPANEL_H
#define PRE_EMOJIPANEL_H

#include "prepainter.h"

#include <QElapsedTimer>
#include <QList>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

class QPainter;

namespace Pre {

/*!
 * The emoji page: every emoji, by Unicode's categories, in a grid that scrolls;
 * a row of category tabs along the bottom between ABC and backspace, as on the
 * Sym page's bottom row. Tapping a tab jumps to its category, and the tab lights
 * as you scroll.
 *
 * The keyboard view owns it, gives it the keys' area, and hands it every touch
 * while it shows.
 */
class EmojiPanel
{
public:
    struct Actions
    {
        std::function<void(const QString &)> emoji;
        std::function<void()> backspace;
        std::function<void()> letters;
        std::function<void()> update;
    };

    enum Area { None = 0, Grid = 1, Abc = 2, Back = 3, Tabs = 4 };

    EmojiPanel(qreal dp, const Actions &actions) : m_dp(dp), m_actions(actions) {}

    void setDp(qreal dp) { m_dp = dp; }
    //! Fills the page: every category is read once, the recent ones each time it opens.
    void open(const QString &emojiFile, const QStringList &recent);
    //! Whether the device's emoji font draws [emoji]: every code point in it, joiners and selectors aside.
    static bool canDraw(const QString &emoji);
    void layout(const QRectF &grid, const QRectF &bar);
    void draw(QPainter &p, Painter &painter, Style style, const std::function<void(QPainter &, const QString &, const QRectF &)> &label);

    void down(qreal x, qreal y);
    void move(qreal x, qreal y);
    void up(qreal x, qreal y);
    void cancel();
    //! While a fling runs: one step of it; false once it has stopped.
    bool animate();

    int pressed() const { return m_pressed; }

private:
    struct Category { QString id; QString icon; QStringList emoji; };

    void relayout();
    qreal cellHeight() const;
    qreal maxScroll() const;
    QRectF abcRect() const;
    QRectF backRect() const;
    QRectF tabRect(int i) const;
    int currentCategory() const;
    void jumpTo(int i);
    void tapGrid(qreal x, qreal y);

    qreal m_dp;
    Actions m_actions;
    QList<Category> m_all;
    QList<Category> m_categories;
    QRectF m_grid, m_bar;
    int m_cols = 8;
    qreal m_cell = 48;
    QVector<qreal> m_starts;
    qreal m_contentHeight = 0;
    qreal m_scrollY = 0;

    // Touch, and the fling after it.
    qreal m_downX = 0, m_downY = 0, m_lastY = 0;
    bool m_dragging = false;
    int m_downIn = None;
    int m_pressed = None;
    QElapsedTimer m_moveClock;
    qreal m_velocity = 0;      // px per ms, positive scrolling down the content
    bool m_flinging = false;
    QElapsedTimer m_flingClock;
};

} // namespace Pre

#endif // PRE_EMOJIPANEL_H
