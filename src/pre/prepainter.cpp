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

#include "prepainter.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace Pre {

namespace {

//! How far a cap's face sits inside its key, in dp: the dark well shows around it.
const qreal CAP_PAD = 2.5;

qreal faceRadius(const QRectF &cap, qreal dp)
{
    return std::min(8 * dp, std::min(cap.width(), cap.height()) / 2);
}

} // namespace

QColor accent(Style style)
{
    return style == Style::White ? QColor(Qt::white) : QColor(0xF2, 0x40, 0x2E);
}

const QImage &Painter::cap(int w, int h, bool pressed)
{
    const quint64 id = (quint64(quint32(w)) << 32) | (quint64(quint32(h)) << 1) | (pressed ? 1 : 0);
    auto it = m_caps.find(id);
    if (it == m_caps.end())
        it = m_caps.insert(id, drawCap(w, h, pressed));
    return it.value();
}

QRectF Painter::capFace(const QRectF &key) const
{
    const qreal pad = CAP_PAD * m_dp;
    return key.adjusted(pad, pad, -pad, -pad);
}

qreal Painter::capRadius(const QRectF &face) const { return faceRadius(face, m_dp); }

QPainterPath Painter::capShape(const QRectF &key) const
{
    const QRectF cap = capFace(key);
    const qreal r = capRadius(cap);
    QPainterPath path;
    path.addRoundedRect(cap, r, r);
    return path;
}

QImage Painter::drawCap(int w, int h, bool pressed) const
{
    const qreal dp = m_dp;
    QImage b(std::max(1, w), std::max(1, h), QImage::Format_ARGB32_Premultiplied);
    b.fill(Qt::transparent);
    QPainter c(&b);
    c.setRenderHint(QPainter::Antialiasing);
    c.setPen(Qt::NoPen);

    // The recess the cap sits in.
    const qreal wellPad = 1.5 * dp;
    c.setBrush(QColor(8, 9, 10));
    c.drawRoundedRect(QRectF(wellPad, wellPad, w - 2 * wellPad, h - 2 * wellPad), 9 * dp, 9 * dp);

    // The cap: a glow under its top edge, darkest through the middle, lifting a
    // little towards the bottom rim. Pressed, every stop is lighter.
    const qreal capPad = CAP_PAD * dp;
    const QRectF cap(capPad, capPad, w - 2 * capPad, h - 2 * capPad);
    const qreal capR = faceRadius(cap, dp);
    const int lift = pressed ? 26 : 0;
    auto col = [lift](int r, int g, int bl) {
        return QColor(std::min(255, r + lift), std::min(255, g + lift), std::min(255, bl + lift));
    };
    const qreal half = 12 * dp;
    auto fromTop = [h](qreal y) { return std::min(y, h / 2.0) / h; };
    auto fromBottom = [h](qreal y) { return std::max(h - y, h / 2.0) / h; };
    QLinearGradient g(0, 0, 0, h);
    g.setColorAt(0, col(64, 71, 76));
    g.setColorAt(fromTop(0.36 * half), col(40, 46, 51));
    g.setColorAt(fromTop(0.8 * half), col(22, 27, 30));
    g.setColorAt(fromBottom(0.8 * half), col(20, 24, 27));
    g.setColorAt(fromBottom(0.32 * half), col(25, 29, 32));
    g.setColorAt(1, col(38, 42, 46));
    c.setBrush(g);
    c.drawRoundedRect(cap, capR, capR);

    // A thin specular line just inside the top edge, short of the corners.
    c.save();
    QPainterPath clip;
    clip.addRoundedRect(cap, capR, capR);
    c.setClipPath(clip);
    c.setBrush(QColor(220, 228, 232, pressed ? 150 : 235));
    const qreal hy = capPad + 0.7 * dp;
    c.drawRoundedRect(QRectF(capPad + 4 * dp, hy, w - 2 * capPad - 8 * dp, 0.55 * dp), 0.5 * dp, 0.5 * dp);
    c.restore();

    // A faint lighter rim along the bottom edge, fading in over its last 7 dp.
    QImage rim(b.size(), QImage::Format_ARGB32_Premultiplied);
    rim.fill(Qt::transparent);
    {
        QPainter rc(&rim);
        rc.setRenderHint(QPainter::Antialiasing);
        rc.setBrush(Qt::NoBrush);
        rc.setPen(QPen(QColor(120, 128, 134, 70), 0.45 * dp));
        rc.drawRoundedRect(cap, capR, capR);
        QLinearGradient fade(0, h - 7.2 * dp, 0, h);
        fade.setColorAt(0, Qt::transparent);
        fade.setColorAt(1, Qt::black);
        rc.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        rc.setPen(Qt::NoPen);
        rc.setBrush(fade);
        rc.drawRect(QRectF(0, 0, w, h));
    }
    c.drawImage(0, 0, rim);
    return b;
}

