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

#include "prekeyboardview.h"
#include "precontroller.h"

#include <QDateTime>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTouchEvent>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Pre {
namespace {

const QColor BAR(36, 40, 43);
const QColor BAR_PRESSED(64, 70, 75);
const QColor BAR_RULE(86, 94, 99);

// webOS's keyboard timings, from LunaCE's TabletKeyboard.cpp (Apache 2.0).
const int FIRST_REPEAT = 350;
const int LETTER_REPEAT = 120;
const int WORD_REPEAT = 275;
const int WORD_DELETE_AFTER = 1500;
const qint64 DOUBLE_TAP = 500;
const int HOLD_FOR_EXTENDED = 400;
// Our own: long enough that a slow tap never trips them.
const int HOLD_FOR_CURSOR = 450;
const int HOLD_FOR_SETTING = 600;
//! Holding a word in the bar, to add it to the dictionary.
const int HOLD_FOR_ADD = 500;
const qreal CURSOR_STEP_X = 12;
//! How far across, in keys, a flick on the space bar goes to change language.
const qreal FLICK_KEYS = 1.2;
//! The language's name on the space bar: shown this long, then fading for this long...
const qint64 FLASH_SHOW = 900;
const qint64 FLASH_FADE = 400;
//! ...from this white to this, just off the key's own black.
const qreal FLASH_ALPHA = 230;
const qreal RESTING_ALPHA = 46;
//! How often the bar's guesses are brought up to date during a swipe.
const qint64 LIVE_EVERY = 50;
//! Prelude's cap height, as a fraction of the font size.
const qreal CAP_HEIGHT = 0.72;
//! How far a letter sits from its key's centre, away from the alternate (off a Pre photo: E, H).
const qreal LETTER_NUDGE = 0.07;
const qint64 NEVER = std::numeric_limits<qint64>::min() / 2;

enum Phase { Down, Move, Up, Cancel };

QRectF inflated(const QRectF &r, qreal by) { return r.adjusted(-by, -by, by, by); }

} // namespace

KeyboardView::KeyboardView(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_painter(2),
      m_emoji(2, EmojiPanel::Actions{
          [this](const QString &e) { if (m_controller) { m_controller->onText(e); m_controller->addRecentEmoji(e); } },
          [this] { if (m_controller) m_controller->onBackspace(false); },
          [this] { show(Page::Letters); },
          [this] { update(); } }),
      m_bold(QStringLiteral("Prelude")),
      m_lastShiftTap(NEVER), m_lastAltTap(NEVER), m_flashAt(NEVER)
{
    m_bold.setBold(true);
    m_bold.setStyleStrategy(QFont::PreferAntialias);
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setOpaquePainting(false);
    setAntialiasing(true);
    m_holdTimer.setSingleShot(true);
    connect(&m_holdTimer, &QTimer::timeout, this, &KeyboardView::onHold);
    m_repeatTimer.setSingleShot(true);
    connect(&m_repeatTimer, &QTimer::timeout, this, &KeyboardView::onRepeat);
    m_barHoldTimer.setSingleShot(true);
    m_barHoldTimer.setInterval(HOLD_FOR_ADD);
    connect(&m_barHoldTimer, &QTimer::timeout, this, &KeyboardView::onBarHold);
    // For what moves on its own: the language's name fading, an emoji fling.
    m_frameTimer.setInterval(16);
    connect(&m_frameTimer, &QTimer::timeout, this, [this] {
        const bool fading = now() - m_flashAt < FLASH_SHOW + FLASH_FADE;
        const bool flinging = m_page == Page::Emoji && m_emoji.animate();
        if (!fading && !flinging) m_frameTimer.stop();
        update();
    });
}

KeyboardView::~KeyboardView()
{
}

qint64 KeyboardView::now()
{
    static QElapsedTimer clock;
    if (!clock.isValid()) clock.start();
    return clock.elapsed();
}

QObject *KeyboardView::controllerObject() const { return m_controller; }

void KeyboardView::setControllerObject(QObject *controller)
{
    Controller *c = qobject_cast<Controller *>(controller);
    if (c == m_controller) return;
    if (m_controller) m_controller->setView(nullptr);
    m_controller = c;
    if (m_controller) m_controller->setView(this);
    Q_EMIT controllerChanged();
}

void KeyboardView::setDp(qreal dp)
{
    if (dp <= 0 || qFuzzyCompare(dp, m_dp)) return;
    m_dp = dp;
    m_painter.setDp(dp);
    m_emoji.setDp(dp);
    layoutKeys();
    Q_EMIT dpChanged();
    Q_EMIT geometryChanged();
    update();
}

void KeyboardView::setLandscape(bool landscape)
{
    if (landscape == m_landscape) return;
    m_landscape = landscape;
    m_painter.clear();
    layoutKeys();
    Q_EMIT geometryChanged();
    update();
}

void KeyboardView::applySettings(const Settings &s)
{
    const bool resize = s.height != m_settings.height || s.showsBar() != m_settings.showsBar();
    const bool relayout = s.language != m_settings.language;
    m_settings = s;
    if (resize) {
        m_painter.clear();
        layoutKeys();
        Q_EMIT geometryChanged();
    } else if (relayout) {
        layoutKeys();
    }
    update();
}

// --- Sizes ------------------------------------------------------------------------------

//! A row's height: the owner's choice, held to a share of the screen in landscape.
qreal KeyboardView::rowHeight() const
{
    const qreal chosen = m_settings.heightDp() * m_dp;
    if (!m_landscape) return chosen;
    const QScreen *screen = QGuiApplication::primaryScreen();
    const qreal h = screen ? std::min(screen->size().width(), screen->size().height()) : chosen * 10;
    return std::min(chosen, h * 0.105);
}

qreal KeyboardView::topBorder() const { return 3 * std::max<qreal>(1, std::round(m_dp)) + 1 * m_dp; }
qreal KeyboardView::bottomPad() const { return 2 * m_dp; }
//! The suggestion bar, above the keys, when it is switched on.
bool KeyboardView::barShown() const { return m_settings.showsBar(); }
qreal KeyboardView::barHeight() const { return barShown() ? std::round(rowHeight() * 0.72) : 0; }
qreal KeyboardView::keysHeight() const { return barHeight() + rowHeight() * 4 + topBorder() + bottomPad(); }
qreal KeyboardView::cellH() const { return rowHeight() * 0.85; }
qreal KeyboardView::popupPad() const { return 5 * m_dp; }
//! Room above the keys for the tallest popup: two lines of accents.
qreal KeyboardView::headroom() const { return std::ceil(2 * cellH() + 2 * popupPad()); }
//! Where the keyboard proper (the bar, if shown, then the keys) starts: above is the popups' room.
qreal KeyboardView::keyboardTop() const { return height() - std::floor(keysHeight()); }

QRectF KeyboardView::rowRect(int i) const
{
    const qreal top = keyboardTop() + barHeight() + topBorder() + i * rowHeight();
    return QRectF(0, std::round(top), width(), std::round(top + rowHeight()) - std::round(top));
}

const Rows &KeyboardView::rows() const
{
    return m_page == Page::Symbols ? Keymap::symbols() : Keymap::letters(m_settings.language);
}

void KeyboardView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        layoutKeys();
        updatePopupRect();
    }
}

