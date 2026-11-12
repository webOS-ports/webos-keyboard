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

#include <QLocale>
#include <QDateTime>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
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
//! In landscape a key is at most this many times as wide as its row is tall.
//! The screen's full width would make them twice as wide, so the keys keep
//! this shape in the middle and the deck fills the sides.
const qreal LANDSCAPE_KEY_SHAPE = 1.35;
//! In landscape a Medium row is at most this share of the screen's short side.
const qreal LANDSCAPE_ROW_SHARE = 0.105;
//! The language's name on the space bar: shown this long, then fading for this long...
const qint64 FLASH_SHOW = 900;
const qint64 FLASH_FADE = 400;
//! ...from this white to this, just off the key's own black.
const qreal FLASH_ALPHA = 230;
const qreal RESTING_ALPHA = 46;
//! How often the bar's guesses are brought up to date during a swipe.
const qint64 LIVE_EVERY = 50;
//! The screen's short side in dp: Keyboard.qml's dp is that side over this.
const qreal SCREEN_DP = 411.43;
//! Prelude's cap height, as a fraction of the font size.
const qreal CAP_HEIGHT = 0.72;
//! How far a letter sits from its key's centre, away from the alternate (off a Pre photo: E, H).
const qreal LETTER_NUDGE = 0.07;
//! An alternate's size, as a share of the letters'.
const qreal ALT_SCALE = 0.7;
//! The least room, in dp, between a key's printing and the edge of its face.
const qreal INK_MARGIN = 1;
//! The furthest, in dp, the alternates are brought in from their corners.
const qreal MAX_ALT_STEP = 4;
//! The smallest the letters are made to fit their keys, in px.
const qreal MIN_LETTER_PX = 8;
const qint64 NEVER = std::numeric_limits<qint64>::min() / 2;

enum Phase { Down, Move, Up, Cancel };

/*!
 * A string's ink, drawn from the origin: its outline, and that outline
 * flattened to points, which is what is held to a key's face.
 */
struct Ink
{
    Ink(const QString &s, const QFont &font)
    {
        path.addText(QPointF(), font, s);
        for (const QPolygonF &poly : path.toSubpathPolygons()) points += poly;
        box = points.boundingRect();
    }
    QPainterPath path;
    QPolygonF points;
    QRectF box;
};

/*!
 * [ink] drawn from [at] inside [face], a rounded rectangle with corners of
 * [radius]. A hundredth of a pixel's grace: a row is lifted to put its lowest
 * letter exactly on the face's edge, and whether exactly on it counts as in
 * is otherwise down to rounding, which differs from one machine to another
 * (on the Sargo it did not, and every page came out a few pixels smaller).
 */
bool inside(const Ink &ink, const QPointF &at, const QRectF &face, qreal radius)
{
    const qreal grace = 0.01;
    if (!face.adjusted(-grace, -grace, grace, grace).contains(ink.box.translated(at))) return false;
    const qreal r2 = (radius + grace) * (radius + grace);
    for (const QPointF &q : ink.points) {
        const QPointF p = q + at;
        const qreal cx = std::clamp(p.x(), face.left() + radius, face.right() - radius);
        const qreal cy = std::clamp(p.y(), face.top() + radius, face.bottom() - radius);
        const qreal dx = p.x() - cx, dy = p.y() - cy;
        if (dx * dx + dy * dy > r2) return false;
    }
    return true;
}

//! [a] drawn from [at] and [b] from [bt] touch.
bool touch(const Ink &a, const QPointF &at, const Ink &b, const QPointF &bt)
{
    if (!a.box.translated(at).intersects(b.box.translated(bt))) return false;
    return a.path.translated(at).intersects(b.path.translated(bt));
}


} // namespace

