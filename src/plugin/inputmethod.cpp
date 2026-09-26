/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
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

#include "inputmethod.h"
#include "inputmethod_p.h"

#include "coreutils.h"
#include "models/key.h"
#include "models/text.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"
#include "models/layout.h"
#include "keyboardlogging.h"

#include <QTimer>


#include "view/setup.h"

#include <maliit/plugins/subviewdescription.h>
#include <maliit/plugins/updateevent.h>
#include <maliit/namespace.h>

#include <QFile>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QWidget>


class MImUpdateEvent;

using namespace MaliitKeyboard;

namespace {

Qt::ScreenOrientation rotationAngleToScreenOrientation(int angle)
{
    const bool portraitIsPrimary = QGuiApplication::primaryScreen()->primaryOrientation()
        == Qt::PortraitOrientation;

    switch (angle) {
        case 0:
            return portraitIsPrimary ? Qt::PortraitOrientation
                                     : Qt::LandscapeOrientation;
            break;
        case 90:
            return portraitIsPrimary ? Qt::InvertedLandscapeOrientation
                                     : Qt::PortraitOrientation;
            break;
        case 180:
            return portraitIsPrimary ? Qt::InvertedPortraitOrientation
                                     : Qt::InvertedLandscapeOrientation;
            break;
        case 270:
        default:
            return portraitIsPrimary ? Qt::LandscapeOrientation
                                     : Qt::InvertedPortraitOrientation;
            break;
    }
}

const QString g_maliit_keyboard_qml(LUNEOS_KEYBOARD_DATA_DIR "/Keyboard.qml");

Key overrideToKey(const SharedOverride &override)
{
    Key key;

    key.rLabel() = override->label();
    key.setIcon(override->icon().toUtf8());
    // TODO: hightlighted and enabled information are not available in
    // Key. Should we just really create a KeyOverride model?

    return key;
}

} // unnamed namespace

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(this, host))
{
    Q_D(InputMethod);

    // FIXME: Reconnect feedback instance.
    Setup::connectAll(&d->event_handler, &d->editor);
    connect(&d->editor,  SIGNAL(autoCapsActivated()), this, SIGNAL(activateAutocaps()));

    connect(this, SIGNAL(contentTypeChanged(TextContentType)), this, SLOT(setContentType(TextContentType)));
	connect(this, SIGNAL(keyboardSizeChanged(QString)), this, SLOT(setKeyboardSize(QString)));
    connect(this, SIGNAL(keyboardLayoutChanged(QString)), this, SLOT(setKeyboardLayout(QString)));
    connect(this, SIGNAL(activeLanguageChanged(QString)), d->editor.wordEngine(), SLOT(onLanguageChanged(QString)));
    connect(d->m_geometry, SIGNAL(visibleRectChanged()), this, SLOT(onVisibleRectChanged()));
    connect(d->m_geometry, SIGNAL(popoverRectChanged()), this, SLOT(updateWindowMask()));

    // Hardware T9 multi-tap inactivity timer: when it fires, the character
    // currently being cycled is committed and the next keypress starts anew.
    d->t9Timer = new QTimer(this);
    d->t9Timer->setSingleShot(true);
    d->t9Timer->setInterval(1500); // multi-tap commit window: academic (MacKenzie) 1.5s; Android uses 2s
    connect(d->t9Timer, SIGNAL(timeout()), this, SLOT(finalizeT9()));

    d->registerFeedbackSetting();
    d->registerAutoCorrectSetting();
    d->registerAutoCapsSetting();
    d->registerWordEngineSetting();
    d->registerActiveLanguage();
    d->registerEnabledLanguages();
    d->registerKeyboardSize();
    d->registerKeyboardLayout();
	
    //fire signal so all listeners know what active language is
    Q_EMIT activeLanguageChanged(d->activeLanguage);
	
    //fire signal so all listeners know what keyboard size is
    Q_EMIT keyboardSizeChanged(d->keyboardSize);

    //fire signal so all listeners know what keyboard layout is
    Q_EMIT keyboardLayoutChanged(d->keyboardLayout);

    // Setting layout orientation depends on word engine and hide word ribbon
    // settings to be initialized first:
    d->setLayoutOrientation(d->appsCurrentOrientation);

    d->view->setSource(QUrl::fromLocalFile(g_maliit_keyboard_qml));
    d->view->setGeometry(qGuiApp->primaryScreen()->geometry());
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);

    d->view->setVisible(true);
    d->m_geometry->setShown(true);
}

