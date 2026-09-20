/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef MALIIT_KEYBOARD_HARDWAREKEYBOARD_H
#define MALIIT_KEYBOARD_HARDWAREKEYBOARD_H

#include <QElapsedTimer>
#include <QEvent>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

namespace MaliitKeyboard {

//! \brief The alternate character levels a phone QWERTY keyboard offers.
//!
//! Names follow what the keys are labelled with on the devices themselves:
//! \c Alt is the level printed on the key faces, \c Sym the second one that
//! BlackBerry-style keyboards reach through a separate Sym key.
enum class HardwareKeyboardLevel {
    Base,
    Shift,
    Alt,
    Sym
};

//! \brief One device's physical keyboard description, keyed on evdev scancode.
//!
//! Loaded from JSON; see data/hwkeyboard/README.md for the format and for how
//! to produce one from a vendor's Android or xkb files.
struct HardwareKeyboardProfile
{
    QString name;
    QString description;
    QStringList inputDeviceNames;
    //! evdev scancodes the input device must advertise for this profile to
    //! apply. Two keyboards can share a device name -- the Unihertz Titan and
    //! Titan Pocket are both "aw9523-key" -- and the key set is what their
    //! drivers actually differ in.
    QSet<quint32> requiredKeys;
    //! evdev scancodes that select the Alt level while held, latched or locked.
    QSet<quint32> altKeys;
    //! evdev scancodes that select the Sym level the same way.
    QSet<quint32> symKeys;
    bool lockOnDoubleTap = true;
    //! level -> (evdev scancode -> the text that key produces at that level).
    QHash<int, QHash<quint32, QString> > levels;

    bool isValid() const { return not name.isEmpty(); }
    QString lookup(HardwareKeyboardLevel level, quint32 scanCode) const;
};

//! \brief Resolves the special keys on a device's physical keyboard.
//!
//! Phones with a hardware QWERTY print a second and sometimes a third
//! character on each key face, reached with the Alt and Sym keys. The kernel
//! driver reports only the plain scancode for those keys, and the compositor's
//! keymap knows nothing about the legends, so the alternate characters have to
//! be resolved here -- where the scancode is still available and the text has
//! not been committed yet.
//!
//! The mapping is data, not code: one JSON profile per keyboard, selected by
//! matching the input device name the kernel driver registered, and where that
//! is not enough the set of keys the device advertises. On a device with no
//! matching profile this class does nothing at all and keys travel the path
//! they always did.
//!
//! Some keyboards need no profile because their driver already resolves the
//! levels in the kernel and reports the resulting keycode -- the Zinwa Q25's
//! bbqX0kbd is one. Those are supposed to find no match here.
class HardwareKeyboard
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HardwareKeyboard)

    Q_PROPERTY(bool present READ isPresent NOTIFY profileChanged)
    Q_PROPERTY(QString profileName READ profileName NOTIFY profileChanged)
    Q_PROPERTY(bool altActive READ isAltActive NOTIFY levelChanged)
    Q_PROPERTY(bool altLocked READ isAltLocked NOTIFY levelChanged)
    Q_PROPERTY(bool symActive READ isSymActive NOTIFY levelChanged)
    Q_PROPERTY(bool symLocked READ isSymLocked NOTIFY levelChanged)

public:
    //! \brief What the caller should do with a key handleKey() was given.
    enum Result {
        //! Not ours: handle it exactly as it would have been handled without
        //! a profile.
        NotHandled,
        //! A level key, or a key this profile drops. Nothing reaches the
        //! application and nothing is inserted.
        Consumed,
        //! The key produces text other than what it reported. Insert the
        //! string handleKey() wrote out instead.
        Text
    };

    explicit HardwareKeyboard(QObject *parent = nullptr);
    ~HardwareKeyboard() override;

    //! \brief Whether a profile matched this device's keyboard.
    bool isPresent() const;
    QString profileName() const;

    bool isAltActive() const;
    bool isAltLocked() const;
    bool isSymActive() const;
    bool isSymLocked() const;

    //! \brief Resolves one key from the physical keyboard.
    //!
    //! \param type press or release; the level state only moves on a press, and
    //!        the text a press resolved to is replayed on the matching release
    //!        so a level that changes mid-keystroke cannot split it in two.
    //! \param nativeScanCode as the framework reports it, i.e. the evdev
    //!        scancode plus MInputContextWestonIMProtocolConnection's offset
    //!        of 8.
    //! \param modifiers the modifier state the compositor reported.
    //! \param text out: the string to insert, when the result is \c Text.
    Result handleKey(QEvent::Type type,
                     quint32 nativeScanCode,
                     Qt::KeyboardModifiers modifiers,
                     QString *text);

    //! \brief Whether an Alt modifier reported right now belongs to a level key.
    //!
    //! The compositor sets Qt::AltModifier for the Alt and Sym keys of these
    //! keyboards, because in a stock keymap those scancodes are Alt and AltGr.
    //! While a profile owns them that bit says "alternate character", not
    //! "keyboard shortcut", and the caller has to discount it.
    bool ownsAltModifier() const;

    //! \brief Forgets any latched level. Called when focus leaves a field.
    void reset();

Q_SIGNALS:
    void profileChanged();
    void levelChanged();

private:
    //! How far a level key has got: nothing, physically down, latched for one
    //! key by a tap, or locked by a second tap.
    enum class LevelState {
        Off,
        Held,
        Latched,
        Locked
    };

    struct LevelKeyState {
        LevelState state = LevelState::Off;
        //! Set when a key was resolved at this level while it was held, so the
        //! release is understood as "done holding" rather than as a tap.
        bool usedWhileHeld = false;
        bool pendingLock = false;
        bool pendingUnlock = false;

        bool isActive() const { return state != LevelState::Off; }
    };

    //! One entry per input device, from /proc/bus/input/devices.
    struct InputDevice {
        QString name;
        //! The device's EV_KEY bitmask, one word per entry, least significant
        //! first, and the width in bits of each word.
        QList<quint64> keyBits;
        int wordBits = 0;

        bool advertises(quint32 scanCode) const;
    };

    void loadProfiles();
    void selectProfile();
    static QList<InputDevice> readInputDevices();

    void handleLevelKeyPress(LevelKeyState *level);
    void handleLevelKeyRelease(LevelKeyState *level);
    //! Drops a latch, or notes the use of a held level, after a key was
    //! resolved at that level.
    void consumeLevel(LevelKeyState *level);

    HardwareKeyboardLevel activeLevel(Qt::KeyboardModifiers modifiers) const;

    QList<HardwareKeyboardProfile> m_profiles;
    //! Index into m_profiles, or -1 when this device has no physical keyboard
    //! we know about.
    int m_activeProfile;

    LevelKeyState m_alt;
    LevelKeyState m_sym;

    //! Scancodes currently down that we resolved to text, so the release can
    //! be answered with the same string the press produced.
    QHash<quint32, QString> m_pressedKeys;

    //! The input device may well enumerate after maliit-server starts, so a
    //! first look that finds nothing is retried -- throttled, because it reads
    //! a proc file on a key press.
    QElapsedTimer m_lastScan;
    bool m_scanned;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_HARDWAREKEYBOARD_H