void KeyboardView::layoutKeys()
{
    m_zones.clear();
    m_pressed = -1;
    m_preview = -1;
    if (width() <= 0) return;
    if (m_page == Page::Emoji) {
        const QRectF top = rowRect(0), bottom = rowRect(2);
        m_emoji.layout(QRectF(0, top.top(), width(), bottom.bottom() - top.top()), rowRect(3));
        return;
    }
    const Rows &rs = rows();
    for (int i = 0; i < rs.size(); ++i) {
        const QRectF r = rowRect(i);
        qreal units = 0;
        for (const Slot &s : rs[i]) units += s.weight;
        qreal left = 0, sofar = 0;
        for (const Slot &s : rs[i]) {
            sofar += s.weight;
            // Whole pixels, so neighbouring caps never share a blurred edge.
            const qreal right = std::round(width() * sofar / units);
            m_zones.append(Zone{ s, QRectF(left, r.top(), right - left, r.height()) });
            left = right;
        }
    }
}

void KeyboardView::show(Page p)
{
    m_page = p;
    m_alt = Alt::Off;
    if (p == Page::Emoji && m_controller)
        m_emoji.open(m_controller->emojiFile(), m_controller->recentEmoji());
    layoutKeys();
    update();
}

// --- Drawing ----------------------------------------------------------------------------

void KeyboardView::paint(QPainter *pp)
{
    QPainter &p = *pp;
    p.setRenderHint(QPainter::TextAntialiasing);
    const qreal top = keyboardTop();
    m_painter.deck(p, QRectF(0, top + barHeight(), width(), height() - top - barHeight()));
    if (barShown()) drawBar(p, QRectF(0, top, width(), barHeight()));
    if (m_page == Page::Emoji) {
        m_emoji.draw(p, m_painter, m_settings.style,
                     [this](QPainter &cp, const QString &s, const QRectF &r) { functionLabel(cp, s, r); });
        drawMenu(p);
        return;
    }
    for (const Zone &z : std::as_const(m_zones))
        if (z.slot.drawn) drawKey(p, z);
    drawTrail(p);
    drawPreview(p);
    drawExtended(p);
    drawMenu(p);
    drawAsk(p);
}

void KeyboardView::drawKey(QPainter &p, const Zone &z)
{
    const QRectF &r = z.rect;
    const bool isDown = m_pressed >= 0 && m_zones[m_pressed].slot.key == z.slot.key;
    p.drawImage(r.topLeft(), m_painter.cap(int(r.width()), int(r.height()), isDown));
    // The lettering moves down a touch while the key is held.
    QRectF box = r;
    if (isDown) box.translate(0, 1 * m_dp);
    const qreal iconSize = std::min(r.width(), r.height()) * 0.42;
    const Key &key = *z.slot.key;
    const QPointF c = box.center();
    switch (key.kind) {
    case Kind::Char: drawCap(p, key, box); break;
    case Kind::Space:
        if (m_cursorMode) m_painter.icon(p, Painter::Icon::Cursor, c.x(), c.y(), iconSize);
        else drawLanguageFlash(p, box);
        break;
    case Kind::Shift:
        m_painter.icon(p, m_shift == Shift::Off ? Painter::Icon::Shift
                          : m_shift == Shift::Once ? Painter::Icon::ShiftOn : Painter::Icon::ShiftLock,
                       c.x(), c.y(), iconSize);
        break;
    case Kind::Backspace: m_painter.icon(p, Painter::Icon::Backspace, c.x(), c.y(), iconSize); break;
    case Kind::Return: m_painter.icon(p, Painter::Icon::Return, c.x(), c.y(), iconSize); break;
    case Kind::Globe: m_painter.icon(p, Painter::Icon::Globe, c.x(), c.y(), iconSize); break;
    case Kind::Emoji: m_painter.icon(p, Painter::Icon::Emoji, c.x(), c.y(), iconSize); break;
    case Kind::Orange: m_painter.orange(p, box, m_settings.style, m_alt); break;
    case Kind::Sym: functionLabel(p, key.label, box); break;
    }
    if (key.corner != Corner::None) cornerIcon(p, key.corner, box);
    if (holds(key)) holdDot(p, key, box);
}

//! The small icon in a key's top right corner, the size of a printed alternate, always white.
void KeyboardView::cornerIcon(QPainter &p, Corner corner, const QRectF &r)
{
    const qreal size = letterSize() * 0.62;
    const qreal i = inset(r);
    const qreal cx = r.right() - i - size / 2;
    const qreal cy = r.top() + i + size / 2 - size * 0.08;
    if (corner == Corner::Gear)
        m_painter.icon(p, Painter::Icon::Gear, cx, cy, size * 0.9, Qt::white);
}

//! Whether holding [key] opens its list: only with "Hold for more" on.
bool KeyboardView::holds(const Key &key) const
{
    return m_settings.holdForMore && !key.extended.isEmpty();
}

//! The mark of a key with more under it: a small dot in the style's colour, in the top corner the alternate leaves free.
void KeyboardView::holdDot(QPainter &p, const Key &key, const QRectF &r)
{
    const qreal radius = std::max(2 * m_dp, r.height() * 0.045);
    const qreal i = inset(r) + radius;
    const qreal x = (!key.alt.isEmpty() && key.altPos == AltPos::Right) ? r.left() + i : r.right() - i;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(accent(m_settings.style));
    p.drawEllipse(QPointF(x, r.top() + i), radius, radius);
    p.restore();
}

/*!
 * The language just changed: its name shows bright on the space bar for a moment,
 * then fades almost into the key, and stays there, faint, while more than one is
 * checked.
 */
void KeyboardView::flashLanguage()
{
    m_flashAt = now();
    m_frameTimer.start();
    update();
}