void InputMethod::hide()
{
    Q_D(InputMethod);
    d->closeOskWindow();
}

//! \brief Called by the framework when the application resets its input
//! context -- the focused field changed, its cursor moved, or its text was
//! changed behind our back.
//!
//! The application has dropped whatever preedit it was showing, so ours has
//! to go too. It is sent in full on every keystroke, so a preedit left over
//! from before the reset would come back attached to the next letter typed.
void InputMethod::reset()
{
    Q_D(InputMethod);

    d->dropPreedit();
}

void InputMethod::setPreedit(const QString &preedit,
                             int cursor_position)
{
    Q_UNUSED(cursor_position)
    Q_D(InputMethod);
    d->editor.replacePreedit(preedit);
}

//! \brief Handles a key coming from a physical keyboard.
//!
//! While an editor is focused the compositor hands every key to whoever holds
//! the input method's keyboard grab, so a hardware key never reaches the
//! application on its own -- this plugin has to deliver it. Text producing keys
//! are pushed through the same editor path the on-screen keyboard uses, which
//! keeps preedit, word prediction and auto-caps consistent between the two
//! keyboards. Everything else (arrows, Tab, Escape, function keys, shortcuts)
//! is handed back to the application untouched.
//!
//! On a device whose physical keyboard carries Alt and Sym levels, those are
//! resolved first by HardwareKeyboard, which needs the scancode -- by the time
//! a key has a Qt::Key and a text the legend printed on the key face is gone.
// The multi-tap cycle for each numeric key, ending in the digit itself. Empty
// for keys that are not part of the T9 pad.
static QString t9CycleFor(Qt::Key keyCode)
{
    switch (keyCode) {
    case Qt::Key_1: return QStringLiteral(".,?!1");
    case Qt::Key_2: return QStringLiteral("abc2");
    case Qt::Key_3: return QStringLiteral("def3");
    case Qt::Key_4: return QStringLiteral("ghi4");
    case Qt::Key_5: return QStringLiteral("jkl5");
    case Qt::Key_6: return QStringLiteral("mno6");
    case Qt::Key_7: return QStringLiteral("pqrs7");
    case Qt::Key_8: return QStringLiteral("tuv8");
    case Qt::Key_9: return QStringLiteral("wxyz9");
    case Qt::Key_0: return QStringLiteral(" 0");
    default:        return QString();
    }
}

bool InputMethod::t9HandleKey(QEvent::Type keyType, Qt::Key keyCode, bool autoRepeat)
{
    Q_D(InputMethod);

    const QString cycle = t9CycleFor(keyCode);

    // Auto-repeat is held-key repetition, not a deliberate tap, so it must not
    // advance the cycle. There is deliberately no burst de-bounce beyond that:
    // measured on this device one physical tap produces exactly one KeyPress
    // and one KeyRelease with autoRepeat clear, so there is nothing to
    // collapse, while the window that used to be here swallowed genuine taps
    // made less than ~180ms apart.
    if (keyType == QEvent::KeyPress && autoRepeat && !cycle.isEmpty())
        return true;

    // Only in text fields. Number/PhoneNumber fields (the dialer) must receive
    // raw digits, so leave those to the normal path.
    if (d->contentType != FreeTextContentType && d->contentType != EmailContentType) {
        qCInfo(lcKeys, "t9: declined, contentType %d takes raw digits",
                int(d->contentType));
        return false;
    }

    // Backspace while a character is being cycled cancels it outright rather
    // than committing then deleting.
    if (keyCode == Qt::Key_Backspace && d->t9Key != 0) {
        if (keyType == QEvent::KeyPress) {
            d->resetT9();
            d->editor.clearPreedit();
        }
        return true;
    }

    if (cycle.isEmpty()) {
        // A non-keypad key ends the character being cycled, so it is committed
        // before the application sees the new key.
        if (d->t9Key != 0 && keyType == QEvent::KeyPress)
            finalizeT9();
        return false;
    }

    // Act on the press; swallow the release.
    if (keyType != QEvent::KeyPress)
        return true;

    if (keyCode == d->t9Key && d->t9Timer->isActive()) {
        // Same key within the window: advance the multi-tap cycle in place.
        d->t9Index = (d->t9Index + 1) % cycle.length();
    } else {
        // New character: fix the previous one (if any) and start fresh.
        finalizeT9();
        d->t9Key = keyCode;
        d->t9Index = 0;
    }
    qCInfo(lcKeys, "t9: '%s' (index %d of \"%s\")",
            qPrintable(QString(cycle.at(d->t9Index))), d->t9Index, qPrintable(cycle));
    d->editor.replacePreedit(QString(cycle.at(d->t9Index)));
    d->t9Timer->start();
    return true;
}

