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

#include "preklipy.h"
#include "prepainter.h"

#include <QElapsedTimer>
#include <QFont>
#include <QList>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <memory>

class QPainter;

class TestPreKeyboard;

namespace Pre {

class MediaPanel;

/*!
 * The emoji page: every emoji, by Unicode's categories, in a grid that scrolls;
 * a row of category tabs along the bottom, then backspace. Tapping a tab jumps to
 * its category, and the tab lights as you scroll.
 *
 * Along the top, where the suggestion bar was: ABC, then Emoji, GIF and a
 * sticker. The other two are KLIPY's (MediaPanel): there a "Search KLIPY" field
 * follows the tabs, and the GIFs take everything below, down to the bottom of the
 * keys; ABC is the only key left.
 *
 * The keyboard view owns it, gives it the keys' area, and hands it every touch
 * while it shows.
 */
class EmojiPanel
{
    friend class ::TestPreKeyboard;

public:
    struct Actions
    {
        std::function<void(const QString &)> emoji;
        std::function<void()> backspace;
        std::function<void()> letters;
        std::function<void()> update;
        //! The search field was tapped: type a search for [kind], starting from [query].
        std::function<void(Klipy::Kind, const QString &)> search;
        //! A GIF or sticker was tapped, found by searching for [query] (empty: trending).
        std::function<void(Klipy::Kind, const Klipy::Item &, const QString &)> media;
    };

    enum Area { None = 0, Grid = 1, Abc = 2, Back = 3, Tabs = 4, Strip = 5, Search = 6 };
    enum class Tab { Emoji, Gif, Sticker };

    EmojiPanel(qreal dp, const Actions &actions);
    ~EmojiPanel();

    void setDp(qreal dp);
    void setFont(const QFont &font) { m_font = font; }
    Tab tab() const { return m_tab; }
    void select(Tab t);
    //! A search was typed for [kind]: its tab, showing what was found.
    void searched(Klipy::Kind kind, const QString &q);
    //! The page is going away: the GIFs stop moving.
    void close();
    //! A word for a moment over the GIFs or stickers.
    void say(Klipy::Kind kind, const QString &s, int ms = 2500);
    //! Fills the page: every category is read once, the recent ones each time it opens.
    void open(const QString &emojiFile, const QStringList &recent);
    //! Whether the device's emoji font draws [emoji]: every code point in it, joiners and selectors aside.
    static bool canDraw(const QString &emoji);
    /*!
     * [top] is where the page starts (the suggestion bar's place, if it shows),
     * [rows] the top three rows of keys, [bar] the bottom row.
     */
    //! [rowHeight]: a row of keys, as laid out before rounding to whole pixels: what the strip and emoji are sized from.
    void layout(qreal top, const QRectF &rows, const QRectF &bar, qreal rowHeight);
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
    MediaPanel *media() const;
    MediaPanel *panel(Klipy::Kind kind) const;
    //! The scroll of whichever grid shows.
    qreal scroll() const;
    void setScroll(qreal y);
    qreal maxScrollNow() const;
    void drawStrip(QPainter &p, Painter &painter, Style style);
    qreal edge() const;
    qreal gap() const;
    QRectF item() const;
    qreal tabText() const;
    qreal tabWidth(int i) const;
    QRectF topTab(int i) const;
    //! The tabs above the grid: Emoji, and GIFs and stickers when KLIPY has a key.
    int tabCount() const;
    QRectF searchRect() const;
    QRectF clearRect() const;
    QRectF abcRect() const;
    QRectF backRect() const;
    QRectF tabRect(int i) const;
    int currentCategory() const;
    void jumpTo(int i);
    void tapGrid(qreal x, qreal y);

    qreal m_dp;
    Actions m_actions;
    QFont m_font;
    Tab m_tab;
    std::unique_ptr<MediaPanel> m_gifs;
    std::unique_ptr<MediaPanel> m_stickers;
    //! The tab shown last, shown again next time.
    static Tab s_lastTab;
    QList<Category> m_all;
    QList<Category> m_categories;
    QRectF m_grid, m_bar;
    //! The GIFs' and stickers' grid: under the strip, all the way down.
    QRectF m_mediaGrid;
    //! The tabs (and on KLIPY's, the search field) above the grid.
    QRectF m_strip;
    qreal m_rowHeight = 0;
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