void KeyboardView::drawLanguageFlash(QPainter &p, const QRectF &box)
{
    if (m_settings.languages.size() < 2) return;
    const QString name = m_settings.language.shortName;
    const qint64 age = now() - m_flashAt;
    const qreal alpha = age <= FLASH_SHOW ? FLASH_ALPHA
        : age < FLASH_SHOW + FLASH_FADE ? FLASH_ALPHA - (FLASH_ALPHA - RESTING_ALPHA) * (age - FLASH_SHOW) / FLASH_FADE
        : RESTING_ALPHA;
    // One size and one baseline for every language, so the names don't jump as
    // they change: the size the longest name fits at, centred on a capital's height.
    QString longest;
    qreal widest = -1;
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(rowHeight() * 0.3)));
    QFontMetricsF fm(f);
    for (const Language &l : Language::all()) {
        const qreal w = fm.horizontalAdvance(l.shortName);
        if (w > widest) { widest = w; longest = l.shortName; }
    }
    f.setPixelSize(std::max(1, int(fitted(longest, rowHeight() * 0.3, box.width() - 12 * m_dp))));
    p.setFont(f);
    QColor c(Qt::white);
    c.setAlpha(int(alpha));
    p.setPen(c);
    const QFontMetricsF m(f);
    const QRectF ink = m.tightBoundingRect(QStringLiteral("H"));
    p.drawText(QPointF(box.center().x() - m.horizontalAdvance(name) / 2, box.center().y() + ink.height() / 2), name);
}

void KeyboardView::functionLabel(QPainter &p, const QString &s, const QRectF &box)
{
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(fitted(s, rowHeight() * 0.34, box.width() - 8 * m_dp))));
    p.setFont(f);
    p.setPen(Qt::white);
    centred(p, s, box);
}

//! The letter's size: big and bold, but held to the key's width and to half its height.
qreal KeyboardView::letterSize() const { return std::min(rowHeight() * 0.5, width() / 10 * 0.56); }

//! Inset of the glyphs' ink from the key's edges.
qreal KeyboardView::inset(const QRectF &r) const { return 2.5 * m_dp + r.width() * 0.08; }

/*!
 * A character key's printing: a key with an alternate puts it in one top corner
 * and the letter low in the key, just off centre away from it; a key without one
 * centres its glyph.
 */
void KeyboardView::drawCap(QPainter &p, const Key &key, const QRectF &r)
{
    const QString letter = m_page == Page::Symbols ? key.text : key.text.toUpper();
    const qreal size = letterSize();
    const QString &a = key.alt;
    QFont f = m_bold;
    if (a.isEmpty()) {
        f.setPixelSize(std::max(1, int(fitted(letter, size, r.width() - 2 * inset(r)))));
        p.setFont(f);
        p.setPen(key.accent ? accent(m_settings.style) : QColor(Qt::white));
        centred(p, letter, r);
        return;
    }
    const qreal i = inset(r);
    const bool altLeft = key.altPos == AltPos::Left;
    // The letter steps back while the orange key says the alternate is what will be typed.
    f.setPixelSize(std::max(1, int(size)));
    p.setFont(f);
    QColor white(Qt::white);
    white.setAlpha(m_alt != Alt::Off ? 89 : 255);
    p.setPen(white);
    // Every letter sits on one baseline, taken from a flat-bottomed capital, so
    // Q's tail hangs below the line instead of lifting the Q. As on the Pre, a
    // letter is centred in its key by its advance, nudged a little away from its
    // alternate. A symbol is lifted until its ink rests on the letters' line.
    QFontMetricsF fm(f);
    qreal baseline = r.bottom() - i - fm.tightBoundingRect(QStringLiteral("H")).bottom();
    bool allLetters = true;
    for (QChar ch : letter) if (!ch.isLetter()) allLetters = false;
    if (!allLetters)
        baseline = std::min(baseline, r.bottom() - i - fm.tightBoundingRect(letter).bottom());
    const qreal nudge = r.width() * LETTER_NUDGE * (altLeft ? 1 : -1);
    p.drawText(QPointF(r.center().x() + nudge - fm.horizontalAdvance(letter) / 2, baseline), letter);

    // The alternate, in the top corner: its edge meets the inset, and it is
    // centred on a capital's height there, so "-" and "'" line up with the digits.
    const qreal asize = size * 0.7;
    f.setPixelSize(std::max(1, int(asize)));
    p.setFont(f);
    p.setPen(a.length() == 1 && a[0].isDigit() ? accent(m_settings.style) : QColor(Qt::white));
    const QFontMetricsF am(f);
    const QRectF ink = am.tightBoundingRect(a);
    const qreal ax = altLeft ? r.left() + i - ink.left() : r.right() - i - ink.right();
    const qreal ay = r.top() + i + asize * CAP_HEIGHT / 2 - (ink.top() + ink.bottom()) / 2;
    p.drawText(QPointF(ax, ay), a);
}

void KeyboardView::centred(QPainter &p, const QString &s, const QRectF &r)
{
    const QRectF ink = QFontMetricsF(p.font()).tightBoundingRect(s);
    p.drawText(QPointF(r.center().x() - (ink.left() + ink.right()) / 2, r.center().y() - (ink.top() + ink.bottom()) / 2), s);
}

//! [size], shrunk until [s] fits [room].
qreal KeyboardView::fitted(const QString &s, qreal size, qreal room) const
{
    QFont f = m_bold;
    qreal px = size;
    f.setPixelSize(std::max(1, int(px)));
    while (room > 0 && QFontMetricsF(f).horizontalAdvance(s) > room && px > 6) {
        px -= 1;
        f.setPixelSize(int(px));
    }
    return px;
}

// --- The key preview --------------------------------------------------------------------

//! What a character key would type right now.
QString KeyboardView::output(const Key &key) const
{
    if (m_alt != Alt::Off && !key.alt.isEmpty()) return key.alt;
    return m_shift != Shift::Off ? key.text.toUpper() : key.text;
}

void KeyboardView::drawPreview(QPainter &p)
{
    if (m_preview < 0 || !m_extended.isEmpty()) return;
    const Zone &z = m_zones[m_preview];
    const QString s = output(*z.slot.key);
    const qreal w = std::max(z.rect.width() * 1.25, rowHeight() * 0.95);
    const qreal h = rowHeight() * 1.15;
    const qreal bottom = z.rect.top() + rowHeight() * 0.1;
    const qreal left = std::max<qreal>(0, std::min(z.rect.center().x() - w / 2, std::max<qreal>(0, width() - w)));
    const QRectF r(left, bottom - h, w, h);
    bubble(p, r);
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(fitted(s, rowHeight() * 0.7, w - 8 * m_dp))));
    p.setFont(f);
    p.setPen(z.slot.key->accent || (m_alt != Alt::Off && s.length() == 1 && s[0].isDigit())
             ? accent(m_settings.style) : QColor(Qt::white));
    centred(p, s, r);
}

void KeyboardView::bubble(QPainter &p, const QRectF &r)
{
    const qreal rad = 8 * m_dp;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(30, 34, 36));
    p.drawRoundedRect(r, rad, rad);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(86, 94, 99), 1 * m_dp));
    p.drawRoundedRect(r.adjusted(m_dp / 2, m_dp / 2, -m_dp / 2, -m_dp / 2), rad, rad);
    p.restore();
}