void Painter::deck(QPainter &p, const QRectF &r) const
{
    QLinearGradient g(0, r.top(), 0, r.bottom());
    g.setColorAt(0, QColor(74, 81, 86));
    g.setColorAt(1, QColor(58, 64, 68));
    p.fillRect(r, g);
    const qreal stripe = std::max<qreal>(1, std::round(m_dp));
    const QColor stripes[] = { QColor(14, 16, 17), QColor(30, 34, 36), QColor(86, 94, 99) };
    for (int i = 0; i < 3; ++i)
        p.fillRect(QRectF(r.left(), r.top() + i * stripe, r.width(), stripe), stripes[i]);
}

namespace {

QPolygonF poly(std::initializer_list<int> pts)
{
    QPolygonF out;
    auto it = pts.begin();
    while (it != pts.end()) {
        const int x = *it++;
        const int y = *it++;
        out << QPointF(x, y);
    }
    return out;
}

} // namespace

void Painter::icon(QPainter &p, Icon icon, qreal cx, qreal cy, qreal size, const QColor &color) const
{
    static const QPolygonF up = poly({ 32, 6, 56, 32, 42, 32, 42, 50, 22, 50, 22, 32, 8, 32 });
    static const QPolygonF left = poly({ 4, 32, 28, 10, 28, 24, 60, 24, 60, 40, 28, 40, 28, 54 });
    static const QPolygonF ret = poly({ 46, 6, 60, 6, 60, 44, 28, 44, 28, 58, 4, 37, 28, 16, 28, 30, 46, 30 });

    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(std::round(cx - size / 2), std::round(cy - size / 2));
    p.scale(size / 64.0, size / 64.0);
    // Shift, when the next letter (or every letter) will be a capital: the Pre's orange, unless [color] says otherwise.
    const QColor c = color.isValid() ? color
        : (icon == Icon::ShiftOn || icon == Icon::ShiftLock) ? accent(Style::Orange) : QColor(Qt::white);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    auto stroke = [&](qreal width, Qt::PenCapStyle capStyle = Qt::FlatCap, Qt::PenJoinStyle join = Qt::MiterJoin) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(c, width, Qt::SolidLine, capStyle, join));
    };
    auto fill = [&]() { p.setPen(Qt::NoPen); p.setBrush(c); };
    switch (icon) {
    case Icon::Shift:
    case Icon::ShiftOn:
        p.drawPolygon(up);
        break;
    case Icon::ShiftOpen:
        stroke(4, Qt::FlatCap, Qt::MiterJoin);
        p.drawPolygon(up);
        break;
    case Icon::ShiftLock:
        p.drawPolygon(up);
        p.drawRect(QRectF(18, 54, 28, 5));
        break;
    case Icon::Backspace:
        p.drawPolygon(left);
        break;
    case Icon::Return:
        p.drawPolygon(ret);
        break;
    case Icon::Globe:
        stroke(4);
        p.drawEllipse(QPointF(32, 32), 24, 24);
        p.drawEllipse(QRectF(20, 8, 24, 48));
        p.drawLine(QPointF(32, 8), QPointF(32, 56));
        p.drawLine(QPointF(8, 32), QPointF(56, 32));
        p.drawLine(QPointF(13, 20), QPointF(51, 20));
        p.drawLine(QPointF(13, 44), QPointF(51, 44));
        break;
    case Icon::Emoji:
        stroke(4.5);
        p.drawEllipse(QPointF(32, 32), 25, 25);
        // Android's arc: from 20 degrees, 140 degrees clockwise (screen y down).
        p.drawArc(QRectF(18, 22, 28, 25), -20 * 16, -140 * 16);
        fill();
        p.drawEllipse(QPointF(23.5, 25), 3.6, 3.6);
        p.drawEllipse(QPointF(40.5, 25), 3.6, 3.6);
        break;
    case Icon::Cursor:
        // Long-pressed space: the cursor follows the finger, any way it goes.
        p.drawPolygon(poly({ 4, 32, 16, 22, 16, 42 }));
        p.drawPolygon(poly({ 60, 32, 48, 22, 48, 42 }));
        p.drawPolygon(poly({ 32, 4, 22, 16, 42, 16 }));
        p.drawPolygon(poly({ 32, 60, 22, 48, 42, 48 }));
        p.drawRect(QRectF(15, 30, 34, 4));
        p.drawRect(QRectF(30, 15, 4, 34));
        break;
    case Icon::Gear:
        // Return's corner on the Sym page: Text Assist.
        for (int i = 0; i < 8; ++i) {
            p.save();
            p.translate(32, 32);
            p.rotate(i * 45);
            p.translate(-32, -32);
            p.drawRoundedRect(QRectF(26, 3, 12, 13), 2, 2);
            p.restore();
        }
        stroke(10);
        p.drawEllipse(QPointF(32, 32), 15, 15);
        break;
    case Icon::Mic:
        // The period's corner: dictation.
        p.drawRoundedRect(QRectF(22, 2, 20, 36), 10, 10);
        stroke(5, Qt::RoundCap);
        p.drawArc(QRectF(13, 16, 38, 32), 180 * 16, 180 * 16);
        p.drawLine(QPointF(32, 48), QPointF(32, 58));
        p.drawLine(QPointF(21, 60), QPointF(43, 60));
        break;
    case Icon::Paste: {
        // The bar's paste: a clipboard with a page on it, its corner turned down.
        stroke(4, Qt::FlatCap, Qt::RoundJoin);
        p.drawRoundedRect(QRectF(10, 9, 44, 52), 6, 6);
        QPainterPath page;
        page.moveTo(20, 22); page.lineTo(36, 22); page.lineTo(44, 30); page.lineTo(44, 52); page.lineTo(20, 52);
        page.closeSubpath();
        p.drawPath(page);
        QPainterPath corner;
        corner.moveTo(36, 22); corner.lineTo(36, 30); corner.lineTo(44, 30);
        p.drawPath(corner);
        stroke(3.5, Qt::RoundCap, Qt::RoundJoin);
        p.drawLine(QPointF(25, 37), QPointF(39, 37));
        p.drawLine(QPointF(25, 44), QPointF(35, 44));
        fill();
        p.drawRoundedRect(QRectF(22, 3, 20, 12), 3, 3);
        break;
    }
    case Icon::Sticker: {
        // The emoji page's sticker tab: a smiling sticker, its corner peeling up.
        stroke(4.5, Qt::RoundCap, Qt::RoundJoin);
        QPainterPath edge;
        edge.moveTo(16, 6); edge.lineTo(48, 6); edge.quadTo(58, 6, 58, 16); edge.lineTo(58, 38);
        edge.lineTo(38, 58); edge.lineTo(16, 58); edge.quadTo(6, 58, 6, 48); edge.lineTo(6, 16);
        edge.quadTo(6, 6, 16, 6); edge.closeSubpath();
        p.drawPath(edge);
        QPainterPath peel;
        peel.moveTo(58, 38); peel.lineTo(44, 38); peel.quadTo(38, 38, 38, 44); peel.lineTo(38, 58);
        p.drawPath(peel);
        // The smile: the bottom of an ellipse, from 25 to 135 degrees clockwise of three o'clock.
        p.drawArc(QRectF(18, 22, 24, 22), -25 * 16, -110 * 16);
        fill();
        p.drawEllipse(QPointF(23, 23), 3.4, 3.4);
        p.drawEllipse(QPointF(38, 23), 3.4, 3.4);
        break;
    }
    case Icon::Search:
        // The search field's magnifying glass.
        stroke(6, Qt::RoundCap);
        p.drawEllipse(QPointF(27, 27), 17, 17);
        stroke(8, Qt::RoundCap);
        p.drawLine(QPointF(40, 40), QPointF(56, 56));
        break;
    case Icon::Close:
        // Clearing a search.
        stroke(7, Qt::RoundCap);
        p.drawLine(QPointF(12, 12), QPointF(52, 52));
        p.drawLine(QPointF(52, 12), QPointF(12, 52));
        break;
    }
    p.restore();
}

