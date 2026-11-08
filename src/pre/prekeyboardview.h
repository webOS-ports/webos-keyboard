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

#ifndef PRE_KEYBOARDVIEW_H
#define PRE_KEYBOARDVIEW_H

#include "preemojipanel.h"
#include "prekeymap.h"
#include "prepainter.h"
#include "presettings.h"
#include "preswipedecoder.h"

#include <QElapsedTimer>
#include <QFont>
#include <QPointer>
#include <QQuickPaintedItem>
#include <QStringList>
#include <QTimer>
#include <QVector>

namespace Pre {

class Controller;

/*!
 * The Palm Pre's keyboard as a Quick item, in one of two styles: Orange, with the
 * alternates and the orange key printed orange as on the Pre, or White, all
 * printing white.
 *
 * The item is taller than its keys by headroom, which is transparent: the key
 * preview and the press-and-hold balloon are drawn there. Only the keys are the
 * panel the application makes room for (keysRect); a popup up there is added to
 * the window's input mask while it shows (popupRect).
 */
class KeyboardView : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QObject *controller READ controllerObject WRITE setControllerObject NOTIFY controllerChanged)
    //! Device pixels per dp: the screen's dpi / 160, as Android counts.
    Q_PROPERTY(qreal dp READ dp WRITE setDp NOTIFY dpChanged)
    //! The keys' height, the bar included: what the application makes room for.
    Q_PROPERTY(qreal keysHeight READ keysHeight NOTIFY geometryChanged)
    //! The room above the keys for popups.
    Q_PROPERTY(qreal headroom READ headroom NOTIFY geometryChanged)
    //! Where a popup is drawn now, in the item's coordinates; empty when there is none.
    Q_PROPERTY(QRectF popupRect READ popupRect NOTIFY popupRectChanged)
    Q_PROPERTY(bool landscape READ landscape WRITE setLandscape NOTIFY geometryChanged)

public:
    explicit KeyboardView(QQuickItem *parent = nullptr);
    ~KeyboardView() override;

    QObject *controllerObject() const;
    void setControllerObject(QObject *controller);
    qreal dp() const { return m_dp; }
    void setDp(qreal dp);
    bool landscape() const { return m_landscape; }
    void setLandscape(bool landscape);
    qreal keysHeight() const;
    qreal headroom() const;
    QRectF popupRect() const { return m_popupRect; }

    void paint(QPainter *painter) override;

    // What the controller tells the view.
    void applySettings(const Settings &s);
    const Settings &settings() const { return m_settings; }
    //! Three words, left to right; the middle one is the likeliest. "" leaves a place empty.
    void setSuggestions(const QStringList &words);
    void setPasteOffered(bool offered);
    bool pasteOffered() const { return m_pasteOffered; }
    //! Swiping may happen here: switched on, and a field for words (not a password, say).
    void setWordsAllowed(bool allowed) { m_wordsAllowed = allowed; }
    //! The language just changed: its name shows bright on the space bar, then fades.
    void flashLanguage();
    //! Opens the add-to-dictionary popup for [word] from outside the bar.
    void ask(const QString &word);
    //! The field has moved on: offer a capital where it wants one, unless shift was chosen by hand.
    void updateCaps();
    //! A new field: start from the letters, nothing held.
    void reset();
    //! Where the letters are, for the swipe decoder; false when they aren't all laid out.
    bool swipeLayout(SwipeDecoder::Layout &out) const;

    static const int CAPS_NONE = 0;
    static const int CAPS_FIRST = 1;
    static const int CAPS_ALL = 2;