// --- Press and hold: accents ------------------------------------------------------------

QStringList KeyboardView::extendedFor(const Key &key) const
{
    if (m_shift != Shift::Off && m_page == Page::Letters) {
        QStringList up;
        for (const QString &s : key.extended) up << s.toUpper();
        return up;
    }
    return key.extended;
}

qreal KeyboardView::cellW() const { return std::max(width() / 10 * 0.92, 34 * m_dp); }

void KeyboardView::showExtended(const Zone &z)
{
    const QStringList chars = extendedFor(*z.slot.key);
    if (chars.isEmpty()) return;
    const int per = perLine(chars.size());
    const int lines = (chars.size() + per - 1) / per;
    const qreal w = per * cellW() + 2 * popupPad();
    const qreal h = lines * cellH() + 2 * popupPad();
    // The key's own character sits right above the finger, on the bottom line.
    const qreal left = std::max<qreal>(0, std::min(z.rect.center().x() - cellW() / 2 - popupPad(), std::max<qreal>(0, width() - w)));
    const qreal top = std::max<qreal>(0, z.rect.top() - h);
    m_extended = chars;
    m_extendedFrame = QRectF(left, top, w, h);
    m_extendedIndex = 0;
    if (m_controller) m_controller->hapticLongPress();
    updatePopupRect();
    update();
}

//! The cell for the [i]th entry: line 0 is the bottom one, nearest the key.
QRectF KeyboardView::cell(int i, int count, const QRectF &frame) const
{
    const int per = perLine(count);
    const int lines = (count + per - 1) / per;
    const qreal x = frame.left() + popupPad() + (i % per) * cellW();
    const qreal y = frame.top() + popupPad() + (lines - 1 - i / per) * cellH();
    return QRectF(x, y, cellW(), cellH());
}

void KeyboardView::drawExtended(QPainter &p)
{
    if (m_extended.isEmpty()) return;
    bubble(p, m_extendedFrame);
    const bool white = m_settings.style == Style::White;
    for (int i = 0; i < m_extended.size(); ++i) {
        const QRectF r = cell(i, m_extended.size(), m_extendedFrame);
        if (i == m_extendedIndex) {
            p.save();
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(white ? QColor(Qt::white) : accent(m_settings.style));
            p.drawRoundedRect(r.adjusted(m_dp, m_dp, -m_dp, -m_dp), 6 * m_dp, 6 * m_dp);
            p.restore();
        }
        QFont f = m_bold;
        f.setPixelSize(std::max(1, int(fitted(m_extended[i], rowHeight() * 0.45, r.width() - 6 * m_dp))));
        p.setFont(f);
        p.setPen(i == m_extendedIndex && white ? QColor(Qt::black) : QColor(Qt::white));
        centred(p, m_extended[i], r);
    }
}

int KeyboardView::extendedAt(qreal x, qreal y) const
{
    if (m_extended.isEmpty()) return 0;
    const int per = perLine(m_extended.size());
    const int lines = (m_extended.size() + per - 1) / per;
    // A finger off the balloon picks the nearest cell, so sliding up from the key works.
    const int col = std::max(0, std::min(per - 1, int((x - m_extendedFrame.left() - popupPad()) / cellW())));
    const int row = std::max(0, std::min(lines - 1, int((y - m_extendedFrame.top() - popupPad()) / cellH())));
    return std::max(0, std::min(int(m_extended.size()) - 1, (lines - 1 - row) * per + col));
}

// --- Press and hold: space moves the cursor ---------------------------------------------

void KeyboardView::startCursor(qreal x, qreal y)
{
    m_cursorMode = true;
    m_cursorX = x;
    m_cursorY = y;
    m_preview = -1;
    if (m_controller) m_controller->hapticLongPress();
    update();
}

//! One character per CURSOR_STEP_X dp across, one line per half a row up or down.
void KeyboardView::moveCursor(qreal x, qreal y)
{
    if (!m_controller) return;
    const bool select = m_shift != Shift::Off;
    const qreal stepX = CURSOR_STEP_X * m_dp;
    const qreal stepY = rowHeight() * 0.5;
    while (x - m_cursorX >= stepX) { m_cursorX += stepX; m_controller->onCursor(1, 0, select); }
    while (m_cursorX - x >= stepX) { m_cursorX -= stepX; m_controller->onCursor(-1, 0, select); }
    while (y - m_cursorY >= stepY) { m_cursorY += stepY; m_controller->onCursor(0, 1, select); }
    while (m_cursorY - y >= stepY) { m_cursorY -= stepY; m_controller->onCursor(0, -1, select); }
}

// --- The suggestion bar -----------------------------------------------------------------

void KeyboardView::setSuggestions(const QStringList &w)
{
    QStringList next;
    for (int i = 0; i < 3; ++i) next << w.value(i);
    if (next != m_words) { m_words = next; update(); }
}

void KeyboardView::setPasteOffered(bool offered)
{
    if (offered != m_pasteOffered) { m_pasteOffered = offered; update(); }
}

void KeyboardView::drawBar(QPainter &p, const QRectF &r)
{
    p.fillRect(r, BAR);
    const qreal cw = r.width() / 3;
    // One baseline for every word, from the font rather than each word's ink, so a
    // word doesn't jump when a letter with a descender ("hop") comes along.
    const qreal size = r.height() * 0.46;
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(size)));
    const QFontMetricsF fm(f);
    const qreal baseline = r.center().y() - (-fm.ascent() + fm.descent()) / 2;
    for (int i = 0; i < 3; ++i) {
        const QRectF cellRect(r.left() + i * cw, r.top(), cw, r.height());
        const bool paste = i == 2 && m_pasteOffered;
        if (i == m_barPressed && (!m_words[i].isEmpty() || paste)) p.fillRect(cellRect, BAR_PRESSED);
        if (i > 0)
            p.fillRect(QRectF(cellRect.left(), r.top() + r.height() * 0.25, std::max<qreal>(1, m_dp), r.height() * 0.5), BAR_RULE);
        if (paste) {
            m_painter.icon(p, Painter::Icon::Paste, cellRect.center().x(), cellRect.center().y(), r.height() * 0.56, Qt::white);
            continue;
        }
        const QString &w = m_words[i];
        if (w.isEmpty()) continue;
        // The likeliest word, in the middle, in the Pre's orange in either style.
        QFont wf = m_bold;
        wf.setPixelSize(std::max(1, int(fitted(w, size, cw - 12 * m_dp))));
        p.setFont(wf);
        p.setPen(i == 1 ? accent(Style::Orange) : QColor(Qt::white));
        p.drawText(QPointF(cellRect.center().x() - QFontMetricsF(wf).horizontalAdvance(w) / 2, baseline), w);
    }
}

// --- Holding a word in the bar: add it to the dictionary -------------------------------

