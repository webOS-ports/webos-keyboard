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

#ifndef PRE_PAINTER_H
#define PRE_PAINTER_H

#include <QColor>
#include <QHash>
#include <QImage>
#include <QPainterPath>
#include <QRectF>

class QPainter;

namespace Pre {

//! Orange or White: the colour of the printed alternates, the orange key and the Sym page's digits.
enum class Style { Orange, White };
QColor accent(Style style);

//! The orange key's state: off, for the next key, or locked on.
enum class Alt { Off, Once, Lock };

/*!
 * The Pre's key art, drawn in code: glossy near-black caps in slightly larger
 * dark recesses on a charcoal deck. The recipe is webos-keyboard's
 * tools/gen_pre_keys.py, which drew it as 48 px nine-patches; here every
 * measurement is in dp, so it is drawn sharp at any density and any key size.
 */
class Painter
{
public:
    explicit Painter(qreal dp) : m_dp(dp) {}

    void setDp(qreal dp) { if (dp != m_dp) { m_dp = dp; clear(); } }

    //! One key cap, [w] x [h] device px, cached by size and state.
    const QImage &cap(int w, int h, bool pressed);
    //! The face of the cap drawn for [key]: what a key's printing has to stay inside.
    QPainterPath capShape(const QRectF &key) const;
    //! That face's rectangle, and the radius of its corners.
    QRectF capFace(const QRectF &key) const;
    qreal capRadius(const QRectF &face) const;
    //! The deck behind the keys, and the dark-to-light stripes along its top edge.
    void deck(QPainter &p, const QRectF &r) const;

    //! ShiftOpen: shift off in the White style, an outline, since its "on" is white too.
    enum class Icon { Shift, ShiftOpen, ShiftOn, ShiftLock, Backspace, Return, Globe, Emoji, Cursor, Gear, Mic, Paste, Sticker, Search, Close };
    //! Draws an icon [size] px square, centred on ([cx], [cy]); an invalid [color] means its usual one.
    void icon(QPainter &p, Icon icon, qreal cx, qreal cy, qreal size, const QColor &color = QColor()) const;
    //! The Pre's orange key: a printed square, lit with a rim while active and a heavier rim when locked.
    void orange(QPainter &p, const QRectF &key, Style style, Alt state) const;

    void clear() { m_caps.clear(); }

private:
    QImage drawCap(int w, int h, bool pressed) const;

    qreal m_dp;
    QHash<quint64, QImage> m_caps;
};

} // namespace Pre

#endif // PRE_PAINTER_H
