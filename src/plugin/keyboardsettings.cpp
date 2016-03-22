/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2013 Canonical, Ltd.
 * Copyright (C) 2014 Simon Busch <morphis@gravedo.de>
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

#include "keyboardsettings.h"

#include <QDebug>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace MaliitKeyboard;

const QLatin1String ACTIVE_LANGUAGE_KEY = QLatin1String("activeLanguage");
const QLatin1String ENABLED_LANGUAGES_KEY = QLatin1String("enabledLanguages");
const QLatin1String AUTO_CAPITALIZATION_KEY = QLatin1String("autoCapitalization");
const QLatin1String AUTO_COMPLETION_KEY = QLatin1String("autoCompletion");
const QLatin1String PREDICTIVE_TEXT_KEY = QLatin1String("predictiveText");
const QLatin1String SPELL_CHECKING_KEY = QLatin1String("spellChecking");
const QLatin1String KEY_PRESS_FEEDBACK_KEY = QLatin1String("keyPressFeedback");
const QLatin1String KEYBOARD_SIZE_KEY = QLatin1String("keyboardSize");
const QLatin1String KEYBOARD_LAYOUT_KEY = QLatin1String("keyboardLayout");

/*!
 * \brief KeyboardSettings::KeyboardSettings class to load the settings, and
 * listens on runtime to changes of them
 * \param parent
 */
KeyboardSettings::KeyboardSettings(QObject *parent) :
    QObject(parent),
    mActiveLanguage("en"),
    mAutoCapitalization(false),
    mAutoCompletion(false),
    mPredictiveText(false),
    mSpellchecing(false),
    mKeyPressFeedback(false),
    mKeyboardSize("M"),
    mKeyboardLayout("LuneOS")
{
    LSError error;
    LSErrorInit(&error);

    if (!LSRegister(NULL, &mServiceHandle, &error)) {
        qWarning("Failed to register service handle: %s", error.message);
        LSErrorFree(&error);
        return;
    }

    if (!LSGmainAttach(mServiceHandle, g_main_loop_new(g_main_context_default(), TRUE), &error)) {
        qWarning("Failed to attach to glib mainloop: %s", error.message);
        LSErrorFree(&error);
        return;
    }

    if (!LSCall(mServiceHandle, "palm://com.palm.lunabus/signal/registerServerStatus",
              "{\"serviceName\":\"com.palm.systemservice\"}",
              systemServiceStatusCallback, this, &mServerStatusToken, &error)) {
        qWarning("Failed when registering watch for com.palm.systemservice");
        LSErrorFree(&error);
        return;
    }

    g_message("Service setup successfully");
}

bool KeyboardSettings::systemServiceStatusCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    if (!message)
        return true;

    const char *payload = LSMessageGetPayload(message);
    if (!payload)
        return true;

    QJsonDocument document = QJsonDocument::fromJson(QByteArray(payload));

    QJsonObject root = document.object();
    if (!root.contains("connected") || !root.value("connected").isBool())
        return true;

    bool connected = root.value("connected").toBool();
    if (!connected)
        return true;

    KeyboardSettings *settings = static_cast<KeyboardSettings*>(user_data);
    settings->preferenceServiceIsAvailable();

    return true;
}

void KeyboardSettings::preferenceServiceIsAvailable()
{
    LSError error;
    LSErrorInit(&error);

    g_message("preference service is available");

    if (!LSCall(mServiceHandle, "palm://com.palm.systemservice/getPreferences",
              "{\"subscribe\":true,\"keys\":[\"keyboard\"]}",
              preferencesChangedCallback, this, NULL, &error)) {
        qWarning("Setting up subscription for keyboard preferences failed: %s", error.message);
        LSErrorFree(&error);
    }
}


bool KeyboardSettings::preferencesChangedCallback(LSHandle *handle, LSMessage *message, void *user_data)
{
    if (!message)
        return true;

    const char *payload = LSMessageGetPayload(message);
    if (!payload)
        return true;

    g_message("Got updated keyboard preferences from service:");
    g_message("%s", payload);

    QByteArray data(payload);
    KeyboardSettings *settings = static_cast<KeyboardSettings*>(user_data);
    settings->preferencesChanged(data);

    return true;
}