void Painter::orange(QPainter &p, const QRectF &key, Style style, Alt state) const
{
    const qreal w = key.width() * 0.5;
    const qreal h = std::min(w * 1.3, key.height() * 0.62);
    QRectF r(key.center().x() - w / 2, key.center().y() - h / 2, w, h);
    const bool on = state != Alt::Off;
    QColor top, bottom;
    if (style == Style::White) {
        top = Qt::white;
        bottom = on ? QColor(0xE8, 0xEC, 0xEE) : QColor(0xD0, 0xD4, 0xD6);
    } else {
        top = on ? QColor(0xFF, 0x7A, 0x60) : QColor(0xFF, 0x5A, 0x40);
        bottom = on ? QColor(0xFF, 0x4A, 0x30) : QColor(0xE0, 0x30, 0x1E);
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient g(0, r.top(), 0, r.bottom());
    g.setColorAt(0, top);
    g.setColorAt(1, bottom);
    p.setPen(Qt::NoPen);
    p.setBrush(g);
    const qreal rad = w * 0.16;
    p.drawRoundedRect(r, rad, rad);
    const qreal rim = state == Alt::Lock ? 2 * m_dp : state == Alt::Once ? 1 * m_dp : 0;
    if (rim > 0) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(style == Style::White ? QColor(0x7A, 0x80, 0x85) : QColor(Qt::white), rim));
        r.adjust(rim / 2, rim / 2, -rim / 2, -rim / 2);
        p.drawRoundedRect(r, rad, rad);
    }
    p.restore();
}

} // namespace Pre
