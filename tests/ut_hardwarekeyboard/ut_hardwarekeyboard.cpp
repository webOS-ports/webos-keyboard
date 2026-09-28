/*
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 3.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

// HardwareKeyboard is the one part of the plugin whose behaviour cannot be
// checked by looking at the screen: the compositor hands the keys to whoever
// holds the input method grab, so evtest sees nothing, and what a key produced
// is only visible as text arriving in a field. It is also per-device, and the
// devices are not on this desk.
//
// Everything it needs comes from two files and three environment variables, so
// all of it can be driven from here: LUNEOS_KEYBOARD_HW_LAYOUT_DIR for the
// profiles, LUNEOS_KEYBOARD_HW_INPUT_DEVICES for a captured
// /proc/bus/input/devices, LUNEOS_KEYBOARD_HW_LAYOUT to force or refuse a
// profile.

#include "hardwarekeyboard.h"

#include <linux/input-event-codes.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QtTest>

using namespace MaliitKeyboard;

namespace {

//! The framework raises every evdev scancode by this much before the plugin
//! sees it, so a test that passes a bare scancode tests nothing.
const quint32 EvdevOffset = 8;

quint32 sc(quint32 evdevCode)
{
    return evdevCode + EvdevOffset;
}

//! A QWERTY keyboard's capability line, as /proc/bus/input/devices prints it.
const char *const QwertyKeys =
    "B: KEY=402000000 3803078f800d001 feffffdfffefffff fffffffffffffffe";

//! A telephone keypad: the ten digits, no letters. Copied from an MP01.
const char *const KeypadKeys =
    "B: KEY=800 0 0 0 0 0 0 0 0 8 0 0 0 1c0000 0 0 ffc";

QSet<int> readCodes(const QJsonArray &array)
{
    QSet<int> codes;

    for (const QJsonValueConstRef &value : array)
        codes.insert(value.toInt(-1));

    codes.remove(-1);

    return codes;
}

QByteArray deviceBlock(const QString &name, const char *keys)
{
    return QStringLiteral("N: Name=\"%1\"\nH: Handlers=kbd event1 \n%2\n\n")
        .arg(name, QString::fromLatin1(keys))
        .toUtf8();
}

} // namespace

class Ut_HardwareKeyboard : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    // Profile loading: everything here is a file on disk that something else
    // wrote, so none of it can be assumed well formed.
    void testNoProfilesInstalled();
    void testProfileLoads();
    void testMalformedJsonIsIgnored();
    void testProfileWithoutANameIsIgnored();
    void testJsonArrayInsteadOfObjectIsIgnored();
    void testEmptyFileIsIgnored();
    void testUnknownLevelNameIsIgnored();
    void testNonNumericScanCodeIsIgnored();
    void testEmptyMappingIsIgnored();
    void testOneBadProfileDoesNotStopTheGoodOne();
    void testNonJsonFilesAreNotRead();

    // Profile selection.
    void testSelectionByDeviceName();
    void testNoMatchingDeviceName();
    void testMostSpecificProfileWins();
    void testRequiredKeysMustBeAdvertised();
    void testForcedProfile();
    void testForcedProfileThatIsNotInstalled();
    void testExplicitOptOut();

    void testTelephoneKeypadDetected();
    void testQwertyIsNotATelephoneKeypad();

    // Key resolution and the level state machine.
    void testUnmappedKeyIsNotHandled();
    void testScanCodeBelowTheOffsetIsNotHandled();
    void testShiftLevelFromTheModifier();

    void testAltHeld();
    void testAltHeldOverTwoKeysDoesNotLatch();
    void testAltTapLatchesForOneKey();
    void testAltDoubleTapLocks();
    void testThirdTapUnlocks();
    void testAltWithNothingOnTheKeySpendsTheLatch();
    void testReleaseReplaysWhatThePressDecided();

    void testSymTakesPrecedenceOverAlt();
    void testOwnsAltModifier();

    void testShiftKeyIsPassedOnButLatches();
    void testShiftLatchIsSpentWhenConsumed();
    void testShiftLatchNeedsAProfileThatAsksForIt();

    void testResetClearsEveryLatch();
    void testLevelChangedIsEmitted();

    // A numeric field taking the digits off the key faces, and a legend-only
    // profile, which is the shape that makes it work on a keyboard whose
    // driver resolves its own levels.
    void testNumericFieldIsOffByDefault();
    void testNumericFieldTakesTheDigitOffTheKeyFace();
    void testNumericFieldLeavesAKeyWithNoDigitAlone();
    void testNumericFieldLeavesAHeldLevelAlone();
    void testNumericFieldReleaseReplaysTheDigit();
    void testNumericFieldNeedsAProfile();
    void testLegendOnlyProfileClaimsNoLevelKeys();

    // The profiles this package ships.
    void testShippedProfilesAreWellFormed_data();
    void testShippedProfilesAreWellFormed();
    void testShippedProfilesLoad();
    void testEmojiKeyIsTakenWholeAndInsertsNothing();

private:
    void writeProfile(const QString &fileName, const QByteArray &contents);
    void writeDevices(const QByteArray &contents);

    //! A profile for a keyboard with Alt on KEY_LEFTALT and Sym on KEY_RIGHTALT,
    //! mapping KEY_Q at each level.
    static QByteArray titanProfile(bool lockOnDoubleTap = true,
                                   bool withShiftKeys = false);

    //! A profile in the shape a keyboard needs when its driver already
    //! resolves Alt and Sym: no level keys at all, and an `alt` level that is
    //! there to say what the key faces are labelled with. KEY_W carries a 1
    //! and KEY_T a character no numeric field wants, as they do on a Q25.
    //! (An "@" rather than the Q25's own "(": moc counts parentheses through
    //! raw strings and an unbalanced one here stops the build.)
    static QByteArray legendOnlyProfile();

    QTemporaryDir *m_profileDir = nullptr;
    QTemporaryFile *m_devices = nullptr;
};

void Ut_HardwareKeyboard::init()
{
    m_profileDir = new QTemporaryDir;
    QVERIFY(m_profileDir->isValid());
    qputenv("LUNEOS_KEYBOARD_HW_LAYOUT_DIR", m_profileDir->path().toLocal8Bit());

    m_devices = new QTemporaryFile;
    QVERIFY(m_devices->open());
    qputenv("LUNEOS_KEYBOARD_HW_INPUT_DEVICES", m_devices->fileName().toLocal8Bit());

    qunsetenv("LUNEOS_KEYBOARD_HW_LAYOUT");

    // Every test that expects a match says so by writing a device; the default
    // is a device list with nothing on it.
    writeDevices(QByteArray());
}

void Ut_HardwareKeyboard::cleanup()
{
    qunsetenv("LUNEOS_KEYBOARD_HW_LAYOUT_DIR");
    qunsetenv("LUNEOS_KEYBOARD_HW_INPUT_DEVICES");
    qunsetenv("LUNEOS_KEYBOARD_HW_LAYOUT");

    delete m_profileDir;
    m_profileDir = nullptr;
    delete m_devices;
    m_devices = nullptr;
}

void Ut_HardwareKeyboard::writeProfile(const QString &fileName,
                                      const QByteArray &contents)
{
    QFile file(m_profileDir->filePath(fileName));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(contents), qint64(contents.size()));
}

void Ut_HardwareKeyboard::writeDevices(const QByteArray &contents)
{
    QVERIFY(m_devices->resize(0));
    QCOMPARE(m_devices->write(contents), qint64(contents.size()));
    QVERIFY(m_devices->flush());
}

QByteArray Ut_HardwareKeyboard::titanProfile(bool lockOnDoubleTap,
                                             bool withShiftKeys)
{
    return QStringLiteral(R"({
    "name": "titan",
    "description": "test keyboard",
    "lockOnDoubleTap": %1,
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "altKeys": [%2],
    "symKeys": [%3],
    "shiftKeys": [%4],
    "levels": {
        "base":  { "%5": "q" },
        "shift": { "%5": "Q" },
        "alt":   { "%5": "#" },
        "sym":   { "%5": "§" }
    }
})")
        .arg(lockOnDoubleTap ? QStringLiteral("true") : QStringLiteral("false"))
        .arg(KEY_LEFTALT)
        .arg(KEY_RIGHTALT)
        .arg(withShiftKeys ? QString::number(KEY_LEFTSHIFT) : QString())
        .arg(KEY_Q)
        .toUtf8();
}

QByteArray Ut_HardwareKeyboard::legendOnlyProfile()
{
    return QStringLiteral(R"({
    "name": "legendonly",
    "description": "a keyboard whose driver owns the levels",
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "altKeys": [],
    "symKeys": [],
    "levels": {
        "alt": { "%1": "1", "%2": "@" }
    }
})")
        .arg(KEY_W)
        .arg(KEY_T)
        .toUtf8();
}

void Ut_HardwareKeyboard::testNoProfilesInstalled()
{
    HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
    QVERIFY(keyboard.profileName().isEmpty());

    // And it must not claim any key.
    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testProfileLoads()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
    QCOMPARE(keyboard.profileName(), QStringLiteral("titan"));
}

void Ut_HardwareKeyboard::testMalformedJsonIsIgnored()
{
    // A truncated file: half-written by an editor, or cut short by a full disk.
    // It must not take maliit-server down with it.
    writeProfile(QStringLiteral("broken.json"),
                 QByteArray("{ \"name\": \"broken\", \"levels\": {"));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testProfileWithoutANameIsIgnored()
{
    writeProfile(QStringLiteral("nameless.json"),
                 QByteArray(R"({ "match": { "inputDeviceNames": ["aw9523-key"] } })"));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testJsonArrayInsteadOfObjectIsIgnored()
{
    writeProfile(QStringLiteral("array.json"), QByteArray("[ { \"name\": \"x\" } ]"));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testEmptyFileIsIgnored()
{
    writeProfile(QStringLiteral("empty.json"), QByteArray());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testUnknownLevelNameIsIgnored()
{
    // A level this build does not know about - a profile written for a later
    // version - loses that level and keeps the rest, rather than the whole
    // profile being thrown away.
    const QByteArray profile = QStringLiteral(R"({
    "name": "future",
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "levels": {
        "base": { "%1": "q" },
        "hyper": { "%1": "!" }
    }
})").arg(KEY_Q).toUtf8();

    writeProfile(QStringLiteral("future.json"), profile);
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
    QCOMPARE(keyboard.profileName(), QStringLiteral("future"));

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("q"));
}

void Ut_HardwareKeyboard::testNonNumericScanCodeIsIgnored()
{
    const QByteArray profile = QStringLiteral(R"({
    "name": "typo",
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "levels": { "base": { "KEY_Q": "q", "%1": "w" } }
})").arg(KEY_W).toUtf8();

    writeProfile(QStringLiteral("typo.json"), profile);
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QVERIFY(keyboard.isPresent());

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("w"));
}

void Ut_HardwareKeyboard::testEmptyMappingIsIgnored()
{
    // "" would resolve to inserting nothing, which reads as a swallowed key.
    const QByteArray profile = QStringLiteral(R"({
    "name": "blank",
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "levels": { "base": { "%1": "" } }
})").arg(KEY_Q).toUtf8();

    writeProfile(QStringLiteral("blank.json"), profile);
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;

    // No level survived, so nothing matched and the profile is not valid to use.
    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testOneBadProfileDoesNotStopTheGoodOne()
{
    // Files are read in name order, so the broken one is read first on purpose.
    writeProfile(QStringLiteral("a-broken.json"), QByteArray("{{{"));
    writeProfile(QStringLiteral("b-titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
    QCOMPARE(keyboard.profileName(), QStringLiteral("titan"));
}

void Ut_HardwareKeyboard::testNonJsonFilesAreNotRead()
{
    writeProfile(QStringLiteral("README.md"), QByteArray("not a profile at all"));
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
}

void Ut_HardwareKeyboard::testSelectionByDeviceName()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("gpio-keys"), QwertyKeys)
                 + deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
}

void Ut_HardwareKeyboard::testNoMatchingDeviceName()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("some-other-keyboard"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testMostSpecificProfileWins()
{
    // The real case this rule exists for: the Unihertz Titan and Titan Pocket
    // both register "aw9523-key", and the only thing that tells them apart is
    // the set of keys the driver advertises.
    const QByteArray nameOnly = QStringLiteral(R"({
    "name": "titan",
    "match": { "inputDeviceNames": ["aw9523-key"] },
    "levels": { "base": { "%1": "q" } }
})").arg(KEY_Q).toUtf8();

    const QByteArray withKeys = QStringLiteral(R"({
    "name": "titanpocket",
    "match": { "inputDeviceNames": ["aw9523-key"], "requireKeys": [%1, %2] },
    "levels": { "base": { "%3": "Q" } }
})").arg(KEY_A).arg(KEY_Z).arg(KEY_Q).toUtf8();

    writeProfile(QStringLiteral("a-titan.json"), nameOnly);
    writeProfile(QStringLiteral("b-pocket.json"), withKeys);
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QCOMPARE(keyboard.profileName(), QStringLiteral("titanpocket"));
}

void Ut_HardwareKeyboard::testRequiredKeysMustBeAdvertised()
{
    // KEY_F24 is not on any of these keyboards, so a profile demanding it must
    // not match however well the name fits.
    const QByteArray profile = QStringLiteral(R"({
    "name": "impossible",
    "match": { "inputDeviceNames": ["aw9523-key"], "requireKeys": [%1] },
    "levels": { "base": { "%2": "q" } }
})").arg(KEY_F24).arg(KEY_Q).toUtf8();

    writeProfile(QStringLiteral("impossible.json"), profile);
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testForcedProfile()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    // No device list at all: the override does not consult it.
    qputenv("LUNEOS_KEYBOARD_HW_LAYOUT", "titan");

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
    QCOMPARE(keyboard.profileName(), QStringLiteral("titan"));
}

void Ut_HardwareKeyboard::testForcedProfileThatIsNotInstalled()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    qputenv("LUNEOS_KEYBOARD_HW_LAYOUT", "nosuchprofile");

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testExplicitOptOut()
{
    // "none" is how a device whose driver resolves the levels itself - the Zinwa
    // Q25's bbqX0kbd - says so, and how a profile under suspicion is taken out
    // of the way.
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));
    qputenv("LUNEOS_KEYBOARD_HW_LAYOUT", "none");

    const HardwareKeyboard keyboard;

    QVERIFY(!keyboard.isPresent());
}

void Ut_HardwareKeyboard::testTelephoneKeypadDetected()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("mtk-kpd"), KeypadKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.hasTelephoneKeypad());
}

void Ut_HardwareKeyboard::testQwertyIsNotATelephoneKeypad()
{
    // Multi-tap on a keyboard with a number row would turn every 2 into an A.
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    const HardwareKeyboard keyboard;

    QVERIFY(keyboard.isPresent());
    QVERIFY(!keyboard.hasTelephoneKeypad());
}

void Ut_HardwareKeyboard::testUnmappedKeyIsNotHandled()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    // KEY_W has no entry at any level, so it travels the path it always did.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testScanCodeBelowTheOffsetIsNotHandled()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    // Subtracting the offset from anything smaller would wrap, and quint32
    // wrapping lands somewhere near four billion rather than failing.
    for (quint32 code = 0; code < EvdevOffset; ++code) {
        QCOMPARE(keyboard.handleKey(QEvent::KeyPress, code, Qt::NoModifier, &text),
                 HardwareKeyboard::NotHandled);
    }
}

void Ut_HardwareKeyboard::testShiftLevelFromTheModifier()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::ShiftModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("Q"));
}

void Ut_HardwareKeyboard::testAltHeld()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text),
             HardwareKeyboard::Consumed);
    QVERIFY(keyboard.isAltActive());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::AltModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("#"));

    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::AltModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("#"));
}

void Ut_HardwareKeyboard::testAltHeldOverTwoKeysDoesNotLatch()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::AltModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::AltModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);

    // The level was used while held, so letting go is "done holding" and not a
    // tap: nothing is left over for the next key.
    QVERIFY(!keyboard.isAltActive());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("q"));
}

void Ut_HardwareKeyboard::testAltTapLatchesForOneKey()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);

    QVERIFY(keyboard.isAltActive());
    QVERIFY(!keyboard.isAltLocked());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("#"));
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::NoModifier, &text);

    // Spent.
    QVERIFY(!keyboard.isAltActive());
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("q"));
}

void Ut_HardwareKeyboard::testAltDoubleTapLocks()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    for (int tap = 0; tap < 2; ++tap) {
        keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
        keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    }

    QVERIFY(keyboard.isAltLocked());

    // Two keys in a row, both at the alt level.
    for (int i = 0; i < 2; ++i) {
        QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
                 HardwareKeyboard::Text);
        QCOMPARE(text, QStringLiteral("#"));
        keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::NoModifier, &text);
    }

    QVERIFY(keyboard.isAltLocked());
}

void Ut_HardwareKeyboard::testThirdTapUnlocks()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    for (int tap = 0; tap < 3; ++tap) {
        keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
        keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    }

    QVERIFY(!keyboard.isAltActive());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("q"));
}

void Ut_HardwareKeyboard::testAltWithNothingOnTheKeySpendsTheLatch()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    QVERIFY(keyboard.isAltActive());

    // Backspace has no alt mapping. It has to reach the application unchanged,
    // and it still costs the latch - otherwise the latch would be waiting on
    // the next letter, which nobody asked for.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_BACKSPACE), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
    QVERIFY(!keyboard.isAltActive());
}

void Ut_HardwareKeyboard::testReleaseReplaysWhatThePressDecided()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    // Latch alt, press Q - which spends the latch - and only then release it.
    // The release must not be re-resolved at the base level, or one keystroke
    // would insert "#" and then delete and replace it with "q".
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("#"));
    QVERIFY(!keyboard.isAltActive());

    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("#"));

    // And the record is not kept: a second release with no press is not ours.
    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testSymTakesPrecedenceOverAlt()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_RIGHTALT), Qt::NoModifier, &text);

    QVERIFY(keyboard.isAltActive());
    QVERIFY(keyboard.isSymActive());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QString::fromUtf8("§"));
}

void Ut_HardwareKeyboard::testOwnsAltModifier()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    QVERIFY(!keyboard.ownsAltModifier());

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    QVERIFY(keyboard.ownsAltModifier());

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::AltModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::AltModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);

    QVERIFY(!keyboard.ownsAltModifier());
}

void Ut_HardwareKeyboard::testShiftKeyIsPassedOnButLatches()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile(true, true));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    // NotHandled on purpose: swallowing Shift would cost capitals altogether,
    // because holding it is Qt's own ShiftModifier doing the work.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);

    QVERIFY(keyboard.shiftLatchActive());
}

void Ut_HardwareKeyboard::testShiftLatchIsSpentWhenConsumed()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile(true, true));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    QVERIFY(keyboard.shiftLatchActive());

    keyboard.consumeShiftLatch();
    QVERIFY(!keyboard.shiftLatchActive());

    // A lock survives being spent: two taps, then consume, and it is still on.
    for (int tap = 0; tap < 2; ++tap) {
        keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
        keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    }
    QVERIFY(keyboard.shiftLatchActive());
    keyboard.consumeShiftLatch();
    QVERIFY(keyboard.shiftLatchActive());
}

void Ut_HardwareKeyboard::testShiftLatchNeedsAProfileThatAsksForIt()
{
    // Without shiftKeys the key is Qt's business alone, and a latch must not
    // appear from nowhere and capitalise the letter after every Shift.
    writeProfile(QStringLiteral("titan.json"), titanProfile(true, false));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);

    QVERIFY(!keyboard.shiftLatchActive());
}

void Ut_HardwareKeyboard::testResetClearsEveryLatch()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile(true, true));
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTSHIFT), Qt::NoModifier, &text);
    QVERIFY(keyboard.isAltActive());
    QVERIFY(keyboard.shiftLatchActive());

    // Focus left the field: a latch belongs to the field it was made in.
    keyboard.reset();

    QVERIFY(!keyboard.isAltActive());
    QVERIFY(!keyboard.isSymActive());
    QVERIFY(!keyboard.shiftLatchActive());

    // And a half-finished keystroke is forgotten with it, rather than the
    // release arriving in the next field as text.
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text);
    keyboard.reset();
    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testLevelChangedIsEmitted()
{
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QSignalSpy changed(&keyboard, &HardwareKeyboard::levelChanged);
    QVERIFY(changed.isValid());
    QString text;

    keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    QCOMPARE(changed.count(), 1);

    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_LEFTALT), Qt::NoModifier, &text);
    QCOMPARE(changed.count(), 2);

    // The QML that draws the Alt and Sym indicators is bound to this, so a
    // level that changes without saying so leaves a stale badge on screen.
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text);
    QCOMPARE(changed.count(), 3);
}

/*
 * A field that can only hold a number wants the digit printed on the key, not
 * the letter the key is called. These cover the bargain and its limits: it is
 * the focused field that asks for it, it applies only where there is a digit
 * to give, and it never overrides a level the user selected by hand.
 */