KeyboardView::KeyboardView(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_painter(2),
      m_emoji(2, EmojiPanel::Actions{
          [this](const QString &e) { if (m_controller) { m_controller->onText(e); m_controller->addRecentEmoji(e); } },
          [this] { if (m_controller) m_controller->onBackspace(false); },
          [this] { show(Page::Letters); },
          [this] { update(); },
          [this](Klipy::Kind kind, const QString &query) { startSearch(kind, query); },
          [this](Klipy::Kind kind, const Klipy::Item &item, const QString &query) {
              const QString link = Klipy::link(item);
              if (!m_controller || link.isEmpty()) return;
              m_controller->onMedia(int(kind), link, item.slug, query);
              m_emoji.say(kind, QStringLiteral("Sent as a link"), 1500);
          } }),
      m_bold(QStringLiteral("Prelude")),
      m_lastShiftTap(NEVER), m_lastAltTap(NEVER), m_flashAt(NEVER)
{
    m_bold.setBold(true);
    m_bold.setStyleStrategy(QFont::PreferAntialias);
    m_emoji.setFont(m_bold);
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
    const bool relayout = s.language != m_settings.language || s.followShift != m_settings.followShift;
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

/*!
 * A row's height: the owner's choice. In landscape a Medium row is held to a
 * share of the screen's short side, and Small and Large keep their proportion
 * to it, so the choice shows there too (held to one share, all three came out
 * the same). The screen's short side is dp's own measure: Keyboard.qml sets dp
 * to it over SCREEN_DP, so the two cannot disagree, and the tests can lay the
 * keys out for any screen without having one.
 */
qreal KeyboardView::rowHeight() const
{
    const qreal chosen = m_settings.heightDp() * m_dp;
    if (!m_landscape) return chosen;
    const qreal share = LANDSCAPE_ROW_SHARE * m_settings.heightDp() / Settings().heightDp();
    return std::min(chosen, m_dp * SCREEN_DP * share);
}

qreal KeyboardView::topBorder() const { return 3 * std::max<qreal>(1, std::round(m_dp)) + 1 * m_dp; }
qreal KeyboardView::bottomPad() const { return 2 * m_dp; }
//! The suggestion bar, above the keys, when it is switched on.
bool KeyboardView::barShown() const { return m_settings.showsBar() || m_searching; }
qreal KeyboardView::barHeight() const { return barShown() ? std::round(rowHeight() * 0.72) : 0; }
qreal KeyboardView::keysHeight() const { return barHeight() + rowHeight() * 4 + topBorder() + bottomPad(); }
qreal KeyboardView::cellH() const { return rowHeight() * 0.85; }
qreal KeyboardView::popupPad() const { return 5 * m_dp; }
//! Room above the keys for the tallest popup: two lines of accents.
qreal KeyboardView::headroom() const { return std::ceil(2 * cellH() + 2 * popupPad()); }
//! Where the keyboard proper (the bar, if shown, then the keys) starts: above is the popups' room.
qreal KeyboardView::keyboardTop() const { return height() - std::floor(keysHeight()); }

/*!
 * The keys' width: the screen's, held in landscape to ten keys of
 * LANDSCAPE_KEY_SHAPE. Ten for every language, as in portrait, so a twelve-key
 * row has narrower keys there too. The keys sit in the middle.
 */
qreal KeyboardView::keysWidth() const
{
    if (!m_landscape) return width();
    return std::min(width(), std::round(10 * LANDSCAPE_KEY_SHAPE * rowHeight()));
}

qreal KeyboardView::keysLeft() const { return std::round((width() - keysWidth()) / 2); }

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
        m_emoji.layout(keyboardTop() + topBorder(), QRectF(0, top.top(), width(), bottom.bottom() - top.top()), rowRect(3));
        return;
    }
    const Rows &rs = rows();
    for (int i = 0; i < rs.size(); ++i) {
        const QRectF r = rowRect(i);
        qreal units = 0;
        for (const Slot &s : rs[i]) units += s.weight;
        const qreal x0 = keysLeft(), w = keysWidth();
        qreal left = x0, sofar = 0;
        for (const Slot &s : rs[i]) {
            sofar += s.weight;
            // Whole pixels, so neighbouring caps never share a blurred edge.
            const qreal right = x0 + std::round(w * sofar / units);
            m_zones.append(Zone{ s, QRectF(left, r.top(), right - left, r.height()) });
            left = right;
        }
    }
    fitLetters();
}