//! \brief Commits the character currently being cycled, if any.
void InputMethod::finalizeT9()
{
    Q_D(InputMethod);

    if (d->t9Key == 0)
        return;

    // Drop the cycle state before touching the editor. Committing goes out
    // through the input-method host and can come straight back as update() or
    // reset(), and those call dropPreedit() -> resetT9(); clearing first keeps
    // that re-entrancy from finalising the same character twice.
    d->resetT9();

    qCInfo(lcKeys, "t9: committing '%s'", qPrintable(d->editor.text()->preedit()));

    // The preedit already holds the character being cycled, so commit it as it
    // stands. Deliberately Editor::commit() and not replaceAndCommitPreedit():
    // that one is the "user picked a candidate" path and runs the preedit
    // through AbstractLanguageFeatures::appendixForReplacedPreedit(), which
    // returns " " for the western languages -- every multi-tap character came
    // out followed by a space and no word could be typed.
    d->editor.commit();
}

void InputMethod::processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                  Qt::KeyboardModifiers modifiers,
                                  const QString &text, bool autoRepeat, int count,
                                  quint32 nativeScanCode, quint32 nativeModifiers,
                                  unsigned long time)
{
    Q_D(InputMethod);

    Key key;

    // Devices with a physical QWERTY print a second and sometimes a third
    // character on each key face, reached with Alt and Sym. The kernel reports
    // only the plain scancode for those, so resolve them before anything else
    // looks at the key.
    QString hardwareText;
    // Before handleKey(), which spends a latched level on any key the profile
    // does not map - Backspace among them.
    const bool altLevelActive = d->hardwareKeyboard.isAltActive();
    const HardwareKeyboard::Result hardwareResult =
        d->hardwareKeyboard.handleKey(keyType, nativeScanCode, modifiers,
                                      &hardwareText);

    if (hardwareResult == HardwareKeyboard::Consumed) {
        // An Alt or Sym key on its own: it selects a level, it is not input.
        return;
    }

    // Those same keys sit on the scancodes a stock keymap calls Alt and AltGr,
    // so while a profile owns them the Alt bit means "alternate character",
    // not "keyboard shortcut".
    Qt::KeyboardModifiers effectiveModifiers = modifiers;
    if (d->hardwareKeyboard.ownsAltModifier())
        effectiveModifiers &= ~Qt::AltModifier;

    const bool isShortcut = effectiveModifiers & (Qt::ControlModifier | Qt::AltModifier |
                                                  Qt::MetaModifier);

    qCInfo(lcKeys, "key %s 0x%x text='%s' repeat=%d scancode=%u mods=0x%x hw=%d->'%s' shortcut=%d shiftlatch=%d",
            keyType == QEvent::KeyPress ? "press" : "release",
            int(keyCode), qPrintable(text), int(autoRepeat),
            unsigned(nativeScanCode), int(modifiers),
            int(hardwareResult), qPrintable(hardwareText), int(isShortcut),
            int(d->hardwareKeyboard.shiftLatchActive()));

    // Hardware T9 numeric keypad -> letters (multi-tap) in text fields. Only
    // where there is a keypad to multi-tap on: a keyboard with a number row
    // wants 2 to be a 2, and without that test every digit on a QWERTY device
    // came out as a letter. Placed after the profile has had its say too, since
    // a key a profile already resolved to an alternate character is a QWERTY
    // level rather than a keypad digit, and a modifier makes this a shortcut.
    if (hardwareResult != HardwareKeyboard::Text && !isShortcut
        && d->hardwareKeyboard.hasTelephoneKeypad()
        && t9HandleKey(keyType, keyCode, autoRepeat))
        return;

    if (hardwareResult == HardwareKeyboard::Text) {
        if (hardwareText == QLatin1String(" ")) {
            key.setAction(Key::ActionSpace);
        } else {
            key.setAction(Key::ActionInsert);
            key.setLabel(hardwareText);
        }
    } else if (isShortcut) {
        key.setAction(Key::NumActions);
    } else switch (keyCode) {
    case Qt::Key_Backspace:
        // Alt+Backspace deletes the word before the cursor, as it does on the
        // keyboards that print an Alt level on their key faces. The Alt bit
        // itself was discounted above - a profile owns that key - so the level
        // state is what says Alt was down, and the editor already knows how to
        // delete a word and how to repeat it while the key is held.
        key.setAction(altLevelActive ? Key::ActionBackspaceWord
                                     : Key::ActionBackspace);
        break;

    case Qt::Key_Space:
        key.setAction(Key::ActionSpace);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        key.setAction(Key::ActionReturn);
        break;

    default:
        if (text.size() == 1 && text.at(0).isPrint()) {
            key.setAction(Key::ActionInsert);

            // A tapped Shift is spent here rather than by the hardware layer.
            // The key itself is passed through so that holding it still gives
            // Qt's ShiftModifier, which means a tap leaves nothing behind for
            // the character that follows and the capital has to be applied
            // now. A profile that maps this key at its shift level has already
            // answered Text and never reaches this branch.
            //
            // The label is capitalised on both the press and the release, but
            // the latch is only spent on the release: AbstractTextEditor
            // appends the label in onKeyReleased(), so consuming it on the
            // press would leave the release - the event that actually inserts
            // the character - building a lowercase label from a latch that had
            // already gone.
            //
            // Auto-capitalisation is applied in the same place and for the same
            // reason: a letter from a physical keyboard never passes through a
            // Key the view shifted, so the view's auto-caps never sees it. The
            // rule is the editor's own, i.e. the language's, not a second copy
            // of it here. A latch still has to be spent; a capital that
            // auto-caps asked for has nothing to spend.
            QString label(text);
            const bool shiftLatched = d->hardwareKeyboard.shiftLatchActive();
            if (shiftLatched or d->editor.atAutoCapsPosition()) {
                label = text.toUpper();
                if (shiftLatched and keyType == QEvent::KeyRelease)
                    d->hardwareKeyboard.consumeShiftLatch();
            }

            key.setLabel(label);
        } else {
            key.setAction(Key::NumActions);
        }
        break;
    }

    if (key.action() == Key::NumActions) {
        // Not ours: cursor keys, Home/End, Delete, function keys, shortcuts.
        // Commit first -- otherwise the application moves its cursor away from
        // a preedit the editor still holds, and every later keystroke is
        // applied against a stale position.
        if (keyType == QEvent::KeyPress)
            d->editor.commit();

        MAbstractInputMethod::processKeyEvent(keyType, keyCode, modifiers, text,
                                              autoRepeat, count, nativeScanCode,
                                              nativeModifiers, time);
        return;
    }

    if (keyType == QEvent::KeyPress)
        d->editor.onKeyPressed(key);
    else if (keyType == QEvent::KeyRelease)
        d->editor.onKeyReleased(key);
}