void Ut_HardwareKeyboard::testNumericFieldIsOffByDefault()
{
    writeProfile(QStringLiteral("legendonly.json"), legendOnlyProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QVERIFY(keyboard.isPresent());
    QVERIFY(keyboard.digitsPreferred().isEmpty());

    // An ordinary field: W is a W, and this profile resolves nothing at all.
    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testNumericFieldTakesTheDigitOffTheKeyFace()
{
    writeProfile(QStringLiteral("legendonly.json"), legendOnlyProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    keyboard.setDigitsPreferred(QStringLiteral("0123456789*#+"));

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("1"));
}

void Ut_HardwareKeyboard::testNumericFieldLeavesAKeyWithNoDigitAlone()
{
    writeProfile(QStringLiteral("legendonly.json"), legendOnlyProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    keyboard.setDigitsPreferred(QStringLiteral("0123456789*#+"));

    // KEY_T's Alt legend is "@", which is not a dial character. The key is left
    // to travel the path it always did rather than being silently rewritten:
    // a field that turns out to accept a letter is not being fought.
    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_T), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);

    // And a key with no legend at all is equally untouched.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_G), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testNumericFieldLeavesAHeldLevelAlone()
{
    // On a keyboard whose levels this plugin does own, a user holding Shift or
    // Alt has said what they want. The numeric field is for the user who has
    // said nothing.
    writeProfile(QStringLiteral("titan.json"), titanProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    keyboard.setDigitsPreferred(QStringLiteral("0123456789*#+"));

    QString text;

    // Shift: the shift level, not the alt legend.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::ShiftModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("Q"));
    keyboard.handleKey(QEvent::KeyRelease, sc(KEY_Q), Qt::ShiftModifier, &text);

    // Sym held: the sym level, even though alt would have given a "#" the
    // field would have taken.
    keyboard.handleKey(QEvent::KeyPress, sc(KEY_RIGHTALT), Qt::NoModifier, &text);
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_Q), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QString::fromUtf8("\xc2\xa7"));
}

void Ut_HardwareKeyboard::testNumericFieldReleaseReplaysTheDigit()
{
    // The release is answered out of the same table as any other resolved key,
    // so a field that stops being numeric mid-keystroke cannot turn one press
    // into two different characters.
    writeProfile(QStringLiteral("legendonly.json"), legendOnlyProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    keyboard.setDigitsPreferred(QStringLiteral("0123456789"));

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::Text);

    keyboard.setDigitsPreferred(QString());

    text.clear();
    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::Text);
    QCOMPARE(text, QStringLiteral("1"));
}