void KeyboardView::show(Page p)
{
    if (m_page == Page::Emoji && p != Page::Emoji) m_emoji.close();
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
    // The emoji page takes the suggestion bar's place for its tabs.
    const bool emojiPage = m_page == Page::Emoji;
    const qreal deckTop = emojiPage ? top : top + barHeight();
    m_painter.deck(p, QRectF(0, deckTop, width(), height() - deckTop));
    if (barShown() && !emojiPage) drawBar(p, QRectF(0, top, width(), barHeight()));
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
    case Kind::Char: drawCap(p, z, box); break;
    case Kind::Space:
        if (m_cursorMode) m_painter.icon(p, Painter::Icon::Cursor, c.x(), c.y(), iconSize);
        else drawLanguageFlash(p, box);
        break;
    case Kind::Shift:
        // A number field has no capitals and no alternates: those keys go blank.
        if (digitMode()) break;
        // On, Shift takes the style's colour: the Pre's orange, or in the White
        // style white, where an outline says it is off.
        if (m_settings.style == Style::White)
            m_painter.icon(p, m_shift == Shift::Off ? Painter::Icon::ShiftOpen
                              : m_shift == Shift::Once ? Painter::Icon::ShiftOn : Painter::Icon::ShiftLock,
                           c.x(), c.y(), iconSize, QColor(Qt::white));
        else
            m_painter.icon(p, m_shift == Shift::Off ? Painter::Icon::Shift
                              : m_shift == Shift::Once ? Painter::Icon::ShiftOn : Painter::Icon::ShiftLock,
                           c.x(), c.y(), iconSize);
        break;
    case Kind::Backspace: m_painter.icon(p, Painter::Icon::Backspace, c.x(), c.y(), iconSize); break;
    case Kind::Return: m_painter.icon(p, Painter::Icon::Return, c.x(), c.y(), iconSize); break;
    case Kind::Globe: m_painter.icon(p, Painter::Icon::Globe, c.x(), c.y(), iconSize); break;
    case Kind::Emoji: m_painter.icon(p, Painter::Icon::Emoji, c.x(), c.y(), iconSize); break;
    case Kind::Orange: if (!digitMode()) m_painter.orange(p, box, m_settings.style, m_alt); break;
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
    return m_settings.holdForMore && !key.extended.isEmpty() && !digitMode();
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
    const QString name = m_settings.language.name;
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
    // The languages the space bar goes through, not every one there is.
    for (const Language &l : m_settings.languages) {
        const qreal w = fm.horizontalAdvance(l.name);
        if (w > widest) { widest = w; longest = l.name; }
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
qreal KeyboardView::letterSize() const { return std::min(rowHeight() * 0.5, keysWidth() / 10 * 0.56); }

//! Inset of the glyphs' ink from the key's edges.
qreal KeyboardView::inset(const QRectF &r) const { return 2.5 * m_dp + r.width() * 0.08; }

void KeyboardView::setDigits(const QString &allowed)
{
    if (allowed == m_digits) return;
    m_digits = allowed;
    m_alt = Alt::Off;
    layoutKeys();
    update();
}

/*!
 * What a character key shows: letters as capitals, as printed on the Pre, or
 * small till Shift is on when the owner wants them to follow it; in a number
 * field, only what the field takes.
 */
QString KeyboardView::capText(const Key &key) const
{
    if (m_page == Page::Symbols) return key.text;
    if (digitMode()) return digitOf(key);
    if (m_settings.followShift && m_shift == Shift::Off) return key.text;
    return key.text.toUpper();
}

//! The alternate printed in [key]'s corner: none in a number field, where it is the key's one character.
QString KeyboardView::altText(const Key &key) const
{
    return digitMode() ? QString() : key.alt;
}

/*!
 * Every way [key]'s letter can look on this page: small and capital when the
 * letters follow Shift. The page is fitted for all of them, so pressing Shift
 * never moves or resizes a letter.
 */
QStringList KeyboardView::faces(const Key &key) const
{
    if (m_page == Page::Letters && !digitMode() && m_settings.followShift) {
        const QString up = key.text.toUpper();
        return up == key.text ? QStringList{ up } : QStringList{ key.text, up };
    }
    return { capText(key) };
}

//! In a number field, what [key] types: its alternate, or itself, where the field takes that; else nothing.
QString KeyboardView::digitOf(const Key &key) const
{
    auto taken = [this](const QString &s) {
        for (QChar c : s) if (!m_digits.contains(c)) return false;
        return !s.isEmpty();
    };
    if (taken(key.alt)) return key.alt;
    if (taken(key.text)) return key.text;
    return QString();
}

/*!
 * The printing of every character key on the page, as one: one size for all
 * of them, and one baseline per row, so a row's letters stand on one line
 * whichever keys have an alternate. The size is the largest at which every
 * letter and alternate stays inside its key's face.
 */
void KeyboardView::fitLetters()
{
    // The same page at the same size comes out the same: work it out once.
    QString id = QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(m_settings.language.id).arg(int(m_page))
                     .arg(width()).arg(rowHeight()).arg(m_dp).arg(m_landscape)
                     .arg(m_settings.followShift).arg(digitMode() ? m_digits : QString());
    // The keys as laid out, whole pixels each: two rows of the same nominal
    // height can come out a pixel apart.
    for (const Zone &z : std::as_const(m_zones))
        id += QStringLiteral("|%1x%2").arg(z.rect.width()).arg(z.rect.height());
    const auto known = m_fits.constFind(id);
    if (known != m_fits.constEnd() && known->baselines.size() == m_zones.size()) {
        m_altStep = known->altStep;
        for (int k = 0; k < m_zones.size(); ++k) {
            m_zones[k].letterPx = known->px;
            m_zones[k].baseline = known->baselines[k];
        }
        return;
    }

    // Start just above the size the widest letter allows, rather than
    // trying every size down from the full one: a letter's width grows with
    // its size, a pixel or so either way for hinting.
    const qreal full = letterSize();
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(full)));
    const QFontMetricsF fm(f);
    qreal px = full;
    for (const Zone &z : std::as_const(m_zones)) {
        if (!z.slot.drawn || z.slot.key->kind != Kind::Char) continue;
        const qreal room = z.rect.width() - 2 * inset(z.rect);
        for (const QString &face : faces(*z.slot.key)) {
            const qreal w = fm.tightBoundingRect(face).width();
            if (w > room && w > 0) px = std::min(px, std::floor(full * room / w) + 2);
        }
    }
    // The largest that fits: halve the range between the start and the
    // smallest, in whole pixels down from the start.
    if (!placeLetters(px)) {
        int fits = std::max(0, int(std::ceil(px - MIN_LETTER_PX))), fails = 0;
        while (fits - fails > 1) {
            const int mid = (fits + fails) / 2;
            if (placeLetters(px - mid)) fits = mid;
            else fails = mid;
        }
        px -= fits;
        placeLetters(px);
    }

    Fit fit;
    fit.px = px;
    fit.altStep = m_altStep;
    for (const Zone &z : std::as_const(m_zones)) fit.baselines << z.baseline;
    if (m_fits.size() > 64) m_fits.clear();
    m_fits.insert(id, fit);
}

