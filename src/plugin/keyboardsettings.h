/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef KEYBOARDSETTINGS_H
#define KEYBOARDSETTINGS_H

#include <QObject>
#include <QStringList>

#include <luna-service2/lunaservice.h>
#include "inputmethod.h"

namespace MaliitKeyboard {

class KeyboardSettings : public QObject
{
    Q_OBJECT
public:
    explicit KeyboardSettings(QObject *parent = 0);
    
    QString activeLanguage() const;
    QStringList enabledLanguages() const;
    bool autoCapitalization() const;
    bool autoCorrection() const;
    bool predictiveText() const;
    bool spellchecking() const;
    bool keyPressFeedback() const;
    QString keyboardSize() const;
    QString keyboardLayout() const;

    static bool systemServiceStatusCallback(LSHandle *handle, LSMessage *message, void *user_data);
    static bool preferencesChangedCallback(LSHandle *handle, LSMessage *message, void *user_data);

    void preferenceServiceIsAvailable();
    void preferencesChanged(const QByteArray &data);
    void savePreferences(InputMethod *q);

Q_SIGNALS:
    void activeLanguageChanged(QString);
    void enabledLanguagesChanged(QStringList);
    void autoCapitalizationChanged(bool);
    void autoCorrectionChanged(bool);
    void predictiveTextChanged(bool);
    void spellCheckingChanged(bool);
    void keyPressFeedbackChanged(bool);
    void keyboardSizeChanged(QString);
    void keyboardLayoutChanged(QString);

private:
    LSHandle *mServiceHandle;
    LSMessageToken mServerStatusToken;
    QStringList mEnabledLanguages;
    QString mActiveLanguage;
    bool mAutoCapitalization;
    bool mAutoCorrection;
    bool mPredictiveText;
    bool mSpellchecking;
    bool mKeyPressFeedback;
    QString mKeyboardSize;
    QString mKeyboardLayout;
    QJsonObject mSavedKeyboardPrefs;
};

} // namespace

#endif // KEYBOARDSETTINGS_H