void Ut_HardwareKeyboard::testNumericFieldNeedsAProfile()
{
    // Nothing installed: there is no legend to read a digit off, and asking for
    // one must not make the class start answering for keys it knows nothing
    // about.
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QVERIFY(!keyboard.isPresent());

    keyboard.setDigitsPreferred(QStringLiteral("0123456789"));

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_W), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testLegendOnlyProfileClaimsNoLevelKeys()
{
    // The whole point of the shape: the driver keeps resolving Alt and Sym, so
    // the profile must not consume those keys, must not latch anything, and
    // must leave the Alt modifier meaning what it always meant.
    writeProfile(QStringLiteral("legendonly.json"), legendOnlyProfile());
    writeDevices(deviceBlock(QStringLiteral("aw9523-key"), QwertyKeys));

    HardwareKeyboard keyboard;
    QVERIFY(keyboard.isPresent());

    QString text;
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_LEFTALT), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
    QVERIFY(!keyboard.isAltActive());
    QVERIFY(!keyboard.ownsAltModifier());

    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, sc(KEY_RIGHTALT), Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
    QVERIFY(!keyboard.isSymActive());
    QVERIFY(!keyboard.ownsAltModifier());
}

void Ut_HardwareKeyboard::testShippedProfilesAreWellFormed_data()
{
    QTest::addColumn<QString>("path");

    QDir dir(QStringLiteral(LUNEOS_KEYBOARD_SOURCE_DATA_DIR "/hwkeyboard"));
    QVERIFY2(dir.exists(), qUtf8Printable(dir.path()));

    const QStringList files(dir.entryList(QStringList(QStringLiteral("*.json")),
                                          QDir::Files, QDir::Name));

    // A guard against this test passing because it found nothing to check.
    QVERIFY2(!files.isEmpty(), "no profiles found to check");

    for (const QString &file : files)
        QTest::newRow(qUtf8Printable(file)) << dir.filePath(file);
}

