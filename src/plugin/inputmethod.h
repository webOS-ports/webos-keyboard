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

#ifndef MALIIT_KEYBOARD_INPUTMETHOD_H
#define MALIIT_KEYBOARD_INPUTMETHOD_H

#include <maliit/plugins/abstractinputmethod.h>
#include <maliit/plugins/abstractinputmethodhost.h>
#include <maliit/plugins/keyoverride.h>

#include <QtGui>
#include <QtQuick/QQuickView>

class InputMethodPrivate;

class InputMethod
    : public MAbstractInputMethod
{
    Q_OBJECT
    Q_DISABLE_COPY(InputMethod)
    Q_DECLARE_PRIVATE(InputMethod)

    Q_PROPERTY(TextContentType contentType READ contentType WRITE setContentType NOTIFY contentTypeChanged)
    Q_PROPERTY(QStringList enabledLanguages READ enabledLanguages NOTIFY enabledLanguagesChanged)
    Q_PROPERTY(QString activeLanguage READ activeLanguage WRITE setActiveLanguage NOTIFY activeLanguageChanged)
    Q_PROPERTY(QString keyboardSize READ keyboardSize WRITE setKeyboardSize NOTIFY keyboardSizeChanged)
    Q_PROPERTY(QString keyboardLayout READ keyboardLayout WRITE setKeyboardLayout NOTIFY keyboardLayoutChanged)
    Q_PROPERTY(bool useAudioFeedback READ useAudioFeedback NOTIFY useAudioFeedbackChanged)
    //! The word that pressing space would commit. drawKeyCap paints this on the
    //! space bar: "if (key == Qt::Key_Space) text = m_candidateBar.autoSelectCandidate()".
    Q_PROPERTY(QString primaryCandidate READ primaryCandidate NOTIFY primaryCandidateChanged)
    //! Label the application asked for on the Return key, through Maliit's
    //! "actionKey" override - the equivalent of the reference's
    //! PalmIME::EditorState::enterKeyLabel. Empty means plain "Enter".
    Q_PROPERTY(QString actionKeyLabel READ actionKeyLabel NOTIFY actionKeyLabelChanged)
    //! True while a physical keyboard is the active input source. The QML
    //! collapses the keys away when it is set, leaving the candidate bar.
    Q_PROPERTY(bool hardwareKeyboardActive READ hardwareKeyboardActive NOTIFY hardwareKeyboardActiveChanged)
    //! True when the keys are not drawn and the panel is only the candidate bar,
    //! whether because a physical keyboard is in use or because the user dragged
    //! the keys away. The QML collapses them on this.
    Q_PROPERTY(bool keysCollapsed READ keysCollapsed NOTIFY keysCollapsedChanged)

public:
    /// Same as Maliit::TextContentType but usable in QML
    enum TextContentType {
        FreeTextContentType = Maliit::FreeTextContentType,
        NumberContentType = Maliit::NumberContentType,
        PhoneNumberContentType = Maliit::PhoneNumberContentType,
        EmailContentType = Maliit::EmailContentType,
        UrlContentType = Maliit::UrlContentType,
        CustomContentType = Maliit::CustomContentType
    };
    Q_ENUM(TextContentType)

    explicit InputMethod(MAbstractInputMethodHost *host);
    ~InputMethod() override;

    //! \reimp
    void show() override;
    Q_SLOT void hide() override;
    void reset() override;
    void setPreedit(const QString &preedit,
                            int cursor_position) override;
    void processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                 Qt::KeyboardModifiers modifiers,
                                 const QString &text, bool autoRepeat, int count,
                                 quint32 nativeScanCode, quint32 nativeModifiers,
                                 unsigned long time) override;
    void switchContext(Maliit::SwitchDirection direction,
                               bool animated) override;
    void setState(const QSet<Maliit::HandlerState> &state) override;

    bool hardwareKeyboardActive() const;
    bool keysCollapsed() const;

    /*! \brief Drags the keys away, keeping the candidate bar.
     *
     * Called by the swipe-down gesture. Where there is no candidate bar to be
     * left with - the word engine is off - the panel goes altogether, because a
     * zero-height panel comes back full height and blank.
     */
    /*!
     * \brief Puts an emoji in as text that is already finished.
     *
     * Not through the character path the keys use: that builds a preedit, which
     * is how a word being typed can still be corrected, and an emoji is not a
     * word being typed. Left in the preedit it drew with the composition
     * highlight under it - a lilac band behind every emoji inserted - and stayed
     * uncommitted until something else happened to commit it.
     */
    Q_INVOKABLE void commitEmoji(const QString &emoji);

    Q_INVOKABLE void collapseKeys();
    Q_INVOKABLE void expandKeys();
    QList<MAbstractInputMethod::MInputMethodSubView>
    subViews(Maliit::HandlerState state = Maliit::OnScreen) const override;
    void setActiveSubView(const QString &id,
                                  Maliit::HandlerState state = Maliit::OnScreen) override;
    QString activeSubView(Maliit::HandlerState state = Maliit::OnScreen) const override;
    void handleFocusChange(bool focusIn) override;
    void handleAppOrientationChanged(int angle) override;
    void handleClientChange() override;
    bool imExtensionEvent(MImExtensionEvent *event) override;
    void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides) override;
    //! \reimp_end

    Q_SLOT void deviceOrientationChanged(Qt::ScreenOrientation orientation);

    Q_SLOT void updateWordEngine();

    TextContentType contentType();
    Q_SLOT void setContentType(TextContentType contentType);

    /*! \brief The characters a field of this kind will take off a physical
     *         keyboard's Alt level without the user holding Alt.
     *
     * Empty for anything that can hold prose, which is most fields and all the
     * ones where a letter has to stay a letter. See
     * HardwareKeyboard::setDigitsPreferred().
     */
    static QString digitsForContentType(TextContentType contentType);

    //! Hardware T9 multi-tap. t9HandleKey runs a physical numeric keypad
    //! through the multi-tap state machine and returns true when it has
    //! consumed the event; finalizeT9 (also the inactivity-timer target)
    //! fixes the character being cycled.
    bool t9HandleKey(QEvent::Type keyType, Qt::Key keyCode, bool autoRepeat);
    Q_SLOT void finalizeT9();

    //! Writes "a text field has focus" where a process outside the compositor
    //! can read it; see the implementation for who wants it and why.
    void publishTextFocus(bool focusIn);

    void update() override;

    const QStringList &enabledLanguages() const;

    const QString &activeLanguage() const;
    QString primaryCandidate() const;
    QString actionKeyLabel() const;
    Q_SLOT void setActiveLanguage(const QString& newLanguage);

    const QString &keyboardSize() const;	
    Q_SLOT void setKeyboardSize(const QString& newKeyboardSize);

    const QString &keyboardLayout() const;
    Q_SLOT void setKeyboardLayout(const QString& newKeyboardLayout);

    Q_SLOT void updateWindowMask();
    Q_SLOT void onVisibleRectChanged();
    Q_SLOT void onWordCandidatesChanged();
    bool useAudioFeedback() const;