void KeyboardSettings::preferencesChanged(const QByteArray &data)
{
    QJsonDocument document = QJsonDocument::fromJson(data);

    QJsonObject root = document.object();

    if (!root.contains("keyboard") || !root.value("keyboard").isObject())
        return;

    QJsonObject keyboardPref = root.value("keyboard").toObject();

    if (keyboardPref.contains(ACTIVE_LANGUAGE_KEY) && keyboardPref.value(ACTIVE_LANGUAGE_KEY).isString()) {
        QString value = keyboardPref.value(ACTIVE_LANGUAGE_KEY).toString();
        if (value != mActiveLanguage) {
            mActiveLanguage = value;
            Q_EMIT activeLanguageChanged(mActiveLanguage);
        }
    }

    if (keyboardPref.contains(ENABLED_LANGUAGES_KEY) && keyboardPref.value(ENABLED_LANGUAGES_KEY).isArray()) {
        QJsonArray languages = keyboardPref.value(ENABLED_LANGUAGES_KEY).toArray();
        QStringList newLanguages;

        for (int n = 0; n < languages.size(); n++) {
            QJsonValue value = languages.at(n);
            newLanguages.append(value.toString());
        }

        if (newLanguages != mEnabledLanguages) {
            mEnabledLanguages = newLanguages;
            Q_EMIT enabledLanguagesChanged(mEnabledLanguages);
        }
    }

    if (keyboardPref.contains(AUTO_CAPITALIZATION_KEY) && keyboardPref.value(AUTO_CAPITALIZATION_KEY).isBool()) {
        bool value = keyboardPref.value(AUTO_CAPITALIZATION_KEY).toBool();
        if (value != mAutoCapitalization) {
            mAutoCapitalization = value;
            Q_EMIT autoCapitalizationChanged(mAutoCapitalization);
        }
    }

    if (keyboardPref.contains(AUTO_COMPLETION_KEY) && keyboardPref.value(AUTO_COMPLETION_KEY).isBool()) {
        bool value = keyboardPref.value(AUTO_COMPLETION_KEY).toBool();
        if (value != mAutoCompletion) {
            mAutoCompletion = value;
            Q_EMIT autoCompletionChanged(mAutoCompletion);
        }
    }

    if (keyboardPref.contains(PREDICTIVE_TEXT_KEY) && keyboardPref.value(PREDICTIVE_TEXT_KEY).isBool()) {
        bool value = keyboardPref.value(PREDICTIVE_TEXT_KEY).toBool();
        if (value != mPredictiveText) {
            mPredictiveText = value;
            Q_EMIT predictiveTextChanged(mPredictiveText);
        }
    }

    if (keyboardPref.contains(SPELL_CHECKING_KEY) && keyboardPref.value(SPELL_CHECKING_KEY).isBool()) {
        bool value = keyboardPref.value(SPELL_CHECKING_KEY).toBool();
        if (value != mSpellchecing) {
            mSpellchecing = value;
            Q_EMIT spellCheckingChanged(mSpellchecing);
        }
    }

    if (keyboardPref.contains(KEY_PRESS_FEEDBACK_KEY) && keyboardPref.value(KEY_PRESS_FEEDBACK_KEY).isBool()) {
        bool value = keyboardPref.value(KEY_PRESS_FEEDBACK_KEY).toBool();
        if (value != mKeyPressFeedback) {
            mKeyPressFeedback = value;
            Q_EMIT keyPressFeedbackChanged(mKeyPressFeedback);
        }
    }
	
    if (keyboardPref.contains(KEYBOARD_SIZE_KEY) && keyboardPref.value(KEYBOARD_SIZE_KEY).isString()) {
        QString value = keyboardPref.value(KEYBOARD_SIZE_KEY).toString();
        if (value != mKeyboardSize) {
            mKeyboardSize = value;
            Q_EMIT keyboardSizeChanged(mKeyboardSize);
        }
    }

    if (keyboardPref.contains(KEYBOARD_LAYOUT_KEY) && keyboardPref.value(KEYBOARD_LAYOUT_KEY).isString()) {
        QString value = keyboardPref.value(KEYBOARD_LAYOUT_KEY).toString();
        if (value != mKeyboardLayout) {
            mKeyboardLayout = value;
            Q_EMIT keyboardLayoutChanged(mKeyboardLayout);
        }
    }

    mSavedKeyboardPrefs = keyboardPref;
}

/*!
 * \brief KeyboardSettings::activeLanguage returns currently active language
 * \return active language
 */

QString KeyboardSettings::activeLanguage() const
{
    return mActiveLanguage;
}


/*!
 * \brief KeyboardSettings::enabledLanguages returns a list of languages that are
 * active
 * \return
 */
QStringList KeyboardSettings::enabledLanguages() const
{
  return mEnabledLanguages;
}

/*!
 * \brief KeyboardSettings::autoCapitalization returns true id the first letter
 * of each sentence should be capitalized
 * \return
 */
bool KeyboardSettings::autoCapitalization() const
{
  return mAutoCapitalization;
}