void Ut_HardwareKeyboard::testShippedProfilesAreWellFormed()
{
    // HardwareKeyboard reports a bad profile with qWarning and carries on, which
    // is right on a device and useless as a gate: a profile that stopped loading
    // would ship and only be noticed by someone whose keyboard went quiet. Read
    // them directly instead, and insist.
    QFETCH(QString, path);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QJsonParseError error {};
    const QJsonDocument document(QJsonDocument::fromJson(file.readAll(), &error));

    QVERIFY2(!document.isNull(), qUtf8Printable(error.errorString()));
    QVERIFY2(document.isObject(), "a profile is a JSON object");

    const QJsonObject object(document.object());

    QVERIFY2(!object.value(QStringLiteral("name")).toString().isEmpty(),
             "a profile without a name is dropped on load");

    const QJsonObject match(object.value(QStringLiteral("match")).toObject());
    QVERIFY2(!match.value(QStringLiteral("inputDeviceNames")).toArray().isEmpty(),
             "a profile with no input device names can never match anything");

    const QJsonObject levels(object.value(QStringLiteral("levels")).toObject());
    QVERIFY2(!levels.isEmpty(), "a profile with no levels resolves no keys");

    // Every level name has to be one this build knows, every scancode has to be
    // a number, and every mapping has to produce something.
    const QStringList known { QStringLiteral("base"), QStringLiteral("shift"),
                              QStringLiteral("alt"), QStringLiteral("sym") };

    for (auto level = levels.constBegin(); level != levels.constEnd(); ++level) {
        QVERIFY2(known.contains(level.key()),
                 qUtf8Printable(QStringLiteral("unknown level \"%1\"").arg(level.key())));

        const QJsonObject entries(level.value().toObject());
        QVERIFY2(!entries.isEmpty(),
                 qUtf8Printable(QStringLiteral("level \"%1\" is empty").arg(level.key())));

        for (auto entry = entries.constBegin(); entry != entries.constEnd(); ++entry) {
            bool numeric = false;
            const uint code = entry.key().toUInt(&numeric);

            QVERIFY2(numeric,
                     qUtf8Printable(QStringLiteral("level \"%1\" key \"%2\" is not a scancode")
                                        .arg(level.key(), entry.key())));
            QVERIFY2(code <= KEY_MAX,
                     qUtf8Printable(QStringLiteral("scancode %1 is past KEY_MAX").arg(code)));
            QVERIFY2(!entry.value().toString().isEmpty(),
                     qUtf8Printable(QStringLiteral("level \"%1\" scancode %2 maps to nothing")
                                        .arg(level.key()).arg(code)));
        }
    }

    // A level key listed in two roles would make the state machine's choice of
    // which latch to move depend on the order the sets are tested in.
    const QSet<int> alt(readCodes(object.value(QStringLiteral("altKeys")).toArray()));
    const QSet<int> sym(readCodes(object.value(QStringLiteral("symKeys")).toArray()));
    const QSet<int> shift(readCodes(object.value(QStringLiteral("shiftKeys")).toArray()));

    QVERIFY2((alt & sym).isEmpty(), "a key cannot be both Alt and Sym");
    QVERIFY2((alt & shift).isEmpty(), "a key cannot be both Alt and Shift");
    QVERIFY2((sym & shift).isEmpty(), "a key cannot be both Sym and Shift");
}

