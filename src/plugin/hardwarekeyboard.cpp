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

#include "hardwarekeyboard.h"
#include "keyboardlogging.h"

#include <linux/input-event-codes.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>

namespace MaliitKeyboard {

namespace {

//! The framework hands plugins the evdev scancode raised by this much; see
//! MInputContextWestonIMProtocolConnectionPrivate::processKeyEvent().
const quint32 g_evdev_offset = 8;

//! A scan that found no keyboard is retried no more often than this. The
//! input device can appear long after maliit-server does.
const qint64 g_rescan_interval_ms = 2000;

//! \brief Where profiles are looked for, most specific first.
//!
//! The environment variable exists so a profile can be tried on a running
//! device without reinstalling the package.
QStringList profileDirectories()
{
    QStringList dirs;

    const QByteArray override(qgetenv("LUNEOS_KEYBOARD_HW_LAYOUT_DIR"));
    if (not override.isEmpty()) {
        dirs.append(QString::fromLocal8Bit(override));
    }

    dirs.append(QString::fromLatin1(LUNEOS_KEYBOARD_DATA_DIR "/hwkeyboard"));

    return dirs;
}

int levelKey(HardwareKeyboardLevel level)
{
    return static_cast<int>(level);
}

HardwareKeyboardLevel levelFromName(const QString &name, bool *ok)
{
    *ok = true;

    if (name == QLatin1String("base"))
        return HardwareKeyboardLevel::Base;
    if (name == QLatin1String("shift"))
        return HardwareKeyboardLevel::Shift;
    if (name == QLatin1String("alt"))
        return HardwareKeyboardLevel::Alt;
    if (name == QLatin1String("sym"))
        return HardwareKeyboardLevel::Sym;

    *ok = false;
    return HardwareKeyboardLevel::Base;
}

QSet<quint32> readScanCodes(const QJsonArray &array)
{
    QSet<quint32> codes;

    for (const QJsonValue &value : array) {
        const int code = value.toInt(-1);
        if (code >= 0)
            codes.insert(static_cast<quint32>(code));
    }

    return codes;
}

HardwareKeyboardProfile readProfile(const QJsonObject &object,
                                    const QString &path)
{
    HardwareKeyboardProfile profile;

    profile.name = object.value("name").toString();
    if (profile.name.isEmpty()) {
        qWarning() << "hardware keyboard profile without a name, ignoring:" << path;
        return HardwareKeyboardProfile();
    }

    profile.description = object.value("description").toString();
    profile.lockOnDoubleTap = object.value("lockOnDoubleTap").toBool(true);
    profile.altKeys = readScanCodes(object.value("altKeys").toArray());
    profile.symKeys = readScanCodes(object.value("symKeys").toArray());
    profile.shiftKeys = readScanCodes(object.value("shiftKeys").toArray());

    const QJsonObject match(object.value("match").toObject());
    profile.requiredKeys = readScanCodes(match.value("requireKeys").toArray());

    const QJsonArray names(match.value("inputDeviceNames").toArray());
    for (const QJsonValue &value : names) {
        const QString name(value.toString());
        if (not name.isEmpty())
            profile.inputDeviceNames.append(name);
    }

    const QJsonObject levels(object.value("levels").toObject());
    for (auto it = levels.constBegin(); it != levels.constEnd(); ++it) {
        bool known = false;
        const HardwareKeyboardLevel level(levelFromName(it.key(), &known));
        if (not known) {
            qWarning() << "hardware keyboard profile" << profile.name
                       << "has an unknown level" << it.key() << "-- ignoring it";
            continue;
        }

        QHash<quint32, QString> characters;
        const QJsonObject entries(it.value().toObject());
        for (auto entry = entries.constBegin(); entry != entries.constEnd(); ++entry) {
            bool valid = false;
            const uint code = entry.key().toUInt(&valid);
            const QString text(entry.value().toString());
            if (valid and not text.isEmpty())
                characters.insert(code, text);
        }

        if (not characters.isEmpty())
            profile.levels.insert(levelKey(level), characters);
    }

    return profile;
}

} // unnamed namespace

QString HardwareKeyboardProfile::lookup(HardwareKeyboardLevel level,
                                        quint32 scanCode) const
{
    const auto it = levels.constFind(levelKey(level));
    if (it == levels.constEnd())
        return QString();

    return it.value().value(scanCode);
}

HardwareKeyboard::HardwareKeyboard(QObject *parent)
    : QObject(parent)
    , m_profiles()
    , m_activeProfile(-1)
    , m_alt()
    , m_sym()
    , m_pressedKeys()
    , m_lastScan()
    , m_scanned(false)
{
    loadProfiles();
    selectProfile();
}

HardwareKeyboard::~HardwareKeyboard() = default;

void HardwareKeyboard::loadProfiles()
{
    const QStringList directories(profileDirectories());

    for (const QString &directory : directories) {
        QDir dir(directory);
        if (not dir.exists())
            continue;

        const QStringList files(dir.entryList(QStringList("*.json"),
                                              QDir::Files, QDir::Name));
        for (const QString &file : files) {
            const QString path(dir.filePath(file));

            QFile json(path);
            if (not json.open(QIODevice::ReadOnly)) {
                qWarning() << "cannot read the hardware keyboard profile" << path
                           << ":" << json.errorString();
                continue;
            }

            QJsonParseError error;
            const QJsonDocument document(QJsonDocument::fromJson(json.readAll(), &error));
            if (document.isNull()) {
                qWarning() << "cannot parse the hardware keyboard profile" << path
                           << ":" << error.errorString();
                continue;
            }

            const HardwareKeyboardProfile profile(readProfile(document.object(), path));
            if (profile.isValid())
                m_profiles.append(profile);
        }
    }
}

bool HardwareKeyboard::InputDevice::advertises(quint32 scanCode) const
{
    if (wordBits <= 0)
        return false;

    const int word = static_cast<int>(scanCode) / wordBits;
    const int bit = static_cast<int>(scanCode) % wordBits;

    if (word >= keyBits.size())
        return false;

    return (keyBits.at(word) >> bit) & 1;
}

//! \brief Every input device the kernel knows about, with its EV_KEY bitmask.
//!
//! /proc/bus/input/devices rather than the evdev nodes: it is world readable,
//! and maliit-server has no business needing a seat on /dev/input.
//!
//! The kernel prints a bitmap most significant word first, padding every word
//! but the first to the machine's word width -- which is therefore what the
//! second word's length tells us, without having to assume 32 or 64 bit.
QList<HardwareKeyboard::InputDevice> HardwareKeyboard::readInputDevices()
{
    QList<InputDevice> devices;

    // The override lets a /proc/bus/input/devices captured off a phone be
    // replayed anywhere, which is how a new profile's matching is checked
    // without the hardware in hand.
    QByteArray path(qgetenv("LUNEOS_KEYBOARD_HW_INPUT_DEVICES"));
    if (path.isEmpty())
        path = "/proc/bus/input/devices";

    QFile proc(QString::fromLocal8Bit(path));
    if (not proc.open(QIODevice::ReadOnly | QIODevice::Text))
        return devices;

    static const QRegularExpression name_line("^N: Name=\"(.*)\"$");
    static const QRegularExpression key_line("^B: KEY=(.*)$");

    // Read the whole file up front rather than streaming it. procfs reports a
    // size of 0, and QFileDevice::atEnd() is size() == pos(), so it answers
    // true before a single line has been read -- a QTextStream loop guarded on
    // atEnd() silently sees no devices at all, and every profile then fails to
    // match on real hardware while matching fine against a captured copy.
    const QList<QByteArray> lines(proc.readAll().split('\n'));

    for (const QByteArray &raw : lines) {
        const QString line(QString::fromLocal8Bit(raw).trimmed());

        const QRegularExpressionMatch name(name_line.match(line));
        if (name.hasMatch()) {
            InputDevice device;
            device.name = name.captured(1);
            devices.append(device);
            continue;
        }

        const QRegularExpressionMatch keys(key_line.match(line));
        if (not keys.hasMatch() or devices.isEmpty())
            continue;

        const QStringList words(keys.captured(1).split(QLatin1Char(' '),
                                                       Qt::SkipEmptyParts));
        if (words.isEmpty())
            continue;

        InputDevice &device = devices.last();
        device.wordBits = 4 * (words.size() > 1 ? words.at(1).size()
                                                : words.at(0).size());

        // Least significant word first, so a scancode indexes straight into
        // the list.
        for (int i = words.size() - 1; i >= 0; --i)
            device.keyBits.append(words.at(i).toULongLong(nullptr, 16));
    }

    return devices;
}

void HardwareKeyboard::selectProfile()
{
    m_scanned = true;
    m_lastScan.start();

    const int previous = m_activeProfile;
    m_activeProfile = -1;

    const QByteArray requested(qgetenv("LUNEOS_KEYBOARD_HW_LAYOUT"));
    if (requested == "none") {
        // An explicit opt-out, for a device whose driver resolves the levels
        // itself or whose profile is being debugged.
        if (previous != m_activeProfile)
            Q_EMIT profileChanged();
        return;
    }

    if (not requested.isEmpty()) {
        const QString wanted(QString::fromLocal8Bit(requested));
        for (int i = 0; i < m_profiles.size(); ++i) {
            if (m_profiles.at(i).name == wanted) {
                m_activeProfile = i;
                break;
            }
        }
        if (m_activeProfile < 0) {
            qWarning() << "LUNEOS_KEYBOARD_HW_LAYOUT asks for the hardware keyboard"
                       << "profile" << wanted << "but no such profile is installed";
        }
    } else {
        const QList<InputDevice> present(readInputDevices());

        // Of the profiles that match, the most specific wins: one that names
        // the keys it needs knows something about this keyboard that a
        // name-only profile does not.
        int best = -1;

        for (int i = 0; i < m_profiles.size(); ++i) {
            const HardwareKeyboardProfile &profile = m_profiles.at(i);

            if (best >= 0
                and profile.requiredKeys.size()
                    <= m_profiles.at(best).requiredKeys.size()) {
                continue;
            }

            for (const InputDevice &device : present) {
                if (not profile.inputDeviceNames.contains(device.name))
                    continue;

                bool has_all = true;
                for (quint32 key : profile.requiredKeys) {
                    if (not device.advertises(key)) {
                        has_all = false;
                        break;
                    }
                }

                if (has_all) {
                    best = i;
                    break;
                }
            }
        }

        // A telephone keypad reports the digits and none of the letters; a
        // keyboard reports both. Worked out here because this is where the
        // capability bitmaps are already in hand, and refreshed on every
        // rescan so a keypad that appears late is still found.
        m_telephoneKeypad = false;
        for (const InputDevice &device : present) {
            const bool digits = device.advertises(KEY_2) and device.advertises(KEY_5)
                and device.advertises(KEY_9);
            const bool letters = device.advertises(KEY_A) or device.advertises(KEY_Q);

            if (digits and not letters) {
                m_telephoneKeypad = true;
                qCInfo(lcHwKeyboard) << "telephone keypad:" << device.name;
            }
        }

        for (const InputDevice &device : present)
            qCInfo(lcHwKeyboard) << "input device present:" << device.name;
        qCInfo(lcHwKeyboard) << present.size() << "input devices,"
                              << m_profiles.size() << "profiles loaded";

        m_activeProfile = best;
    }

    if (m_activeProfile < 0)
        qCInfo(lcHwKeyboard) << "no hardware keyboard profile matched";

    if (m_activeProfile >= 0) {
        qInfo() << "using the hardware keyboard profile"
                << m_profiles.at(m_activeProfile).name
                << "--" << m_profiles.at(m_activeProfile).description;
    }

    if (previous != m_activeProfile)
        Q_EMIT profileChanged();
}

bool HardwareKeyboard::isPresent() const
{
    return m_activeProfile >= 0;
}

QString HardwareKeyboard::profileName() const
{
    return isPresent() ? m_profiles.at(m_activeProfile).name : QString();
}

bool HardwareKeyboard::isAltActive() const
{
    return m_alt.isActive();
}

bool HardwareKeyboard::isAltLocked() const
{
    return m_alt.state == LevelState::Locked;
}

bool HardwareKeyboard::isSymActive() const
{
    return m_sym.isActive();
}

bool HardwareKeyboard::isSymLocked() const
{
    return m_sym.state == LevelState::Locked;
}

bool HardwareKeyboard::hasTelephoneKeypad() const
{
    return m_telephoneKeypad;
}

bool HardwareKeyboard::shiftLatchActive() const
{
    if (not isPresent())
        return false;

    if (m_profiles.at(m_activeProfile).shiftKeys.isEmpty())
        return false;

    // Held is Qt's business, not ours.
    return m_shift.state == LevelState::Latched
        or m_shift.state == LevelState::Locked;
}

void HardwareKeyboard::consumeShiftLatch()
{
    consumeLevel(&m_shift);
    Q_EMIT levelChanged();
}

bool HardwareKeyboard::ownsAltModifier() const
{
    if (not isPresent())
        return false;

    const HardwareKeyboardProfile &profile = m_profiles.at(m_activeProfile);

    // Both the Alt and the Sym key of these keyboards sit on a scancode a
    // stock keymap reads as an Alt of some kind, so either being engaged
    // explains an Alt modifier we should not treat as a shortcut.
    return (not profile.altKeys.isEmpty() and m_alt.isActive())
        or (not profile.symKeys.isEmpty() and m_sym.isActive());
}

void HardwareKeyboard::reset()
{
    const bool was_active = m_alt.isActive() or m_sym.isActive()
        or m_shift.isActive();

    m_alt = LevelKeyState();
    m_sym = LevelKeyState();
    m_shift = LevelKeyState();
    m_pressedKeys.clear();

    if (was_active)
        Q_EMIT levelChanged();
}


const char *HardwareKeyboard::levelStateName(LevelState state)
{
    switch (state) {
    case LevelState::Off:     return "off";
    case LevelState::Held:    return "held";
    case LevelState::Latched: return "latched";
    case LevelState::Locked:  return "locked";
    }
    return "?";
}

void HardwareKeyboard::handleLevelKeyPress(LevelKeyState *level)
{
    switch (level->state) {
    case LevelState::Held:
        // Autorepeat while the key is down; nothing to do.
        return;

    case LevelState::Latched:
        level->pendingLock = m_profiles.at(m_activeProfile).lockOnDoubleTap;
        level->pendingUnlock = false;
        break;

    case LevelState::Locked:
        level->pendingLock = false;
        level->pendingUnlock = true;
        break;

    case LevelState::Off:
        level->pendingLock = false;
        level->pendingUnlock = false;
        break;
    }

    const LevelState was = level->state;
    level->state = LevelState::Held;
    level->usedWhileHeld = false;

    qCInfo(lcHwKeyboard, "level key down: %s -> held (pendingLock=%d pendingUnlock=%d)",
           levelStateName(was), int(level->pendingLock), int(level->pendingUnlock));
}

void HardwareKeyboard::handleLevelKeyRelease(LevelKeyState *level)
{
    if (level->state != LevelState::Held)
        return;

    if (level->usedWhileHeld or level->pendingUnlock) {
        // Held down over one or more keys, or a third tap: back to normal.
        level->state = LevelState::Off;
    } else if (level->pendingLock) {
        level->state = LevelState::Locked;
    } else {
        // A tap on its own: the level applies to the next key only.
        level->state = LevelState::Latched;
    }

    qCInfo(lcHwKeyboard, "level key up: -> %s (usedWhileHeld=%d pendingLock=%d pendingUnlock=%d)",
           levelStateName(level->state), int(level->usedWhileHeld),
           int(level->pendingLock), int(level->pendingUnlock));

    level->pendingLock = false;
    level->pendingUnlock = false;
    level->usedWhileHeld = false;
}

void HardwareKeyboard::consumeLevel(LevelKeyState *level)
{
    switch (level->state) {
    case LevelState::Held:
        level->usedWhileHeld = true;
        break;

    case LevelState::Latched:
        level->state = LevelState::Off;
        break;

    case LevelState::Locked:
    case LevelState::Off:
        break;
    }
}

HardwareKeyboardLevel HardwareKeyboard::activeLevel(Qt::KeyboardModifiers modifiers) const
{
    if (m_sym.isActive())
        return HardwareKeyboardLevel::Sym;

    if (m_alt.isActive())
        return HardwareKeyboardLevel::Alt;

    if ((modifiers & Qt::ShiftModifier) or m_shift.isActive())
        return HardwareKeyboardLevel::Shift;

    return HardwareKeyboardLevel::Base;
}

HardwareKeyboard::Result HardwareKeyboard::handleKey(QEvent::Type type,
                                                     quint32 nativeScanCode,
                                                     Qt::KeyboardModifiers modifiers,
                                                     QString *text)
{
    if (not isPresent()) {
        // The keyboard may only have shown up after maliit-server started.
        if (not m_scanned
            or (m_lastScan.isValid() and m_lastScan.elapsed() > g_rescan_interval_ms)) {
            selectProfile();
        }

        if (not isPresent())
            return NotHandled;
    }

    if (nativeScanCode < g_evdev_offset)
        return NotHandled;

    const HardwareKeyboardProfile &profile = m_profiles.at(m_activeProfile);
    const quint32 code = nativeScanCode - g_evdev_offset;

    if (profile.shiftKeys.contains(code)) {
        // Deliberately NotHandled rather than Consumed: the application still
        // needs the key, because holding Shift is Qt's ShiftModifier doing the
        // work and swallowing it here would cost us capitals altogether. All
        // this adds is the latch a tap leaves behind.
        if (type == QEvent::KeyPress)
            handleLevelKeyPress(&m_shift);
        else if (type == QEvent::KeyRelease)
            handleLevelKeyRelease(&m_shift);

        Q_EMIT levelChanged();
        return NotHandled;
    }

    if (profile.altKeys.contains(code) or profile.symKeys.contains(code)) {
        LevelKeyState *const level = profile.altKeys.contains(code) ? &m_alt : &m_sym;

        if (type == QEvent::KeyPress)
            handleLevelKeyPress(level);
        else if (type == QEvent::KeyRelease)
            handleLevelKeyRelease(level);

        Q_EMIT levelChanged();
        return Consumed;
    }

    if (type == QEvent::KeyRelease) {
        // Answer the release with whatever the press decided, so a level that
        // was latched and is now spent cannot turn one keystroke into two
        // different characters.
        const auto pressed = m_pressedKeys.constFind(code);
        if (pressed == m_pressedKeys.constEnd())
            return NotHandled;

        *text = pressed.value();
        m_pressedKeys.erase(pressed);
        return Text;
    }

    if (type != QEvent::KeyPress)
        return NotHandled;

    const HardwareKeyboardLevel level(activeLevel(modifiers));
    const QString mapped(profile.lookup(level, code));

    qCInfo(lcHwKeyboard, "code %u at level %d -> '%s' (alt=%s sym=%s)",
           code, int(level), qPrintable(mapped),
           levelStateName(m_alt.state), levelStateName(m_sym.state));

    // An alternate level that has nothing on this key -- Backspace or Return,
    // typically -- still spends the latch, but the key itself carries on to
    // the application unchanged rather than disappearing.
    if (mapped.isEmpty()) {
        if (level == HardwareKeyboardLevel::Alt or level == HardwareKeyboardLevel::Sym) {
            consumeLevel(&m_sym);
            consumeLevel(&m_alt);
            Q_EMIT levelChanged();
        }
        return NotHandled;
    }

    if (level == HardwareKeyboardLevel::Alt or level == HardwareKeyboardLevel::Sym) {
        consumeLevel(&m_sym);
        consumeLevel(&m_alt);
        Q_EMIT levelChanged();
    }

    m_pressedKeys.insert(code, mapped);
    *text = mapped;
    return Text;
}

} // namespace MaliitKeyboard