/*!
 * Lays the page's letters out at [px]; false if one of them, or an
 * alternate, would leave its key's face or run into the other.
 *
 * In a row with alternates the letters sit low, as on the Pre, their line
 * taken from a flat-bottomed capital so a Q's tail hangs below it; a row
 * without any has its capitals centred. Where a letter hangs lower than the
 * face allows (Arabic's ر and و, Hebrew's ק and ף), every row comes up
 * together. A symbol is lifted until its ink rests on the line; in a row
 * without alternates (the Sym page) it is centred on the face by its own ink
 * instead, so < > ~ { } line up by their middles. Marks whose place is their
 * meaning (see placedMark) stay on the line.
 */
bool KeyboardView::placeLetters(qreal px)
{
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(px)));
    const QFontMetricsF fm(f);
    QFont af = m_bold;
    af.setPixelSize(std::max(1, int(px * ALT_SCALE)));
    const QFontMetricsF am(af);
    const QRectF capital = fm.tightBoundingRect(QStringLiteral("H"));
    const qreal margin = INK_MARGIN * m_dp;

    auto isChar = [](const Zone &z) { return z.slot.drawn && z.slot.key->kind == Kind::Char; };
    auto allLetters = [](const QString &s) {
        for (QChar ch : s) if (!ch.isLetter() && !ch.isMark()) return false;
        return !s.isEmpty();
    };
    // Each key in its own coordinates: what fits depends on its size alone,
    // never on where it is, so the answer is the same wherever it is asked.
    auto local = [](const Zone &z) { return QRectF(0, 0, z.rect.width(), z.rect.height()); };
    //! Where ink may go on [z]: its face, a margin in.
    auto faceOf = [&](const Zone &z) {
        return m_painter.capFace(local(z)).adjusted(margin, margin, -margin, -margin);
    };

    // Every key's ink at this size, once: each of its faces, and its alternate.
    QVector<QStringList> texts;
    QVector<QVector<Ink>> letters;
    QVector<Ink> alts;
    texts.reserve(m_zones.size());
    letters.reserve(m_zones.size());
    alts.reserve(m_zones.size());
    for (Zone &z : m_zones) {
        const bool c = isChar(z);
        texts << (c ? faces(*z.slot.key) : QStringList());
        QVector<Ink> inks;
        for (const QString &t : std::as_const(texts.last())) inks << Ink(t, f);
        letters << inks;
        alts << Ink(c ? altText(*z.slot.key) : QString(), af);
        if (c) z.letterPx = px;
    }

    // How low the page's letters reach below their line.
    qreal hangs = capital.bottom();
    for (int k = 0; k < m_zones.size(); ++k)
        for (int j = 0; j < texts[k].size(); ++j)
            if (allLetters(texts[k][j])) hangs = std::max(hangs, letters[k][j].box.bottom());

    /*
     * The alternates go in their corners, but a narrow key's face curves away
     * there: bring them all in by the same step, the least that puts every one
     * of them inside its face, so they still line up across the row.
     */
    // Every row is laid out even when something does not fit, so the page is
    // always whole: the smallest size is drawn whether it fits or not.
    bool fits = true;
    for (m_altStep = 0;; m_altStep += 0.5 * m_dp) {
        if (m_altStep > MAX_ALT_STEP * m_dp) {
            m_altStep = MAX_ALT_STEP * m_dp;
            fits = false;
            break;
        }
        bool in = true;
        for (int k = 0; in && k < m_zones.size(); ++k) {
            const Zone &z = m_zones[k];
            if (!isChar(z) || altText(*z.slot.key).isEmpty()) continue;
            const QRectF face = faceOf(z);
            in = inside(alts[k], altAt(z, local(z), am), face, m_painter.capRadius(face));
        }
        if (in) break;
    }

    for (int a = 0; a < m_zones.size();) {
        int b = a;
        while (b < m_zones.size() && m_zones[b].rect.top() == m_zones[a].rect.top()) ++b;
        bool low = false;
        qreal i = 0;
        for (int k = a; k < b; ++k) {
            if (!isChar(m_zones[k])) continue;
            low = low || !altText(*m_zones[k].slot.key).isEmpty();
            i = std::max(i, inset(m_zones[k].rect));
        }
        const qreal h = m_zones[a].rect.height();
        const QRectF rowFace = faceOf(m_zones[a]);
        const qreal floor = rowFace.bottom();
        // Centred on the face, not the key: the key's shadow is below it.
        const qreal middle = rowFace.center().y();
        qreal line = low ? h - i - capital.bottom() : middle - (capital.top() + capital.bottom()) / 2;
        line = std::min(line, floor - hangs);

        for (int k = a; k < b; ++k) {
            Zone &z = m_zones[k];
            if (!isChar(z)) continue;
            const Key &key = *z.slot.key;
            const QString s = capText(key);
            const QRectF ink = fm.tightBoundingRect(s);
            if (allLetters(s) || s.isEmpty()) z.baseline = line;
            else if (low) z.baseline = std::min(line, line + capital.bottom() - ink.bottom());
            else if (placedMark(s)) z.baseline = line;
            else z.baseline = middle - (ink.top() + ink.bottom()) / 2;

            // The glyphs themselves, not their boxes: a box's corner can be off a
            // rounded corner of the face that the letter never comes near.
            const QRectF face = faceOf(z);
            const qreal radius = m_painter.capRadius(face);
            // Once something does not fit, the rest is only laid out, not checked.
            const bool alt = !altText(key).isEmpty();
            for (int j = 0; fits && j < texts[k].size(); ++j) {
                // A blank key (a number field's letters) has nothing to fit.
                if (texts[k][j].isEmpty()) continue;
                const QPointF at = letterAt(z, local(z), fm, texts[k][j]);
                if (fm.tightBoundingRect(texts[k][j]).width() > z.rect.width() - 2 * inset(z.rect)
                    || !inside(letters[k][j], at, face, radius)
                    || (alt && touch(alts[k], altAt(z, local(z), am), letters[k][j], at)))
                    fits = false;
            }
        }
        a = b;
    }
    return fits;
}