void KeyboardView::onBarHold()
{
    const int i = m_barPressed;
    if (i < 0 || (i == 2 && m_pasteOffered)) return;
    const QString w = m_words.value(i);
    if (w.isEmpty() || !m_controller || !m_controller->canAdd(w)) return;
    m_barPressed = -1;
    m_asking = w;
    m_askDown = false;
    if (m_controller) m_controller->hapticLongPress();
    updatePopupRect();
    update();
}

void KeyboardView::ask(const QString &word)
{
    if (!m_asking.isEmpty()) return;
    m_asking = word;
    m_askDown = false;
    if (m_controller) m_controller->hapticLongPress();
    updatePopupRect();
    update();
}

//! While the popup is up, every touch is its: Yes adds, Cancel or anywhere else closes it.
bool KeyboardView::askTouch(int phase, qreal x, qreal y)
{
    auto hit = [&](qreal hx, qreal hy) {
        return m_askYes.contains(QPointF(hx, hy)) ? 1 : m_askNo.contains(QPointF(hx, hy)) ? -1 : 0;
    };
    switch (phase) {
    case Down:
        m_askDown = true;
        m_askPressed = hit(x, y);
        if (m_askPressed != 0) keyFeedback();
        break;
    case Up:
        if (m_askDown) {
            const QString word = m_asking;
            const int h = hit(x, y);
            if (h == m_askPressed) {
                m_asking.clear();
                updatePopupRect();
                if (h == 1 && !word.isEmpty() && m_controller) m_controller->onAddWord(word);
            }
            m_askDown = false;
            m_askPressed = 0;
        }
        break;
    case Cancel:
        m_askDown = false;
        m_askPressed = 0;
        break;
    }
    update();
    return true;
}

//! A small popup over the top of the keyboard: Add "word" to dictionary? Cancel / Yes.
void KeyboardView::drawAsk(QPainter &p)
{
    if (m_asking.isEmpty()) return;
    const qreal top = keyboardTop();
    p.fillRect(QRectF(0, top, width(), height() - top), QColor(0, 0, 0, 150));
    const qreal pad = 10 * m_dp;
    const qreal w = std::min(width() - 24 * m_dp, 340 * m_dp);
    const qreal buttonH = rowHeight() * 0.72;
    const qreal lineH = rowHeight() * 0.8;
    const qreal h = pad + lineH + buttonH + pad;
    const qreal left = (width() - w) / 2;
    const qreal boxTop = top + std::max(4 * m_dp, (barHeight() + rowHeight() - h) / 2 + 4 * m_dp);
    m_askBox = QRectF(left, boxTop, w, h);
    bubble(p, m_askBox);
    const QString question = QStringLiteral("Add “%1” to dictionary?").arg(m_asking);
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(fitted(question, rowHeight() * 0.33, w - 2 * pad))));
    p.setFont(f);
    p.setPen(Qt::white);
    centred(p, question, QRectF(left, boxTop + pad, w, lineH));
    const qreal bw = (w - 3 * pad) / 2;
    const qreal by = boxTop + pad + lineH;
    m_askNo = QRectF(left + pad, by, bw, buttonH);
    m_askYes = QRectF(left + 2 * pad + bw, by, bw, buttonH);
    const qreal rad = 6 * m_dp;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(m_askDown && m_askPressed == -1 ? QColor(96, 104, 110) : QColor(64, 70, 75));
    p.drawRoundedRect(m_askNo, rad, rad);
    p.setBrush(m_askDown && m_askPressed == 1 ? QColor(0xFF, 0x7A, 0x60) : accent(Style::Orange));
    p.drawRoundedRect(m_askYes, rad, rad);
    p.restore();
    const QList<QPair<QString, QRectF>> labels = { { QStringLiteral("Cancel"), m_askNo }, { QStringLiteral("Yes"), m_askYes } };
    for (const auto &l : labels) {
        QFont lf = m_bold;
        lf.setPixelSize(std::max(1, int(fitted(l.first, rowHeight() * 0.32, l.second.width() - 8 * m_dp))));
        p.setFont(lf);
        p.setPen(Qt::white);
        centred(p, l.first, l.second);
    }
}

// --- The globe's menu -------------------------------------------------------------------

/*!
 * The globe on the Sym page, where LuneOS's own keyboard has its language key: a
 * list of the languages checked, to go straight to one, and LuneOS's own
 * keyboard, to go back to it. (Holding the globe still switches the style.)
 */
void KeyboardView::showMenu()
{
    m_menu.clear();
    for (const Language &l : m_settings.languages)
        m_menu << l.name;
    m_menu << QStringLiteral("LuneOS keyboard");
    int globe = -1;
    for (int i = 0; i < m_zones.size(); ++i)
        if (m_zones[i].slot.key->kind == Kind::Globe) { globe = i; break; }
    const qreal rowH = cellH();
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(rowHeight() * 0.36)));
    qreal w = 0;
    for (const QString &s : m_menu) w = std::max(w, QFontMetricsF(f).horizontalAdvance(s));
    w = std::min(width(), w + 4 * popupPad() + 16 * m_dp);
    const qreal h = m_menu.size() * rowH + 2 * popupPad();
    const QRectF key = globe >= 0 ? m_zones[globe].rect : QRectF(0, keyboardTop(), 0, 0);
    const qreal top = std::max<qreal>(0, key.top() - h);
    m_menuFrame = QRectF(std::min(key.left(), width() - w), top, w, h);
    m_menuPressed = -1;
    m_menuDown = false;
    updatePopupRect();
    update();
}

QRectF KeyboardView::menuRow(int i) const
{
    return QRectF(m_menuFrame.left() + popupPad(), m_menuFrame.top() + popupPad() + i * cellH(),
                  m_menuFrame.width() - 2 * popupPad(), cellH());
}

int KeyboardView::menuAt(qreal x, qreal y) const
{
    for (int i = 0; i < m_menu.size(); ++i)
        if (menuRow(i).contains(QPointF(x, y))) return i;
    return -1;
}

void KeyboardView::drawMenu(QPainter &p)
{
    if (m_menu.isEmpty()) return;
    bubble(p, m_menuFrame);
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(rowHeight() * 0.36)));
    p.setFont(f);
    const int languages = m_settings.languages.size();
    for (int i = 0; i < m_menu.size(); ++i) {
        const QRectF r = menuRow(i);
        const bool current = i < languages && m_settings.languages[i] == m_settings.language;
        if (i == m_menuPressed) {
            p.save();
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(m_settings.style == Style::White ? QColor(Qt::white) : accent(m_settings.style));
            p.drawRoundedRect(r.adjusted(m_dp, m_dp, -m_dp, -m_dp), 6 * m_dp, 6 * m_dp);
            p.restore();
        }
        if (i == languages)
            p.fillRect(QRectF(r.left() + 6 * m_dp, r.top(), r.width() - 12 * m_dp, std::max<qreal>(1, m_dp)), BAR_RULE);
        p.setPen(i == m_menuPressed && m_settings.style == Style::White ? QColor(Qt::black)
                 : current ? accent(Style::Orange) : QColor(Qt::white));
        const QFontMetricsF fm(f);
        p.drawText(QPointF(r.left() + 8 * m_dp, r.center().y() - (-fm.ascent() + fm.descent()) / 2), m_menu[i]);
    }
}