void InputMethod::switchContext(Maliit::SwitchDirection direction,
                                bool animated)
{
    Q_UNUSED(direction)
    Q_UNUSED(animated)
}

QList<MAbstractInputMethod::MInputMethodSubView>
InputMethod::subViews(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    QList<MInputMethodSubView> views;

	MInputMethodSubView v;
	v.subViewId = d->activeLanguage;
	views.append(v);

    return views;
}

// called by settings change/language change
void InputMethod::setActiveSubView(const QString &id,
                                   Maliit::HandlerState state)
{
    Q_UNUSED(state)
    Q_UNUSED(id);

	setActiveLanguage(id);
}

QString InputMethod::activeSubView(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    return d->activeLanguage;
}

void InputMethod::handleFocusChange(bool focusIn)
{
    Q_D(InputMethod);

    if (focusIn) {
        checkInitialAutocaps();
    } else {
        // Whatever was in the preedit belongs to the field we just left.
        d->dropPreedit();
        hide();
    }

    // A latched Alt or Sym, and any half-cycled T9 character, belonged to the
    // field we just left.
    d->hardwareKeyboard.reset();
    d->resetT9();

    // this is for hardware keyboard
    inputMethodHost()->setRedirectKeys(focusIn);

    publishTextFocus(focusIn);
}