Q_SIGNALS:
    void contentTypeChanged(TextContentType contentType);
    void activateAutocaps();
    void enabledLanguagesChanged(QStringList languages);
    void activeLanguageChanged(QString language);
    void useAudioFeedbackChanged();
    void primaryCandidateChanged(const QString &primaryCandidate);
    void actionKeyLabelChanged(const QString &actionKeyLabel);
    void wordEngineEnabledChanged(bool wordEngineEnabled);
    void wordRibbonEnabledChanged(bool wordRibbonEnabled);
    void windowGeometryRectChanged(QRect rect);
    void keyboardSizeChanged(QString size);
    void keyboardLayoutChanged(QString layout);
    void hardwareKeyboardActiveChanged();

    /*! \brief A key asked for the emoji panel.
     *
     * The QML opens it, because that is where the panel lives. Emitted rather
     * than a property set, because asking twice means asking twice - the key is
     * a request, not a state.
     */
    void emojiPanelRequested();
    void keysCollapsedChanged();

private:
    //! How many suggestions the shell is given for a misspelled word.
    static constexpr int kMaxSpellingSuggestions = 4;

    void updateSpellingSuggestions(const QString &text, int position);

    Q_SLOT void onAutoCorrectSettingChanged();
    Q_SLOT void onEnabledLanguageSettingsChanged();
    Q_SLOT void updateAutoCaps();

    Q_SLOT void updateKey(const QString &key_id,
                          const MKeyOverride::KeyOverrideAttributes changed_attributes);
    Q_SLOT void onKeyboardClosed();
    Q_SLOT void onHardwareProfileChanged();

    Q_SLOT void onLayoutWidthChanged(int width);
    Q_SLOT void onLayoutHeightChanged(int height);

    void checkInitialAutocaps();

    //! \brief Puts the panel on screen, or takes it off, from what is wanted now.
    void applyPanelVisibility();

    //! \brief Hands a dismissal to the framework when the keys are only up
    //!        because they were forced there, and says whether it did.
    bool releaseForcedOnScreenKeyboard();

    //! \brief Tells the application how much of the screen the panel is using,
    //!        and follows the panel with the window mask.
    void reportPanelArea();

    //! \brief Tells the application the area, without touching the mask.
    void announcePanelArea();

    //! \brief Masks the window to the strip the panel occupies, anchored to the
    //!        bottom of the view so it is right before the panel animates in.
    void maskPanelStrip();

    const QScopedPointer<InputMethodPrivate> d_ptr;
};

#endif // MALIIT_KEYBOARD_INPUTMETHOD_H