/*!
 * A mark whose height on the line is what it means - a full stop, a comma, an
 * underscore, quotes, a caret, a degree sign - so it is not centred like the
 * other symbols: a centred full stop would read as a middle dot.
 */
bool KeyboardView::placedMark(const QString &s)
{
    static const QString marks = QStringLiteral(".,_'\"`^°´¨¸‚„‘’“”…˚");
    for (QChar c : s) if (!marks.contains(c)) return false;
    return !s.isEmpty();
}

/*!
 * Where [z]'s letter is drawn from in [box]: on its row's line, centred in the
 * key - by its advance and nudged a little away from an alternate, as on the
 * Pre, or by its ink where there is none - and moved in off the key's sides.
 */
QPointF KeyboardView::letterAt(const Zone &z, const QRectF &box, const QFontMetricsF &fm) const
{
    return letterAt(z, box, fm, capText(*z.slot.key));
}

//! Where [z]'s letter would be drawn from in [box] if it showed [s].
QPointF KeyboardView::letterAt(const Zone &z, const QRectF &box, const QFontMetricsF &fm, const QString &s) const
{
    const Key &key = *z.slot.key;
    const QRectF ink = fm.tightBoundingRect(s);
    qreal x;
    if (altText(key).isEmpty()) {
        x = box.center().x() - (ink.left() + ink.right()) / 2;
    } else {
        const qreal nudge = box.width() * LETTER_NUDGE * (key.altPos == AltPos::Left ? 1 : -1);
        x = box.center().x() + nudge - fm.horizontalAdvance(s) / 2;
    }
    const qreal i = inset(box);
    x = std::min(std::max(x, box.left() + i - ink.left()), box.right() - i - ink.right());
    return QPointF(x, box.top() + z.baseline);
}

/*!
 * Where [z]'s alternate is drawn from in [box]: in its top corner, its edge on
 * the inset (and in by the page's step, see placeLetters), centred on a capital's height there so "-" and "'" line up with
 * the digits.
 */
QPointF KeyboardView::altAt(const Zone &z, const QRectF &box, const QFontMetricsF &fm) const
{
    const Key &key = *z.slot.key;
    const QRectF ink = fm.tightBoundingRect(altText(key));
    const qreal i = inset(box);
    const qreal x = key.altPos == AltPos::Left ? box.left() + i + m_altStep - ink.left()
                                               : box.right() - i - m_altStep - ink.right();
    const qreal y = box.top() + i + m_altStep + int(z.letterPx * ALT_SCALE) * CAP_HEIGHT / 2
                    - (ink.top() + ink.bottom()) / 2;
    return QPointF(x, y);
}

/*!
 * A character key's printing, as fitLetters laid it out: a key with an
 * alternate puts it in one top corner and the letter low in the key, just off
 * centre away from it.
 */
void KeyboardView::drawCap(QPainter &p, const Zone &z, const QRectF &box)
{
    const Key &key = *z.slot.key;
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(z.letterPx)));
    p.setFont(f);
    const QString s = capText(key);
    const QString a = altText(key);
    // In a number field its digits are the ones printed in colour, as the alternates were.
    const bool coloured = key.accent || (digitMode() && s.length() == 1 && s[0].isDigit());
    QColor letter = coloured ? accent(m_settings.style) : QColor(Qt::white);
    // The letter steps back while the orange key says the alternate is what will be typed.
    if (!a.isEmpty() && m_alt != Alt::Off) letter.setAlpha(89);
    p.setPen(letter);
    p.drawText(letterAt(z, box, QFontMetricsF(f), s), s);
    if (a.isEmpty()) return;

    f.setPixelSize(std::max(1, int(z.letterPx * ALT_SCALE)));
    p.setFont(f);
    p.setPen(a.length() == 1 && a[0].isDigit() ? accent(m_settings.style) : QColor(Qt::white));
    p.drawText(altAt(z, box, QFontMetricsF(f)), a);
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
    if (digitMode()) return digitOf(key);
    if (m_alt != Alt::Off && !key.alt.isEmpty()) return key.alt;
    if (m_shift == Shift::Off) return key.text;
    return key.shifted.isEmpty() ? key.text.toUpper() : key.shifted;
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
    p.setPen(z.slot.key->accent || ((m_alt != Alt::Off || digitMode()) && s.length() == 1 && s[0].isDigit())
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
        if (!key.extendedShifted.isEmpty()) return key.extendedShifted;
        QStringList up;
        for (const QString &s : key.extended) up << s.toUpper();
        return up;
    }
    return key.extended;
}