//! While the menu is up, every touch is its: a row picks, anywhere else closes it.
bool KeyboardView::menuTouch(int phase, qreal x, qreal y)
{
    switch (phase) {
    case Down:
        m_menuDown = true;
        m_menuPressed = menuAt(x, y);
        if (m_menuPressed >= 0) keyFeedback();
        break;
    case Move:
        if (m_menuDown) m_menuPressed = menuAt(x, y);
        break;
    case Up: {
        if (!m_menuDown) break;
        const int i = menuAt(x, y);
        const int languages = m_settings.languages.size();
        m_menu.clear();
        m_menuDown = false;
        m_menuPressed = -1;
        updatePopupRect();
        if (m_controller && i >= 0) {
            if (i < languages) m_controller->onUseLanguage(m_settings.languages[i].id);
            else m_controller->onUseLuneOSKeyboard();
        }
        break;
    }
    case Cancel:
        m_menuDown = false;
        m_menuPressed = -1;
        break;
    }
    update();
    return true;
}

// --- Swipe to type ----------------------------------------------------------------------

bool KeyboardView::canSwipe(const Zone &z) const
{
    const Key &k = *z.slot.key;
    return m_settings.swipe && m_wordsAllowed && m_page == Page::Letters && m_alt == Alt::Off &&
           k.kind == Kind::Char && k.text.length() == 1 && k.text[0] >= QLatin1Char('a') && k.text[0] <= QLatin1Char('z');
}

void KeyboardView::addSwipePoint(qreal x, qreal y)
{
    m_swipeX << float(x);
    m_swipeY << float(y);
}

int KeyboardView::capsNow() const
{
    return m_shift == Shift::Off ? CAPS_NONE : m_shift == Shift::Once ? CAPS_FIRST : CAPS_ALL;
}

bool KeyboardView::swipeLayout(SwipeDecoder::Layout &out) const
{
    int found = 0;
    float keyW = 0;
    for (const Zone &z : m_zones) {
        const Key &k = *z.slot.key;
        if (k.kind != Kind::Char || k.text.length() != 1 || k.text[0] < QLatin1Char('a') || k.text[0] > QLatin1Char('z'))
            continue;
        const int i = k.text[0].unicode() - 'a';
        out.x[i] = float(z.rect.center().x());
        out.y[i] = float(z.rect.center().y());
        if (k.text == QLatin1String("q")) keyW = float(z.rect.width());
        found++;
    }
    out.keyWidth = keyW;
    out.keyHeight = float(rowHeight());
    return found == 26 && keyW > 0;
}