//! \brief Publishes whether a text field has focus, for readers outside the
//!        compositor.
//!
//! kbdscroll - which turns a slide over a capacitive keyboard or a trackpad into
//! scrolling - deletes the word before the cursor on a right-to-left slide, but
//! only in a text field; anywhere else that slide is a sideways drag. Nothing in
//! the stack publishes "a field has focus" outside the compositor and its input
//! method, and a file is the cheapest thing a C daemon can look at. Writing it
//! is best effort: on a read-only /run, or with no such directory, there is
//! simply no reader.
void InputMethod::publishTextFocus(bool focusIn)
{
    static const QString path = qEnvironmentVariableIsSet("MALIIT_TEXT_FOCUS_FILE")
        ? qEnvironmentVariable("MALIIT_TEXT_FOCUS_FILE")
        : QStringLiteral("/run/maliit-text-focus");

    if (path.isEmpty())
        return;

    QFile flag(path);
    if (flag.open(QIODevice::WriteOnly | QIODevice::Truncate))
        flag.write(focusIn ? "1\n" : "0\n");
}

void InputMethod::handleAppOrientationChanged(int angle)
{
    Q_D(InputMethod);

    d->appsCurrentOrientation = rotationAngleToScreenOrientation(angle);
    d->setLayoutOrientation(d->appsCurrentOrientation);
}

void InputMethod::handleClientChange()
{
    // Clients connect to Maliit on startup and disconnect at quit. This method is called
    // for both those events. It makes sense to hide the keyboard always on these events,
    // especially if the client crashes, so that the OSK is closed. Fixes bug lp:1267550
    // Note that clients request OSK to appear & disappear with focus events.
    hide();
}

bool InputMethod::imExtensionEvent(MImExtensionEvent *event)
{
    if (not event or event->type() != MImExtensionEvent::Update) {
        return false;
    }
    return true;
}

void InputMethod::onAutoCorrectSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCorrectEnabled(d->m_settings.autoCorrection());
}

/*!
 * \brief InputMethod::updateAutoCaps enabled the use of auto capitalization
 * when the setting iss eto true, and the text area does not prevent to use it
 */
void InputMethod::updateAutoCaps()
{
    Q_D(InputMethod);
    bool enabled = d->m_settings.autoCapitalization();
    enabled &= d->contentType == FreeTextContentType;
    bool valid = true;
    bool autocap = d->host->autoCapitalizationEnabled(valid);

    // A text field inside a web page reaches maliit with no auto-capitalisation
    // hint at all and the host then answers false, so taking that as a "no"
    // means auto-caps never works in the browser or in any Enyo or Mojo
    // application - which is most of what runs here. Where the host has no
    // opinion, decide from the field itself: free text (tested above) and not a
    // password. The user setting still gates all of it.
    if (not autocap) {
        bool hiddenValid = true;
        const bool hidden = d->host->hiddenText(hiddenValid);
        autocap = not hidden;
        qCInfo(lcKeys, "autocaps: host gave no hint (valid=%d); hidden=%d -> %d",
               int(valid), int(hidden), int(autocap));
    }
    enabled &= autocap;

    if (enabled != d->autocapsEnabled) {
        d->autocapsEnabled = enabled;
        d->editor.setAutoCapsEnabled(enabled);
    }
}

