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

#include "models/key.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"
#include "models/layout.h"


#include "view/setup.h"

#include <maliit/plugins/subviewdescription.h>
#include <maliit/plugins/updateevent.h>
#include <maliit/namespace.h>

#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>


class MImUpdateEvent;

using namespace MaliitKeyboard;

namespace {

Qt::ScreenOrientation rotationAngleToScreenOrientation(int angle)
{
    bool portraitIsPrimary = QGuiApplication::primaryScreen()->primaryOrientation()
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

void InputMethod::reset()
{
    hide();
}

void InputMethod::setPreedit(const QString &preedit,
                             int cursor_position)
{
    Q_UNUSED(cursor_position)
    Q_D(InputMethod);
    d->editor.replacePreedit(preedit);
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
    if (focusIn) {
        checkInitialAutocaps();
    } else {
        hide();
    }
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

}
// todo remove
void InputMethod::updateKey(const QString &key_id,
                            const MKeyOverride::KeyOverrideAttributes changed_attributes)
{
    Q_D(InputMethod);

    Q_UNUSED(changed_attributes);

    QMap<QString, SharedOverride>::iterator iter(d->key_overrides.find(key_id));

    if (iter != d->key_overrides.end()) {
        const Key &override_key(overrideToKey(iter.value()));
        Logic::KeyOverrides overrides_update;

        overrides_update.insert(key_id, override_key);
    }
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
    bool ok = d->host->surroundingText(text, position);
    if (ok) {
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

    d->editor.clearPreedit();
    d->editor.wordEngine()->setEnabled( d->wordEngineEnabled );
}

//! \brief InputMethod::contentType returns the type, of the input field, like free text, email, url
//! \return
InputMethod::TextContentType InputMethod::contentType()
{
    Q_D(InputMethod);
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
        bool ok = d->host->surroundingText(text, position);
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
//! \param keyboardSize of the new size. For example "XS", "S", "M" or "L"
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
//! \param keyboardLayout of the new layout. For example "LuneOS", "Dvorak" or "Thumb"
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

    QRect visibleRect = d->m_geometry->visibleRect().toRect();

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