//! A key the profile names for the emoji panel is answered with EmojiPanel and
//! never becomes text - and its release is swallowed too, because handing the
//! application half an event for a key it knows nothing about is how a stray
//! keypress reaches a text field.
void Ut_HardwareKeyboard::testEmojiKeyIsTakenWholeAndInsertsNothing()
{
    writeProfile(QStringLiteral("emoji.json"), QStringLiteral(R"({
        "name": "emoji-test",
        "match": { "inputDeviceNames": ["test-kbd"] },
        "altKeys": [], "symKeys": [], "shiftKeys": [],
        "emojiKeys": [585],
        "levels": { "alt": { "17": "1" } }
    })").toUtf8());
    writeDevices(deviceBlock(QStringLiteral("test-kbd"), QwertyKeys));

    HardwareKeyboard keyboard;
    keyboard.rescan();
    QVERIFY2(keyboard.isPresent(), "the test profile should have matched");

    // handleKey() takes the scancode as it arrives from the input method -
    // evdev's, offset by 8 - and the profile names evdev's. KEY_EMOJI_PICKER is
    // 585, so 593 is what comes in.
    const quint32 emojiScanCode = 585 + 8;

    QString text("unchanged");
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, emojiScanCode, Qt::NoModifier, &text),
             HardwareKeyboard::EmojiPanel);
    QCOMPARE(text, QStringLiteral("unchanged"));

    QCOMPARE(keyboard.handleKey(QEvent::KeyRelease, emojiScanCode, Qt::NoModifier, &text),
             HardwareKeyboard::Consumed);
    QCOMPARE(text, QStringLiteral("unchanged"));

    // A key it does not name is untouched.
    QCOMPARE(keyboard.handleKey(QEvent::KeyPress, 30 + 8, Qt::NoModifier, &text),
             HardwareKeyboard::NotHandled);
}