//! \brief InputMethod::onEnabledLanguageSettingsChanged
//! Updates the list of languages that can be used
void InputMethod::onEnabledLanguageSettingsChanged()
{
    Q_D(InputMethod);
    d->truncateEnabledLanguageLocales(d->m_settings.enabledLanguages());
    Q_EMIT enabledLanguagesChanged(d->enabledLanguages);
}
// todo remove
void InputMethod::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    Q_D(InputMethod);

    for (OverridesIterator i(d->key_overrides.begin()), e(d->key_overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            disconnect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                       this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
        }
    }

    d->key_overrides.clear();
    QMap<QString, Key> overriden_keys;

    for (OverridesIterator i(overrides.begin()), e(overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            d->key_overrides.insert(i.key(), override);
            connect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                    this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
            overriden_keys.insert(i.key(), overrideToKey(override));
        }
    }

    const QMap<QString, Key>::const_iterator action(overriden_keys.constFind(
                                                        QLatin1String(CoreUtils::actionKeyId())));
    const QString label(action != overriden_keys.constEnd() ? action->label() : QString());

    if (label != d->actionKeyLabel) {
        d->actionKeyLabel = label;
        Q_EMIT actionKeyLabelChanged(d->actionKeyLabel);
    }
}
// todo remove
void InputMethod::updateKey(const QString &key_id,
                            const MKeyOverride::KeyOverrideAttributes changed_attributes)
{
    Q_D(InputMethod);

    Q_UNUSED(changed_attributes);

    const QMap<QString, SharedOverride>::iterator iter(d->key_overrides.find(key_id));

    if (iter != d->key_overrides.end()) {
        const Key &override_key(overrideToKey(iter.value()));
        Logic::KeyOverrides overrides_update;

        overrides_update.insert(key_id, override_key);

        if (key_id == QLatin1String(CoreUtils::actionKeyId())
            and override_key.label() != d->actionKeyLabel) {
            d->actionKeyLabel = override_key.label();
            Q_EMIT actionKeyLabelChanged(d->actionKeyLabel);
        }
    }
}

//! \brief Republishes the word that space would commit, for the space bar to show.
void InputMethod::onWordCandidatesChanged()
{
    Q_D(InputMethod);

    const QString candidate(d->editor.text() ? d->editor.text()->primaryCandidate()
                                             : QString());

    if (candidate != d->primaryCandidate) {
        d->primaryCandidate = candidate;
        Q_EMIT primaryCandidateChanged(d->primaryCandidate);
    }
}

//! \brief The word that pressing space would commit.
//!
//! drawKeyCap paints this on the space bar rather than leaving it blank:
//! "if (key == Qt::Key_Space) text = m_candidateBar.autoSelectCandidate()".
QString InputMethod::primaryCandidate() const
{
    Q_D(const InputMethod);
    return d->primaryCandidate;
}

//! \brief Label the application asked for on the Return key.
//!
//! Maliit delivers it as the "actionKey" override, which is the equivalent of the
//! reference's PalmIME::EditorState::enterKeyLabel. Empty means a plain "Enter".
QString InputMethod::actionKeyLabel() const
{
    Q_D(const InputMethod);
    return d->actionKeyLabel;
}

void InputMethod::onKeyboardClosed()
{
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
}

void InputMethod::onLayoutWidthChanged(int width)
{
    Q_UNUSED(width);
}

void InputMethod::onLayoutHeightChanged(int height)
{
    Q_UNUSED(height);
}

void InputMethod::deviceOrientationChanged(Qt::ScreenOrientation orientation)
{
    Q_UNUSED(orientation);
    Q_D(InputMethod);

    d->setLayoutOrientation(d->appsCurrentOrientation);
}