void KeyboardView::drawTrail(QPainter &p)
{
    if (!m_swiping || m_swipeX.size() < 2) return;
    QPainterPath trail;
    trail.moveTo(m_swipeX[0], m_swipeY[0]);
    for (int i = 1; i < m_swipeX.size(); ++i) trail.lineTo(m_swipeX[i], m_swipeY[i]);
    QColor c = m_settings.style == Style::White ? QColor(Qt::white) : accent(m_settings.style);
    c.setAlpha(200);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(c, 6 * m_dp, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(trail);
    p.restore();
}

// --- Popups and the input mask ----------------------------------------------------------

//! Where a popup is, for the window's input mask: the balloon, the menu, or the question.
void KeyboardView::updatePopupRect()
{
    QRectF r;
    if (!m_extended.isEmpty()) r = r.united(m_extendedFrame);
    if (!m_menu.isEmpty()) r = r.united(m_menuFrame);
    // The key preview is drawn above the keys too, but a touch never goes to it.
    if (r != m_popupRect) {
        m_popupRect = r;
        Q_EMIT popupRectChanged();
    }
}

// --- Touch ------------------------------------------------------------------------------

int KeyboardView::zoneAt(qreal x, qreal y) const
{
    for (int i = 0; i < m_zones.size(); ++i)
        if (m_zones[i].rect.contains(QPointF(x, y))) return i;
    if (y < keyboardTop() || m_zones.isEmpty()) return -1;
    // Below the last row (the bottom pad) or in a rounding gap: the key in that column.
    const qreal yy = std::max(m_zones.first().rect.top(), std::min(y, m_zones.last().rect.bottom() - 1));
    for (int i = 0; i < m_zones.size(); ++i) {
        const QRectF &r = m_zones[i].rect;
        if (yy >= r.top() && yy < r.bottom() && x >= r.left() && x < r.right()) return i;
    }
    return -1;
}

int KeyboardView::barCell(qreal x, qreal y) const
{
    const qreal top = keyboardTop();
    if (!barShown() || y < top || y >= top + barHeight()) return -1;
    return std::max(0, std::min(2, int(x / (width() / 3))));
}

void KeyboardView::keyFeedback()
{
    if (m_settings.vibrate && m_controller) m_controller->hapticKey();
    if (m_settings.sound) Q_EMIT keyClick();
}

void KeyboardView::stopRepeat()
{
    m_repeatTimer.stop();
    if (m_deleting) {
        m_deleting = false;
        if (m_controller) m_controller->onBackspaceHeld(false);
    }
}

void KeyboardView::onRepeat()
{
    if (m_pressed < 0 || m_zones[m_pressed].slot.key->kind != Kind::Backspace) return;
    m_repeats++;
    const bool word = m_repeats * LETTER_REPEAT > WORD_DELETE_AFTER;
    if (m_controller) m_controller->onBackspace(word);
    m_repeatTimer.start(word ? WORD_REPEAT : LETTER_REPEAT);
}

void KeyboardView::onHold()
{
    if (m_pressed < 0) return;
    const Zone z = m_zones[m_pressed];
    const Key &key = *z.slot.key;
    switch (key.kind) {
    case Kind::Space:
        m_held = true;
        startCursor(m_downX, m_downY);
        break;
    case Kind::Orange:
        m_held = true;
        if (m_controller) { m_controller->hapticLongPress(); m_controller->onToggleHold(); }
        break;
    case Kind::Globe:
        m_held = true;
        if (m_controller) { m_controller->hapticLongPress(); m_controller->onToggleStyle(); }
        break;
    default:
        if (key.corner == Corner::Gear) {
            m_held = true;
            if (m_controller) { m_controller->hapticLongPress(); m_controller->onOpenSettings(); }
        } else if (holds(key)) {
            m_held = true;
            showExtended(z);
        }
        break;
    }
}

void KeyboardView::press(int zone)
{
    m_holdTimer.stop();
    stopRepeat();
    m_pressed = zone;
    m_held = false;
    m_preview = (zone >= 0 && m_zones[zone].slot.key->kind == Kind::Char) ? zone : -1;
    if (zone < 0) { update(); return; }
    const Key &key = *m_zones[zone].slot.key;
    switch (key.kind) {
    case Kind::Backspace:
        m_repeats = 0;
        m_deleting = true;
        if (m_controller) { m_controller->onBackspaceHeld(true); m_controller->onBackspace(false); }
        m_repeatTimer.start(FIRST_REPEAT);
        break;
    case Kind::Space:
        m_holdTimer.start(HOLD_FOR_CURSOR);
        break;
    case Kind::Orange:
    case Kind::Globe:
        m_holdTimer.start(HOLD_FOR_SETTING);
        break;
    default:
        if (key.corner == Corner::Gear) m_holdTimer.start(HOLD_FOR_SETTING);
        else if (holds(key)) m_holdTimer.start(HOLD_FOR_EXTENDED);
        break;
    }
    update();
}

//! A finger landed at [x], [y]: false when it isn't on a key.
bool KeyboardView::down(qreal x, qreal y)
{
    const int z = zoneAt(x, y);
    if (z < 0) return false;
    m_downX = x;
    m_downY = y;
    keyFeedback();
    m_spent = false;
    press(z);
    m_swiping = false;
    m_swipeMaybe = canSwipe(m_zones[z]);
    m_swipeX.clear();
    m_swipeY.clear();
    if (m_swipeMaybe) addSwipePoint(x, y);
    return true;
}

void KeyboardView::move(qreal x, qreal y)
{
    if (m_spent) return;
    if (m_cursorMode) { moveCursor(x, y); return; }
    if (m_swipeMaybe && m_extended.isEmpty() && !m_held) {
        addSwipePoint(x, y);
        if (!m_swiping && std::hypot(x - m_downX, y - m_downY) > swipeStart()) {
            // Off the first key and on its way: a swipe, not a tap.
            m_swiping = true;
            m_holdTimer.stop();
            m_pressed = -1;
            m_preview = -1;
        }
        if (m_swiping) {
            const qint64 t = now();
            if (t - m_lastLive >= LIVE_EVERY && m_controller) {
                m_lastLive = t;
                m_controller->onSwipeProgress(m_swipeX, m_swipeY, capsNow());
            }
            update();
            return;
        }
    }
    if (!m_extended.isEmpty()) {
        const int i = extendedAt(x, y);
        if (i != m_extendedIndex) { m_extendedIndex = i; update(); }
        return;
    }
    if (m_held) return;
    if (m_pressed >= 0 && m_zones[m_pressed].slot.key->kind == Kind::Space && flickedLanguage(x, y)) return;
    const int z = zoneAt(x, y);
    if (z >= 0 && m_pressed >= 0 && m_zones[z].slot.key == m_zones[m_pressed].slot.key) return;
    if (z < 0 && m_pressed < 0) return;
    // Off backspace, or onto it from another key: backspace only works where the
    // finger landed, and the press is over till the finger lifts.
    if ((m_pressed >= 0 && m_zones[m_pressed].slot.key->kind == Kind::Backspace) ||
        (z >= 0 && m_zones[z].slot.key->kind == Kind::Backspace)) {
        m_holdTimer.stop();
        stopRepeat();
        m_spent = true;
        m_pressed = -1;
        m_preview = -1;
        update();
        return;
    }
    // Sliding off a key onto another moves the press, as webOS allowed.
    press(z);
}

/*!
 * A quick flick sideways from the space bar, before the hold that moves the
 * cursor: the next language or the one before, when more than one is checked.
 * The press is spent, so lifting types no space.
 */
bool KeyboardView::flickedLanguage(qreal x, qreal y)
{
    if (m_settings.languages.size() < 2) return false;
    const qreal dx = x - m_downX;
    if (std::abs(dx) < width() / 10 * FLICK_KEYS || std::abs(dx) < 2 * std::abs(y - m_downY)) return false;
    m_holdTimer.stop();
    m_spent = true;
    m_pressed = -1;
    m_preview = -1;
    if (m_controller) m_controller->onLanguageFlick(dx > 0 ? 1 : -1);
    update();
    return true;
}

//! The finger being followed lifted ([up]), or the touch was called off: finish its press.
void KeyboardView::lift(bool up)
{
    m_holdTimer.stop();
    stopRepeat();
    if (m_swiping) {
        if (up && m_controller) {
            m_controller->onSwipe(m_swipeX, m_swipeY, capsNow());
            if (m_shift == Shift::Once) { m_shift = Shift::Off; m_autoShift = false; m_lastShiftTap = NEVER; }
        }
        m_pressed = -1;
    } else if (up && !m_spent) {
        commit();
    }
    m_swiping = false;
    m_swipeMaybe = false;
    m_swipeX.clear();
    m_swipeY.clear();
    const bool hadPopup = !m_extended.isEmpty();
    m_extended.clear();
    m_extendedFrame = QRectF();
    m_cursorMode = false;
    m_held = false;
    m_spent = false;
    m_pressed = -1;
    m_preview = -1;
    if (hadPopup) updatePopupRect();
    update();
}

void KeyboardView::commit()
{
    if (!m_extended.isEmpty()) {
        type(m_extended.value(m_extendedIndex), false);
        return;
    }
    if (m_held || m_pressed < 0) return;
    const Key &key = *m_zones[m_pressed].slot.key;
    switch (key.kind) {
    case Kind::Shift: {
        const qint64 t = now();
        if (m_shift == Shift::Lock) m_shift = Shift::Off;
        else if (t - m_lastShiftTap < DOUBLE_TAP) m_shift = Shift::Lock;
        else if (m_shift == Shift::Once) m_shift = Shift::Off;
        else m_shift = Shift::Once;
        m_autoShift = false;
        m_lastShiftTap = m_shift == Shift::Off ? NEVER : t;
        break;
    }
    // Tap: the next key types its printed alternate. Double tap: locked on. Tap
    // while on: off again. The same rhythm as shift, as on the Pre.
    case Kind::Orange: {
        const qint64 t = now();
        if (m_alt == Alt::Lock) m_alt = Alt::Off;
        else if (t - m_lastAltTap < DOUBLE_TAP) m_alt = Alt::Lock;
        else if (m_alt == Alt::Once) m_alt = Alt::Off;
        else m_alt = Alt::Once;
        m_lastAltTap = m_alt == Alt::Off ? NEVER : t;
        break;
    }
    case Kind::Sym: show(m_page == Page::Letters ? Page::Symbols : Page::Letters); break;
    case Kind::Emoji: show(Page::Emoji); break;
    case Kind::Backspace: break;     // sent on the way down, and repeated since
    case Kind::Return: if (m_controller) m_controller->onEnter(); break;
    case Kind::Globe: showMenu(); break;
    case Kind::Space: if (m_controller) m_controller->onText(QStringLiteral(" ")); break;
    case Kind::Char: type(output(key), m_alt != Alt::Off && !key.alt.isEmpty()); break;
    }
}

void KeyboardView::type(const QString &s, bool usedAlt)
{
    if (m_controller) m_controller->onText(s);
    // A one-shot spent is not half of a double tap.
    if (m_shift == Shift::Once) { m_shift = Shift::Off; m_autoShift = false; m_lastShiftTap = NEVER; }
    if (usedAlt && m_alt == Alt::Once) { m_alt = Alt::Off; m_lastAltTap = NEVER; }
}

void KeyboardView::updateCaps()
{
    if (m_page != Page::Letters || m_shift == Shift::Lock || (m_shift == Shift::Once && !m_autoShift)) return;
    const bool caps = m_controller && m_controller->wantsCaps();
    const Shift next = caps ? Shift::Once : Shift::Off;
    if (next != m_shift) { m_shift = next; update(); }
    m_autoShift = caps;
}

void KeyboardView::reset()
{
    m_shift = Shift::Off;
    m_autoShift = false;
    m_alt = Alt::Off;
    m_lastShiftTap = NEVER;
    m_lastAltTap = NEVER;
    if (m_page != Page::Letters) show(Page::Letters);
    m_extended.clear();
    m_extendedFrame = QRectF();
    m_pressed = -1;
    m_preview = -1;
    m_cursorMode = false;
    m_held = false;
    m_swiping = false;
    m_swipeMaybe = false;
    m_swipeX.clear();
    m_swipeY.clear();
    m_barPressed = -1;
    m_asking.clear();
    m_askDown = false;
    m_menu.clear();
    m_finger = -1;
    m_holdTimer.stop();
    m_barHoldTimer.stop();
    stopRepeat();
    updatePopupRect();
    updateCaps();
    update();
}

// --- Pointers: touch, and a mouse when there is no touch screen --------------------------

bool KeyboardView::pointerDown(int id, qreal x, qreal y)
{
    // A popup takes the touch; this is the finger whose lift it waits for.
    if (!m_menu.isEmpty()) { m_finger = id; return menuTouch(Down, x, y); }
    if (m_page == Page::Emoji) {
        if (y < keyboardTop()) return false;
        keyFeedback();
        m_finger = id;
        m_emoji.down(x, y);
        return true;
    }
    if (!m_asking.isEmpty()) { m_finger = id; return askTouch(Down, x, y); }
    // A touch that starts in the suggestion bar stays there.
    m_barPressed = -1;
    const int bc = barCell(x, y);
    if (bc >= 0) {
        m_finger = id;
        m_barPressed = bc;
        if (!m_words[bc].isEmpty() || (bc == 2 && m_pasteOffered)) keyFeedback();
        m_barHoldTimer.start();
        update();
        return true;
    }
    if (m_finger >= 0) {
        // Typing quickly, the next finger lands before the last one lifts. The key
        // under the first is typed then, as if it had lifted, and the new finger's
        // press begins; a finger that lands during a swipe, a hold or the accents is ignored.
        if (m_swiping || m_cursorMode || !m_extended.isEmpty() || m_held) return true;
        lift(true);
    }
    m_finger = id;
    if (!down(x, y)) { m_finger = -1; return false; }
    return true;
}

void KeyboardView::pointerMove(int id, qreal x, qreal y)
{
    if (!m_menu.isEmpty()) { menuTouch(Move, x, y); return; }
    if (id != m_finger) return;
    if (m_page == Page::Emoji) { m_emoji.move(x, y); if (m_emoji.animate()) m_frameTimer.start(); return; }
    if (!m_asking.isEmpty()) return;
    if (m_barPressed >= 0) return;
    move(x, y);
}

void KeyboardView::pointerUp(int id, qreal x, qreal y)
{
    if (!m_menu.isEmpty() || m_menuDown) { menuTouch(Up, x, y); return; }
    if (id != m_finger) return;
    m_finger = -1;
    if (m_page == Page::Emoji) {
        m_emoji.up(x, y);
        if (m_emoji.animate()) m_frameTimer.start();
        return;
    }
    if (!m_asking.isEmpty() || m_askDown) { askTouch(Up, x, y); return; }
    if (m_barPressed >= 0) {
        m_barHoldTimer.stop();
        const int i = m_barPressed;
        m_barPressed = -1;
        if (barCell(x, y) == i && m_controller) {
            if (i == 2 && m_pasteOffered) m_controller->onPaste();
            else if (!m_words[i].isEmpty()) m_controller->onPick(i);
        }
        update();
        return;
    }
    if (m_swiping) addSwipePoint(x, y);
    lift(true);
}

void KeyboardView::pointerCancel()
{
    m_finger = -1;
    if (!m_menu.isEmpty()) { menuTouch(Cancel, 0, 0); return; }
    if (m_page == Page::Emoji) { m_emoji.cancel(); return; }
    if (!m_asking.isEmpty()) { askTouch(Cancel, 0, 0); return; }
    m_barHoldTimer.stop();
    m_barPressed = -1;
    lift(false);
}

void KeyboardView::touchEvent(QTouchEvent *event)
{
    if (event->type() == QEvent::TouchCancel) {
        pointerCancel();
        event->accept();
        return;
    }
    bool any = false;
    for (const QEventPoint &pt : event->points()) {
        const QPointF pos = pt.position();
        switch (pt.state()) {
        case QEventPoint::Pressed:
            if (pointerDown(pt.id(), pos.x(), pos.y())) any = true;
            break;
        case QEventPoint::Updated:
            pointerMove(pt.id(), pos.x(), pos.y());
            any = true;
            break;
        case QEventPoint::Released:
            pointerUp(pt.id(), pos.x(), pos.y());
            any = true;
            break;
        default:
            any = true;
            break;
        }
    }
    event->setAccepted(any || m_finger >= 0);
}

void KeyboardView::mousePressEvent(QMouseEvent *event)
{
    event->setAccepted(pointerDown(-2, event->position().x(), event->position().y()));
}

void KeyboardView::mouseMoveEvent(QMouseEvent *event)
{
    pointerMove(-2, event->position().x(), event->position().y());
}

void KeyboardView::mouseReleaseEvent(QMouseEvent *event)
{
    pointerUp(-2, event->position().x(), event->position().y());
}

void KeyboardView::touchUngrabEvent()
{
    pointerCancel();
}

void KeyboardView::mouseUngrabEvent()
{
    if (m_finger == -2) pointerCancel();
}

} // namespace Pre