/*!
 * \brief KeyboardSettings::autoCompletion returns true if the current word should
 * be completed with first suggestion when hitting space
 * \return
 */
bool KeyboardSettings::autoCompletion() const
{
  return mAutoCompletion;
}

/*!
 * \brief KeyboardSettings::predictiveText returns true, if potential words in the
 * word ribbon should be suggested
 * \return
 */
bool KeyboardSettings::predictiveText() const
{
  return mPredictiveText;
}

/*!
 * \brief KeyboardSettings::spellchecking returns true if spellchecking should be used
 * \return
 */
bool KeyboardSettings::spellchecking() const
{
  return mSpellchecing;
}

/*!
 * \brief KeyboardSettings::keyPressFeedback returns true if feedback is enabled
 * when the user presses a keyboad key
 * \return
 */
bool KeyboardSettings::keyPressFeedback() const
{
  return mKeyPressFeedback;
}

/*!
 * \brief KeyboardSettings::keyboardSize returns current keyboard size
 * \return keyboard size
 */

QString KeyboardSettings::keyboardSize() const
{
    return mKeyboardSize;
}

/*!
 * \brief KeyboardSettings::keyboardLayout returns current keyboard layout
 * \return keyboard layout
 */

QString KeyboardSettings::keyboardLayout() const
{
    return mKeyboardLayout;
}

void KeyboardSettings::savePreferences(InputMethod *q)
{
    QJsonObject keyboardObj;

    keyboardObj.insert(ACTIVE_LANGUAGE_KEY, QJsonValue(q->activeLanguage()));
    keyboardObj.insert(ENABLED_LANGUAGES_KEY, QJsonValue(QJsonArray::fromStringList(q->enabledLanguages())));
    keyboardObj.insert(AUTO_CAPITALIZATION_KEY, QJsonValue(mAutoCapitalization));
    keyboardObj.insert(AUTO_COMPLETION_KEY, QJsonValue(mAutoCompletion));
    keyboardObj.insert(PREDICTIVE_TEXT_KEY, QJsonValue(mPredictiveText));
    keyboardObj.insert(SPELL_CHECKING_KEY, QJsonValue(mSpellchecing));
    keyboardObj.insert(KEY_PRESS_FEEDBACK_KEY, QJsonValue(mKeyPressFeedback));
    keyboardObj.insert(KEYBOARD_SIZE_KEY, QJsonValue(q->keyboardSize()));
    keyboardObj.insert(KEYBOARD_LAYOUT_KEY, QJsonValue(q->keyboardLayout()));

    if (keyboardObj == mSavedKeyboardPrefs)
        return;

    QJsonObject prefObj;
    prefObj.insert("keyboard", keyboardObj);

    QJsonDocument document;
    document.setObject(prefObj);
    QString payload = document.toJson();
    LSError error;
    LSErrorInit(&error);
    if (!LSCallOneReply(mServiceHandle, "palm://com.palm.systemservice/setPreferences",
                        payload.toStdString().c_str(), NULL, this, NULL, &error)) {
        qWarning("Failed to save keyboard preferences: %s", error.message);
        LSErrorFree(&error);
    }
    return;
}

#if 0
/*!
 * \brief KeyboardSettings::settingUpdated slot to handle changes in the settings backend
 * A specialized signal is emitted for the affected setting
 * \param key
 */
void KeyboardSettings::settingUpdated(const QString &key)
{
    if (key == ACTIVE_LANGUAGE_KEY) {
        Q_EMIT activeLanguageChanged(activeLanguage());
        return;
    } else if (key == ENABLED_LANGUAGES_KEY) {
        Q_EMIT enabledLanguagesChanged(enabledLanguages());
        return;
    } else if (key == AUTO_CAPITALIZATION_KEY) {
        Q_EMIT autoCapitalizationChanged(autoCapitalization());
        return;
    } else if (key == AUTO_COMPLETION_KEY) {
        Q_EMIT autoCompletionChanged(autoCompletion());
        return;
    } else if (key == PREDICTIVE_TEXT_KEY) {
        Q_EMIT predictiveTextChanged(predictiveText());
        return;
    } else if (key == SPELL_CHECKING_KEY) {
        Q_EMIT spellCheckingChanged(spellchecking());
        return;
    } else if (key == KEY_PRESS_FEEDBACK_KEY) {
        Q_EMIT keyPressFeedbackChanged(keyPressFeedback());
        return;
    } else if (key == KEYBOARD_SIZE_KEY) {
        Q_EMIT keyboardSizeChanged(keyboardSize());
        return;
    }
	

    qWarning() << Q_FUNC_INFO << "unknown settings key:" << key;
    return;
}
#endif