void InputMethod::update()
{
    Q_D(InputMethod);

    bool valid;

    bool emitPredictionEnabled = false;

    bool newPredictionEnabled = inputMethodHost()->predictionEnabled(valid);

    if (!valid)
        newPredictionEnabled = true;

    if (d->wordEngineEnabled != newPredictionEnabled) {
        d->wordEngineEnabled = newPredictionEnabled;
        emitPredictionEnabled = true;
    }

    TextContentType newContentType = static_cast<TextContentType>( inputMethodHost()->contentType(valid) );
    if (!valid) {
        newContentType = FreeTextContentType;
    }
    setContentType(newContentType);

    if (emitPredictionEnabled) {
        updateWordEngine();
    }

    QString text;
    int position;
    const bool ok = d->host->surroundingText(text, position);
    if (ok) {
        // The application tells us where its cursor is, but never that it
        // moved it, and not every client sends an input context reset when
        // the user taps somewhere else in the field. While we hold a preedit
        // the cursor belongs inside it: clients that leave the preedit out of
        // the surrounding text they report keep the cursor at its start,
        // clients that count it in put the cursor at its end, and a backspace
        // walks back through it -- so anywhere in [anchor, anchor + length]
        // is us. Outside that span the application moved the cursor away from
        // text we still think we own, and the preedit has to go now: it is
        // sent in full on every keystroke, so keeping it would paste the word
        // typed here into wherever the cursor went.
        const int preeditLength = d->editor.text()->preedit().length();

        if (preeditLength < 1 || d->preeditCursorAnchor < 0) {
            d->preeditCursorAnchor = position;
        } else if (position < d->preeditCursorAnchor
                   || position > d->preeditCursorAnchor + preeditLength) {
            d->dropPreedit();
        }

        d->editor.text()->setSurrounding(text);
        d->editor.text()->setSurroundingOffset(position);
    }

    updateAutoCaps();
}

void InputMethod::updateWordEngine()
{
    Q_D(InputMethod);

    if (d->contentType != FreeTextContentType)
        d->wordEngineEnabled = false;

    // Clears the preedit directly rather than through dropPreedit(), so the
    // T9 cycle that lived in it has to be dropped here too. Reached on every
    // content-type change, which is exactly when a half-cycled letter must not
    // survive into the next field.
    d->resetT9();
    d->editor.clearPreedit();
    d->editor.wordEngine()->setEnabled( d->wordEngineEnabled );
}

//! \brief InputMethod::contentType returns the type, of the input field, like free text, email, url
//! \return
InputMethod::TextContentType InputMethod::contentType()
{
    Q_D(const InputMethod);
    return d->contentType;
}

//! \brief InputMethod::setContentType sets the type, of the input field, like free text, email, url
//! \param contentType
void InputMethod::setContentType(TextContentType contentType)
{
    Q_D(InputMethod);

    if (contentType == d->contentType)
        return;

    setActiveLanguage(d->activeLanguage);

    d->contentType = contentType;
    Q_EMIT contentTypeChanged(contentType);

    updateWordEngine();
    updateAutoCaps();
}

//! \brief InputMethod::checkInitialAutocaps  Checks if the keyboard should be
//! set to uppercase, because the auto caps is enabled and the text is empty.
void InputMethod::checkInitialAutocaps()
{
    Q_D(InputMethod);
    update();

    if (d->autocapsEnabled) {
        QString text;
        int position;
        const bool ok = d->host->surroundingText(text, position);
        if (ok && text.isEmpty() && position == 0)
            Q_EMIT activateAutocaps();
    }
}

//! \brief InputMethod::enabledLanguages list of all languages that can be selected
const QStringList &InputMethod::enabledLanguages() const
{
    Q_D(const InputMethod);
    return d->enabledLanguages;
}

//! \brief InputMethod::activeLanguage returns the language that is currently
//! used by the keyboard
const QString &InputMethod::activeLanguage() const
{
    Q_D(const InputMethod);
    return d->activeLanguage;
}


//! \brief InputMethod::useAudioFeedback is true, when keys should play a audio
//! feedback when pressed
//! \return
bool InputMethod::useAudioFeedback() const
{
    Q_D(const InputMethod);
    return d->m_settings.keyPressFeedback();
}