qreal KeyboardView::cellW() const { return std::max(keysWidth() / 10 * 0.92, 34 * m_dp); }

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
    if (m_searching) { drawSearch(p, r); return; }
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
        // Shown as shift will put it in: a capital first, or all capitals when locked.
        const QString w = withShift(m_words[i]);
        if (w.isEmpty()) continue;
        // The likeliest word, in the middle, in the Pre's orange in either style.
        QFont wf = m_bold;
        wf.setPixelSize(std::max(1, int(fitted(w, size, cw - 12 * m_dp))));
        p.setFont(wf);
        p.setPen(i == 1 ? accent(Style::Orange) : QColor(Qt::white));
        p.drawText(QPointF(cellRect.center().x() - QFontMetricsF(wf).horizontalAdvance(w) / 2, baseline), w);
    }
}

// --- Searching KLIPY ---------------------------------------------------------------------

/*!
 * A search for GIFs or stickers is being typed: the letters show, and what they
 * type goes into the search field drawn in the bar, not into the application.
 * Return searches; the bar's arrow, or the emoji key, goes back.
 */
void KeyboardView::startSearch(Klipy::Kind kind, const QString &query)
{
    barChange([&] {
        m_searching = true;
        m_searchKind = kind;
        m_searchText = query;
    });
    m_shift = Shift::Off;
    m_autoShift = false;
    show(Page::Letters);
}

//! Back to the GIFs or stickers: what was typed is searched for ([go]), or put away.
void KeyboardView::endSearch(bool go)
{
    if (!m_searching) return;
    const Klipy::Kind kind = m_searchKind;
    const QString q = m_searchText;
    barChange([&] { m_searching = false; m_searchText.clear(); });
    show(Page::Emoji);
    if (go) m_emoji.searched(kind, q);
}

void KeyboardView::send(const QString &s)
{
    if (m_searching) { m_searchText += s; update(); return; }
    if (m_controller) m_controller->onText(s);
}

void KeyboardView::sendBackspace(bool word)
{
    if (!m_searching) { if (m_controller) m_controller->onBackspace(word); return; }
    if (word) {
        QString t = m_searchText;
        while (t.endsWith(QLatin1Char(' '))) t.chop(1);
        m_searchText.truncate(t.lastIndexOf(QLatin1Char(' ')) + 1);
    } else if (!m_searchText.isEmpty()) {
        m_searchText.chop(1);
    }
    update();
}

void KeyboardView::barChange(const std::function<void()> &change)
{
    const qreal before = keysHeight();
    change();
    if (keysHeight() != before) {
        layoutKeys();
        updatePopupRect();
        Q_EMIT geometryChanged();
    }
    update();
}

void KeyboardView::hidden()
{
    if (m_page == Page::Emoji) m_emoji.close();
}

void KeyboardView::mediaSays(Klipy::Kind kind, const QString &s, int ms)
{
    m_emoji.say(kind, s, ms);
}

//! Where the search's back arrow is, at the bar's left.
QRectF KeyboardView::searchBack(const QRectF &bar) const
{
    return QRectF(bar.left(), bar.top(), bar.height() * 1.3, bar.height());
}

//! The search being typed: a back arrow, then the field, "Search KLIPY" till something is typed.
void KeyboardView::drawSearch(QPainter &p, const QRectF &r)
{
    const QRectF back = searchBack(r);
    if (m_barPressed == 0) p.fillRect(back, BAR_PRESSED);
    m_painter.icon(p, Painter::Icon::Backspace, back.center().x(), back.center().y(), r.height() * 0.42);
    const qreal pad = r.height() * 0.14;
    const QRectF field(back.right(), r.top() + pad, r.right() - 8 * m_dp - back.right(), r.height() - 2 * pad);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 40));
    p.drawRoundedRect(field, field.height() / 2, field.height() / 2);
    m_painter.icon(p, Painter::Icon::Search, field.left() + field.height() * 0.6, field.center().y(), field.height() * 0.5,
                   QColor(255, 255, 255, 170));
    const qreal left = field.left() + field.height() * 1.1;
    const qreal room = field.right() - left - field.height() * 0.5;
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(field.height() * 0.5)));
    p.setFont(f);
    const QFontMetricsF fm(f);
    const qreal baseline = field.center().y() + (fm.ascent() - fm.descent()) / 2;
    const QRectF caret(0, field.top() + field.height() * 0.22, 2 * m_dp, field.height() * 0.56);
    if (m_searchText.isEmpty()) {
        p.setPen(QColor(255, 255, 255, 130));
        p.drawText(QPointF(left, baseline), QStringLiteral("Search KLIPY"));
        p.fillRect(caret.translated(left - m_dp, 0), accent(Style::Orange));
    } else {
        // The end of what's typed, where the cursor is, when it's too long to show whole.
        QString shown = m_searchText;
        while (shown.length() > 1 && fm.horizontalAdvance(shown) > room) shown.remove(0, 1);
        p.setPen(Qt::white);
        p.drawText(QPointF(left, baseline), shown);
        p.fillRect(caret.translated(left + fm.horizontalAdvance(shown) + m_dp, 0), accent(Style::Orange));
    }
    p.restore();
}

// --- Holding a word in the bar: add it to the dictionary -------------------------------