void Ut_HardwareKeyboard::testShippedProfilesLoad()
{
    // And through the real loader, by name, which is the other half: a profile
    // can be valid JSON and still be dropped by readProfile().
    QDir dir(QStringLiteral(LUNEOS_KEYBOARD_SOURCE_DATA_DIR "/hwkeyboard"));
    const QStringList files(dir.entryList(QStringList(QStringLiteral("*.json")),
                                          QDir::Files, QDir::Name));
    QVERIFY(!files.isEmpty());

    qputenv("LUNEOS_KEYBOARD_HW_LAYOUT_DIR", dir.path().toLocal8Bit());

    for (const QString &file : files) {
        QFile json(dir.filePath(file));
        QVERIFY(json.open(QIODevice::ReadOnly));

        const QString name(QJsonDocument::fromJson(json.readAll())
                               .object()
                               .value(QStringLiteral("name"))
                               .toString());
        QVERIFY(!name.isEmpty());

        qputenv("LUNEOS_KEYBOARD_HW_LAYOUT", name.toLocal8Bit());

        const HardwareKeyboard keyboard;

        QVERIFY2(keyboard.isPresent(),
                 qUtf8Printable(QStringLiteral("%1 declares the profile \"%2\" but the"
                                               " loader did not keep it")
                                    .arg(file, name)));
        QCOMPARE(keyboard.profileName(), name);
    }
}

QTEST_GUILESS_MAIN(Ut_HardwareKeyboard)
#include "ut_hardwarekeyboard.moc"