//! \brief InputMethod::setActiveLanguage
//! Sets the currently active/used language
//! \param newLanguage id of the new language. For example "en" or "es"
//! FIXME check if the language is supported - if not use "en" as fallback
void InputMethod::setActiveLanguage(const QString &newLanguage)
{
    Q_D(InputMethod);

    if (newLanguage.length() != 2) {
        qWarning() << Q_FUNC_INFO << "newLanguage is not valid:" << newLanguage;
        return;
    }

    qDebug() << "in inputMethod.cpp setActiveLanguage() activeLanguage is:" << newLanguage;

    if (d->activeLanguage == newLanguage)
        return;

    d->activeLanguage = newLanguage;
    d->host->setLanguage(newLanguage);
    /// TODO: d->m_settings.setActiveLanguage(newLanguage);

    qDebug() << "in inputMethod.cpp setActiveLanguage() emitting activeLanguageChanged to" << d->activeLanguage;
    Q_EMIT activeLanguageChanged(d->activeLanguage);
}

const QString &InputMethod::keyboardSize() const
{
    Q_D(const InputMethod);
    return d->keyboardSize;
}

//! \brief InputMethod::setKeyboardSize
//! Sets the keyboard size
//! \param newKeyboardSize the new size. For example "XS", "S", "M" or "L"
//! FIXME check if the size is supported - if not use "M" as fallback
void InputMethod::setKeyboardSize(const QString &newKeyboardSize)
{
    Q_D(InputMethod);

    if (newKeyboardSize.length() != 2 && newKeyboardSize.length() != 1) {
        qWarning() << Q_FUNC_INFO << "newKeyboardSize is not valid:" << newKeyboardSize;
        return;
    }

    qDebug() << "in inputMethod.cpp setKeyboardSize() keyboardSize is:" << newKeyboardSize;

    if (d->keyboardSize == newKeyboardSize)
        return;

    d->keyboardSize = newKeyboardSize;
    
    qDebug() << "in inputMethod.cpp setKeyboardSize() emitting keyboardSizeChanged to" << d->keyboardSize;
    Q_EMIT keyboardSizeChanged(d->keyboardSize);
}

const QString &InputMethod::keyboardLayout() const
{
    Q_D(const InputMethod);
    return d->keyboardLayout;
}

//! \brief InputMethod::setKeyboardLayout
//! Sets the keyboard layout
//! \param newKeyboardLayout the new layout. For example "LuneOS", "Dvorak" or "Thumb"
//! FIXME check if the layout is supported - if not use "LuneOS" as fallback
void InputMethod::setKeyboardLayout(const QString &newKeyboardLayout)
{
    Q_D(InputMethod);

    qDebug() << "in inputMethod.cpp setKeyboardLayout() keyboardLayout is:" << newKeyboardLayout;

    if (d->keyboardLayout == newKeyboardLayout)
        return;

    d->keyboardLayout = newKeyboardLayout;

    qDebug() << "in inputMethod.cpp setKeyboardLayout() emitting keyboardLayoutChanged to" << d->keyboardLayout;
    Q_EMIT keyboardLayoutChanged(d->keyboardLayout);
}

void InputMethod::updateWindowMask()
{
    Q_D(InputMethod);

    QRegion vkbMask(d->m_geometry->visibleRect().toRect());
    vkbMask += d->m_geometry->popoverRect().toRect();

    d->view->setMask(vkbMask);
}

void InputMethod::onVisibleRectChanged()
{
    Q_D(InputMethod);

    const QRect visibleRect = d->m_geometry->visibleRect().toRect();

    qDebug() << "keyboard is reporting <x y w h>: <"
                << visibleRect.x()
                << visibleRect.y()
                << visibleRect.width()
                << visibleRect.height()
                << "> as a new visibleRect.";

    inputMethodHost()->setScreenRegion(QRegion(visibleRect));
    inputMethodHost()->setInputMethodArea(visibleRect, d->view);

    // update window mask
    updateWindowMask();

    d->applicationApiWrapper->reportOSKVisible(
                visibleRect.x(),
                visibleRect.y(),
                visibleRect.width(),
                visibleRect.height()
                );
}