void KeyboardView::onBarHold()
{
    const int i = m_barPressed;
    if (i < 0 || (i == 2 && m_pasteOffered)) return;
    const QString w = m_words.value(i);
    if (w.isEmpty() || !m_controller) return;
    // A word the dictionary lacks may be added; one it has (a typo it knows: "yest"), deleted.
    if (m_controller->canAdd(w)) m_askingRemove = false;
    else if (m_controller->canRemove(w)) m_askingRemove = true;
    else return;
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
    m_askingRemove = false;
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
                if (h == 1 && !word.isEmpty() && m_controller) {
                    if (m_askingRemove) m_controller->onRemoveWord(word);
                    else m_controller->onAddWord(word);
                }
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
    const QString question = (m_askingRemove ? QStringLiteral("Delete “%1” from dictionary?")
                                             : QStringLiteral("Add “%1” to dictionary?")).arg(m_asking);
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
 * The globe on the Sym page: a list of the languages checked, to go straight to
 * one. (Holding the globe switches the style.)
 */
void KeyboardView::showMenu()
{
    m_menu.clear();
    for (const Language &l : m_settings.languages)
        m_menu << l.name;
    int globe = -1;
    for (int i = 0; i < m_zones.size(); ++i)
        if (m_zones[i].slot.key->kind == Kind::Globe) { globe = i; break; }
    const qreal rowH = cellH();
    const qreal pad = popupPad();
    QFont f = m_bold;
    f.setPixelSize(std::max(1, int(rowHeight() * 0.36)));
    qreal textW = 0;
    for (const QString &s : m_menu) textW = std::max(textW, QFontMetricsF(f).horizontalAdvance(s));
    // As many rows as the keyboard is tall, and as many columns as that takes:
    // every language stays on screen and in reach, however many are enabled.
    const int n = std::max(1, int(m_menu.size()));
    const int fits = std::max(1, int((height() - 2 * pad) / rowH));
    const int cols = (n + fits - 1) / fits;
    m_menuRows = (n + cols - 1) / cols;
    m_menuColW = std::min(textW + 2 * pad + 16 * m_dp, (width() - 2 * pad) / cols);
    const qreal w = cols * m_menuColW + 2 * pad;
    const qreal h = m_menuRows * rowH + 2 * pad;
    const QRectF key = globe >= 0 ? m_zones[globe].rect : QRectF(0, keyboardTop(), 0, 0);
    // Above the globe where there is room; otherwise as low as it goes over the keys.
    const qreal top = key.top() - h >= 0 ? key.top() - h : std::max<qreal>(0, height() - h);
    m_menuFrame = QRectF(std::max<qreal>(0, std::min(key.left(), width() - w)), top, w, h);
    m_menuPressed = -1;
    m_menuDown = false;
    updatePopupRect();
    update();
}

QRectF KeyboardView::menuRow(int i) const
{
    const int perColumn = std::max(1, m_menuRows);
    const int column = i / perColumn;
    const int row = i % perColumn;
    return QRectF(m_menuFrame.left() + popupPad() + column * m_menuColW,
                  m_menuFrame.top() + popupPad() + row * cellH(),
                  m_menuColW, cellH());
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
        if (m_controller && i >= 0 && i < languages) m_controller->onUseLanguage(m_settings.languages[i].id);
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
    return m_settings.swipe && m_wordsAllowed && m_page == Page::Letters && !digitMode() && !m_searching && m_alt == Alt::Off &&
           k.kind == Kind::Char && k.text.length() == 1 && k.text[0].isLetter();
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

//! [w] as shift would type it now.
QString KeyboardView::withShift(const QString &w) const
{
    if (w.isEmpty() || m_shift == Shift::Off) return w;
    if (m_shift == Shift::Lock) return QLocale(m_settings.language.id).toUpper(w);
    return w.left(1).toUpper() + w.mid(1);
}

bool KeyboardView::swipeLayout(SwipeDecoder::Layout &out) const
{
    out = SwipeDecoder::Layout();
    QVector<float> widths;
    for (const Zone &z : m_zones) {
        const Key &k = *z.slot.key;
        if (!z.slot.drawn || k.kind != Kind::Char || k.text.length() != 1 || !k.text[0].isLetter()) continue;
        const QChar c = k.text[0].toLower();
        if (out.keys.contains(c)) continue;
        out.keys += c;
        out.x << float(z.rect.center().x());
        out.y << float(z.rect.center().y());
        widths << float(z.rect.width());
    }
    if (widths.isEmpty()) return false;
    // A letter key's width: most are the same, a few (a wide last key) are not.
    std::nth_element(widths.begin(), widths.begin() + widths.size() / 2, widths.end());
    out.keyWidth = widths[widths.size() / 2];
    out.keyHeight = float(rowHeight());
    return out.isValid();
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
    // Below the last row (the bottom pad), in a rounding gap, or beside the keys
    // in landscape: the nearest key in that row.
    const qreal yy = std::max(m_zones.first().rect.top(), std::min(y, m_zones.last().rect.bottom() - 1));
    const qreal xx = std::max(keysLeft(), std::min(x, keysLeft() + keysWidth() - 1));
    for (int i = 0; i < m_zones.size(); ++i) {
        const QRectF &r = m_zones[i].rect;
        if (yy >= r.top() && yy < r.bottom() && xx >= r.left() && xx < r.right()) return i;
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

void KeyboardView::stopRepeat(bool lifted)
{
    m_repeatTimer.stop();
    if (m_deleting) {
        m_deleting = false;
        if (m_controller) m_controller->onBackspaceHeld(false, lifted);
    }
}

void KeyboardView::onRepeat()
{
    if (m_pressed < 0 || m_zones[m_pressed].slot.key->kind != Kind::Backspace) return;
    m_repeats++;
    const bool word = m_repeats * LETTER_REPEAT > WORD_DELETE_AFTER;
    sendBackspace(word);
    m_repeatTimer.start(word ? WORD_REPEAT : LETTER_REPEAT);
}

void KeyboardView::onHold()
{
    if (m_pressed < 0) return;
    const Zone z = m_zones[m_pressed];
    const Key &key = *z.slot.key;
    switch (key.kind) {
    case Kind::Space:
        if (m_searching) break;
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
    // A new press never inherits an accent popup: what it types is its own key.
    if (!m_extended.isEmpty()) {
        m_extended.clear();
        m_extendedFrame = QRectF();
        updatePopupRect();
    }
    m_pressed = zone;
    m_held = false;
    m_preview = (zone >= 0 && m_zones[zone].slot.key->kind == Kind::Char
                 && !output(*m_zones[zone].slot.key).isEmpty()) ? zone : -1;
    if (zone < 0) { update(); return; }
    const Key &key = *m_zones[zone].slot.key;
    switch (key.kind) {
    case Kind::Backspace:
        m_repeats = 0;
        if (!m_searching) {
            // Down here, deleted as it comes up or repeated while held, by the
            // editor, as LuneOS's own backspace is.
            m_deleting = true;
            if (m_controller) m_controller->onBackspaceHeld(true);
            break;
        }
        sendBackspace(false);
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
    if (std::abs(dx) < keysWidth() / 10 * FLICK_KEYS || std::abs(dx) < 2 * std::abs(y - m_downY)) return false;
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
    stopRepeat(up);
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
    // In a number field shift and the orange key have nothing to do.
    if (digitMode() && (key.kind == Kind::Shift || key.kind == Kind::Orange)) return;
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
    case Kind::Emoji: if (m_searching) endSearch(false); else show(Page::Emoji); break;
    case Kind::Backspace: break;     // sent on the way down, and repeated since
    case Kind::Return: if (m_searching) endSearch(true); else if (m_controller) m_controller->onEnter(); break;
    case Kind::Globe: showMenu(); break;
    case Kind::Space: send(QStringLiteral(" ")); break;
    case Kind::Char: {
        const QString s = output(key);
        if (!s.isEmpty()) type(s, !digitMode() && m_alt != Alt::Off && !key.alt.isEmpty());
        break;
    }
    }
}

void KeyboardView::type(const QString &s, bool usedAlt)
{
    send(s);
    // A one-shot spent is not half of a double tap.
    if (m_shift == Shift::Once) { m_shift = Shift::Off; m_autoShift = false; m_lastShiftTap = NEVER; }
    if (usedAlt && m_alt == Alt::Once) { m_alt = Alt::Off; m_lastAltTap = NEVER; }
}

void KeyboardView::updateCaps()
{
    if (m_page != Page::Letters || m_searching || m_shift == Shift::Lock || (m_shift == Shift::Once && !m_autoShift)) return;
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
    if (m_searching) barChange([this] { m_searching = false; m_searchText.clear(); });
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
    if (bc >= 0 && m_searching) {
        // The search field: only its back arrow does anything.
        if (m_finger >= 0) lift(!(m_swiping || m_cursorMode || !m_extended.isEmpty() || m_held));
        m_finger = id;
        m_barPressed = searchBack(QRectF(0, keyboardTop(), width(), barHeight())).contains(QPointF(x, y)) ? 0 : 1;
        if (m_barPressed == 0) keyFeedback();
        update();
        return true;
    }
    if (bc >= 0) {
        // A key still held by another finger is finished first, as a finger
        // landing on another key does: typed if it was a tap, otherwise let go.
        if (m_finger >= 0) lift(!(m_swiping || m_cursorMode || !m_extended.isEmpty() || m_held));
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
    if (m_barPressed >= 0 && m_searching) {
        const int was = m_barPressed;
        m_barPressed = -1;
        if (was == 0 && searchBack(QRectF(0, keyboardTop(), width(), barHeight())).contains(QPointF(x, y))) endSearch(false);
        update();
        return;
    }
    if (m_barPressed >= 0) {
        m_barHoldTimer.stop();
        const int i = m_barPressed;
        m_barPressed = -1;
        if (barCell(x, y) == i && m_controller) {
            if (i == 2 && m_pasteOffered) m_controller->onPaste();
            else if (!m_words[i].isEmpty()) {
                m_controller->onPick(i, capsNow());
                // Shift for one letter is spent on the word, as a letter would spend it.
                if (m_shift == Shift::Once) { m_shift = Shift::Off; m_autoShift = false; m_lastShiftTap = NEVER; }
            }
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