Q_SIGNALS:
    void controllerChanged();
    void dpChanged();
    void geometryChanged();
    void popupRectChanged();
    //! A key went down and the owner wants a click: QML plays it.
    void keyClick();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void touchEvent(QTouchEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void touchUngrabEvent() override;
    void mouseUngrabEvent() override;

private:
    enum class Shift { Off, Once, Lock };
    enum class Page { Letters, Symbols, Emoji };

    struct Zone { Slot slot; QRectF rect; };

    // Sizes
    qreal rowHeight() const;
    qreal topBorder() const;
    qreal bottomPad() const;
    bool barShown() const;
    qreal barHeight() const;
    qreal cellH() const;
    qreal popupPad() const;
    qreal keyboardTop() const;
    QRectF rowRect(int i) const;
    void layoutKeys();
    const Rows &rows() const;
    void show(Page p);

    // Drawing
    void drawKey(QPainter &p, const Zone &z);
    void cornerIcon(QPainter &p, Corner corner, const QRectF &r);
    bool holds(const Key &key) const;
    void holdDot(QPainter &p, const Key &key, const QRectF &r);
    void drawLanguageFlash(QPainter &p, const QRectF &box);
    void functionLabel(QPainter &p, const QString &s, const QRectF &box);
    qreal letterSize() const;
    qreal inset(const QRectF &r) const;
    void drawCap(QPainter &p, const Key &key, const QRectF &r);
    void centred(QPainter &p, const QString &s, const QRectF &r);
    qreal fitted(const QString &s, qreal size, qreal room) const;
    QString output(const Key &key) const;
    void drawPreview(QPainter &p);
    void bubble(QPainter &p, const QRectF &r);
    void drawExtended(QPainter &p);
    void drawBar(QPainter &p, const QRectF &r);
    void drawAsk(QPainter &p);
    void drawMenu(QPainter &p);
    void drawTrail(QPainter &p);
    void updatePopupRect();

    // Press and hold
    QStringList extendedFor(const Key &key) const;
    qreal cellW() const;
    static int perLine(int n) { return n > 6 ? (n + 1) / 2 : n; }
    void showExtended(const Zone &z);
    QRectF cell(int i, int count, const QRectF &frame) const;
    int extendedAt(qreal x, qreal y) const;
    void startCursor(qreal x, qreal y);
    void moveCursor(qreal x, qreal y);

    // The globe's menu: the keyboards, and the languages checked.
    void showMenu();
    int menuAt(qreal x, qreal y) const;
    QRectF menuRow(int i) const;

    // Touch
    int zoneAt(qreal x, qreal y) const;
    int barCell(qreal x, qreal y) const;
    bool pointerDown(int id, qreal x, qreal y);
    void pointerMove(int id, qreal x, qreal y);
    void pointerUp(int id, qreal x, qreal y);
    void pointerCancel();
    bool down(qreal x, qreal y);
    void move(qreal x, qreal y);
    bool flickedLanguage(qreal x, qreal y);
    void lift(bool up);
    void commit();
    void type(const QString &s, bool usedAlt);
    void press(int zone);
    void onHold();
    void onRepeat();
    void onBarHold();
    void stopRepeat();
    void keyFeedback();
    bool canSwipe(const Zone &z) const;
    void addSwipePoint(qreal x, qreal y);
    int capsNow() const;
    qreal swipeStart() const { return width() / 10 * 0.6; }
    bool askTouch(int phase, qreal x, qreal y);
    bool menuTouch(int phase, qreal x, qreal y);
    static qint64 now();

    QPointer<Controller> m_controller;
    qreal m_dp = 2;
    bool m_landscape = false;
    Settings m_settings;
    Painter m_painter;
    EmojiPanel m_emoji;
    QFont m_bold;

    Shift m_shift = Shift::Off;
    bool m_autoShift = false;
    qint64 m_lastShiftTap;
    Alt m_alt = Alt::Off;
    qint64 m_lastAltTap;
    Page m_page = Page::Letters;

    QVector<Zone> m_zones;
    int m_pressed = -1;
    int m_preview = -1;

    // Press and hold: accents.
    QStringList m_extended;
    QRectF m_extendedFrame;
    int m_extendedIndex = 0;

    // Press and hold: space moves the cursor.
    bool m_cursorMode = false;
    qreal m_cursorX = 0, m_cursorY = 0;

    // The suggestion bar.
    QStringList m_words = { QString(), QString(), QString() };
    int m_barPressed = -1;
    bool m_pasteOffered = false;
    bool m_wordsAllowed = true;
    qint64 m_flashAt;

    // Holding a word in the bar: add it to the dictionary.
    QString m_asking;
    QRectF m_askBox, m_askYes, m_askNo;
    bool m_askDown = false;
    int m_askPressed = 0;

    // The globe's menu.
    QStringList m_menu;
    QRectF m_menuFrame;
    int m_menuPressed = -1;
    bool m_menuDown = false;

    // Swipe to type.
    QVector<float> m_swipeX, m_swipeY;
    bool m_swipeMaybe = false;
    bool m_swiping = false;
    qint64 m_lastLive = 0;

    // Touch.
    int m_finger = -1;
    bool m_spent = false;
    bool m_held = false;
    qreal m_downX = 0, m_downY = 0;
    int m_repeats = 0;
    bool m_deleting = false;
    QTimer m_holdTimer;
    QTimer m_repeatTimer;
    QTimer m_barHoldTimer;
    QTimer m_frameTimer;
    QRectF m_popupRect;
};

} // namespace Pre

#endif // PRE_KEYBOARDVIEW_H
