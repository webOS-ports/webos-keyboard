
#include "inputmethod.h"

#include "editor.h"
#include "hardwarekeyboard.h"
#include "keyboardgeometry.h"
#include "keyboardsettings.h"
#include "panelstate.h"
#include "t9multitap.h"

#include "logic/layoutupdater.h"
#include "logic/eventhandler.h"
#include "logic/wordengine.h"

#include "luneosapplicationapiwrapper.h"
#include "pre/precontroller.h"


#include <maliit/plugins/abstractinputmethodhost.h>
#include <maliit/plugins/abstractpluginsetting.h>

#include <QtQuick>
#include <QStringList>
#include <QDir>
#include <QTimer>
#include <qglobal.h>
#include <QDebug>

using namespace MaliitKeyboard;

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;
typedef QSharedPointer<MKeyOverride> SharedOverride;
typedef QMap<QString, SharedOverride>::const_iterator OverridesIterator;

static QQuickView *createWindow(MAbstractInputMethodHost *host)
{
    QScopedPointer<QQuickView> view(new QQuickView);

    QSurfaceFormat format = view->format();
    format.setAlphaBufferSize(8);
    view->setFormat(format);
    view->setColor(QColor(Qt::transparent));

    host->registerWindow(view.data(), Maliit::PositionCenterBottom);

    return view.take();
}

class InputMethodPrivate
{
    Q_DISABLE_COPY(InputMethodPrivate)

public:
    InputMethod* q;
    Editor editor;
    QMap<QString, SharedOverride> key_overrides;
    Logic::EventHandler event_handler;
    MAbstractInputMethodHost* host;
    QQuickView* view;
    LuneOSApplicationApiWrapper* applicationApiWrapper;

    bool autocapsEnabled;
    InputMethod::TextContentType contentType;
    QString activeLanguage;
    //! Painted on the space bar, and the label the application asked for on Return.
    QString primaryCandidate;
    QString actionKeyLabel;
	QString keyboardSize;
    QString keyboardLayout;
    QStringList enabledLanguages;
    Qt::ScreenOrientation appsCurrentOrientation;

    KeyboardGeometry *m_geometry;
    //! True while applyPanelVisibility() is inside setVisible(true).
    bool showingPanel = false;
    //! Set when the window was hidden from under that call; see the connection
    //! to visibleChanged in the constructor.
    bool panelForcedHidden = false;
    KeyboardSettings m_settings;
    //! The Pre Keyboard: the phone keyboard (Keyboard.qml sets it active on a phone).
    Pre::Controller pre;
    //! Resolves the Alt and Sym levels of a physical keyboard, if this device
    //! has one we have a profile for. Inert otherwise.
    HardwareKeyboard hardwareKeyboard;

    //! True while Maliit has this plugin serving Maliit::Hardware, i.e. while
    //! the framework has found a usable physical keyboard. The on-screen
    //! keyboard stays down for as long as it is set; see InputMethod::show().
    //!
    //! Whether a field has focus, whether a physical keyboard is the input source,
    //! whether the word engine has anything to offer, and what the user has
    //! dragged away - and the rules relating them. All of it lives in PanelState
    //! rather than as members here so that the transitions can be driven by a unit
    //! test: every bug in this area has been a transition, and reproducing one
    //! otherwise needs a plugin, a QQuickView and a compositor. See
    //! tests/ut_panelstate.
    PanelState panel;

    //! The scancode currently held down, so a press for a key that is already
    //! down can be recognised as the kernel repeating it. Maliit cannot say:
    //! the connection passes autoRepeat as a hardcoded false, and the compositor
    //! clears the flag before that to get the event delivered at all.
    quint32 heldScanCode = 0;
    //! The letter as typed, when the last one inserted from a physical keyboard
    //! was capitalised by auto-caps alone. A backspace straight after takes the
    //! capital back and puts this in its place, as legacy webOS did
    //! (EditorClient::handleKeyboardEvent, undidPreviousAutocap). Empty otherwise.
    QString autocapUndoLetter;

    WordRibbon* wordRibbon;

    //! Where the application's cursor sat when the preedit we are holding
    //! started, or -1 when we are not tracking one. See InputMethod::update().
    int preeditCursorAnchor;
    //! The last word the space key autocorrected: what was typed, what replaced it,
    //! and the end of the text before it. A tap on that word offers the typed one
    //! back, as legacy's spelling widget did for a word it had replaced.
    QString autoCorrectedOriginal;
    QString autoCorrectedWord;
    QString autoCorrectedLeft;
    //! The last word updateSpellingSuggestions() logged, to log each word once.
    QString lastLoggedSpellingWord;

    //! Hardware T9 multi-tap. A physical numeric keypad sends KEY_0..KEY_9,
    //! and in a text field those cycle through letters (2 -> a/b/c/2) until
    //! t9Timer fires. t9 is the cycle; t9Commits says where its character
    //! lives: committed straight away and replaced in place, in a field that
    //! hides what is typed, or in the preedit until it is done, elsewhere.
    T9Multitap t9;
    bool t9Commits;
    //! What was in the preedit, typed on the screen keys, when the character
    //! being cycled was begun: the character goes on the end of it.
    QString t9Before;
    QTimer *t9Timer;
    //! The focused field hides what is typed in it (a password), from the
    //! last update().
    bool hiddenText;

    explicit InputMethodPrivate(InputMethod * const _q,
                                MAbstractInputMethodHost *host)
        : q(_q)
        , editor(EditorOptions(), new Model::Text, new Logic::WordEngine)
        , key_overrides()
        , event_handler()
        , host(host)
        , view(nullptr)
        , applicationApiWrapper(new LuneOSApplicationApiWrapper)
        , autocapsEnabled(false)
        , contentType(InputMethod::FreeTextContentType)
        , activeLanguage("en")
        , primaryCandidate()
        , actionKeyLabel()
		, keyboardSize("M")
        , keyboardLayout("LuneOS")
        , enabledLanguages(activeLanguage)
        , appsCurrentOrientation(qGuiApp->primaryScreen()->orientation())
        , m_geometry(new KeyboardGeometry(q))
        , m_settings()
        , hardwareKeyboard()
        , wordRibbon(new WordRibbon)
        , preeditCursorAnchor(-1)
        , t9()
        , t9Commits(false)
        , t9Timer(nullptr)
        , hiddenText(false)
    {
        applicationApiWrapper->setGeometryItem(m_geometry);

        view = createWindow(host);

        /*
         * The geometry follows the window, including when the window was hidden
         * by someone else.
         *
         * maliit's WindowGroup force-hides the window of a plugin whose group it
         * has not activated yet, and it does so behind this plugin's back. What
         * that used to leave behind was a keyboard that never came back: shown
         * is change-gated, so a geometry still saying "shown" while the window
         * is hidden can never be set to shown again, Keyboard.qml's
         * "maliit_geometry.shown === true" transition never re-runs, the panel
         * stays parked at no height, no buffer is ever drawn, and the compositor
         * is left holding a keyboard surface that was created and never mapped.
         * closeOskWindow() could not undo it either - it returns early on a
         * window that is already hidden.
         *
         * Mirroring the real state here means the next applyPanelVisibility()
         * is a fresh false -> true transition and the panel draws.
         */
        QObject::connect(view, &QWindow::visibleChanged,
                         m_geometry, [this](bool visible) {
                             if (visible) {
                                 return;
                             }

                             m_geometry->setShown(false);

                             // Hidden while we were in the middle of showing it
                             // is not something the plugin did: it is the
                             // framework's force-hide, arriving from inside our
                             // own setVisible(). Worth remembering, because it
                             // is the one case that needs the window taken down
                             // and put up again, and it is the only way to tell
                             // it apart from a window that is simply not exposed
                             // yet - which is every window, for a moment, and on
                             // a loaded device for longer.
                             if (showingPanel) {
                                 panelForcedHidden = true;
                             }
                         });

        editor.setHost(host);

        //! connect wordRibbon
        QObject::connect(&event_handler, SIGNAL(wordCandidatePressed(WordCandidate)),
                         wordRibbon, SLOT( onWordCandidatePressed(WordCandidate) ));

        QObject::connect(&event_handler, SIGNAL(wordCandidateReleased(WordCandidate)),
                         wordRibbon, SLOT( onWordCandidateReleased(WordCandidate) ));

        QObject::connect(&editor,  SIGNAL(wordCandidatesChanged(WordCandidateList)),
                         wordRibbon, SLOT(onWordCandidatesChanged(WordCandidateList)));

        //! the space bar shows whatever space would commit
        QObject::connect(&editor,  SIGNAL(wordCandidatesChanged(WordCandidateList)),
                         _q,       SLOT(onWordCandidatesChanged()));

        QObject::connect(wordRibbon, SIGNAL(wordCandidateSelected(QString)),
                         &editor,  SLOT(replaceAndCommitPreedit(QString)));

        QObject::connect(wordRibbon, SIGNAL(userCandidateSelected(QString)),
                         &editor,  SLOT(addToUserDictionary(QString)));

        QObject::connect(&editor,  SIGNAL(preeditEnabledChanged(bool)),
                         wordRibbon, SLOT(setWordRibbonVisible(bool)));

        QObject::connect(wordRibbon, SIGNAL(wordCandidateSelected(QString)),
                         editor.wordEngine(),  SLOT(onWordCandidateSelected(QString)));

    #ifdef DISABLED_FLAGS_FROM_SURFACE
        view->setFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                          | Qt::X11BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    #endif
        view->setWindowState(Qt::WindowNoState);

        QSurfaceFormat format = view->format();
        format.setAlphaBufferSize(8);
        view->setFormat(format);
        view->setColor(QColor(Qt::transparent));

        view->setVisible(false);

        // TODO: Figure out whether two views can share one engine.
        QQmlEngine *const engine(view->engine());
        engine->addImportPath(LUNEOS_KEYBOARD_DATA_DIR);
        setContextProperties(engine->rootContext());

        // following used to help shell identify the OSK surface
        view->setProperty("role", applicationApiWrapper->oskWindowRole());
        view->setTitle("MaliitOnScreenKeyboard");

        // workaround: resizeMode not working in current qpa imlementation
        // http://qt-project.org/doc/qt-5.0/qtquick/qquickview.html#ResizeMode-enum
        view->setResizeMode(QQuickView::SizeRootObjectToView);
    }

    ~InputMethodPrivate()
    {
        delete applicationApiWrapper;
    }

    static Logic::LayoutHelper::Orientation screenToMaliitOrientation(Qt::ScreenOrientation screenOrientation)
    {
        switch (screenOrientation) {
        case Qt::LandscapeOrientation:
        case Qt::InvertedLandscapeOrientation:
            return Logic::LayoutHelper::Landscape;
            break;
        case Qt::PortraitOrientation:
        case Qt::InvertedPortraitOrientation:
        case Qt::PrimaryOrientation:
        default:
            return Logic::LayoutHelper::Portrait;
        }

        return Logic::LayoutHelper::Portrait;
    }

    void setLayoutOrientation(Qt::ScreenOrientation screenOrientation)
    {
        m_geometry->setOrientation(screenOrientation);
    }

    void setContextProperties(QQmlContext *qml_context)
    {
        qml_context->setContextProperty("maliit_input_method", q);
        qml_context->setContextProperty("maliit_geometry", m_geometry);
        qml_context->setContextProperty("maliit_event_handler", &event_handler);
        qml_context->setContextProperty("maliit_wordribbon", wordRibbon);
        qml_context->setContextProperty("maliit_word_engine", editor.wordEngine());
        qml_context->setContextProperty("maliit_hw_keyboard", &hardwareKeyboard);
        qml_context->setContextProperty("pre_keyboard", &pre);
    }

    /*!
     * The Pre Keyboard's connections. Its keys go through the event handler and
     * the editor as LuneOS's own keys do, so its words - the bar, autocorrect,
     * auto-capitals, the owner's dictionary - are the word engine's, and its
     * languages are the keyboard's.
     */
    void setUpPreKeyboard()
    {
        pre.setDataDir(QStringLiteral(LUNEOS_KEYBOARD_DATA_DIR "/pre"));
        pre.setStateDir(QDir::homePath() + QStringLiteral("/prekeyboard"));
        // Swiped words come from what the word engine reads: presage's databases and hunspell's dictionaries.
        pre.setWordDirs(QStringLiteral(MALIIT_PLUGINS_DATA_DIR "/org/luneos/lib"), QStringLiteral(HUNSPELL_DICT_PATH));

        Pre::Bridge b;
        b.press = [this](const QString &label, const QString &action) { event_handler.onKeyPressed(label, action); };
        b.release = [this](const QString &label, const QString &action) { event_handler.onKeyReleased(label, action); };
        b.cancelRepeat = [this] { editor.cancelRepeat(); };
        b.pick = [this](const QString &word) { event_handler.onWordCandidateReleased(word); };
        b.commit = [this](const QString &text) { q->commitEmoji(text); };
        b.setWord = [this](const QString &word) { editor.replacePreedit(word); };
        b.atWordEnd = [this] {
            const Model::Text *t = editor.text();
            if (!t || !t->preedit().isEmpty()) return false;
            const QString left = t->surroundingLeft();
            return !left.isEmpty() && left.at(left.size() - 1).isLetterOrNumber();
        };
        b.cursor = [this](int dx, int dy, bool select) {
            if (!select) {
                const QString action = dx > 0 ? QStringLiteral("keyRight") : dx < 0 ? QStringLiteral("keyLeft")
                                     : dy > 0 ? QStringLiteral("keyDown") : QStringLiteral("keyUp");
                event_handler.onKeyPressed(QString(), action);
                event_handler.onKeyReleased(QString(), action);
                return;
            }
            // The editor's arrows don't select: the word being typed goes in
            // first, then the application gets shift and the arrow.
            editor.commit();
            const int key = dx > 0 ? Qt::Key_Right : dx < 0 ? Qt::Key_Left : dy > 0 ? Qt::Key_Down : Qt::Key_Up;
            host->sendKeyEvent(QKeyEvent(QEvent::KeyPress, key, Qt::ShiftModifier));
            host->sendKeyEvent(QKeyEvent(QEvent::KeyRelease, key, Qt::ShiftModifier));
        };
        b.autoCaps = [this] { return editor.atAutoCapsPosition(); };
        b.before = [this](int n) {
            const Model::Text *t = editor.text();
            return t ? (t->surroundingLeft() + t->preedit()).right(n) : QString();
        };
        b.after = [this](int n) {
            const Model::Text *t = editor.text();
            return t ? t->surroundingRight().left(n) : QString();
        };
        b.prose = [this] { return contentType == InputMethod::FreeTextContentType && !hiddenText; };
        b.nextWords = [this] {
            const Model::Text *t = editor.text();
            return t ? editor.wordEngine()->nextWords(t->surroundingLeft(), 3) : QStringList();
        };
        b.replace = [this](int before, int after, const QString &text) {
            host->sendCommitString(text, -before, before + after);
        };
        b.position = [this] {
            const Model::Text *t = editor.text();
            return t ? int(t->surroundingOffset()) : -1;
        };
        b.alternatives = [this](const QString &word, const QString &left) {
            QStringList out;
            // The word the space key put in place of another, still where it was put: that one first.
            if (!autoCorrectedWord.isEmpty() && word == autoCorrectedWord && left.endsWith(autoCorrectedLeft))
                out << autoCorrectedOriginal;
            out << editor.wordEngine()->spellingSuggestions(word, 4);
            return out;
        };
        b.canAdd = [this](const QString &word) { return editor.wordEngine()->isMisspelled(word); };
        b.addWord = [this](const QString &word) { editor.addToUserDictionary(word); };
        b.removeWord = [this](const QString &word) {
            editor.wordEngine()->removeFromUserDictionary(word);
            blockRemovedWords();
            editor.refreshCandidates();
        };
        b.useLanguage = [this](const QString &id) { q->setActiveLanguage(id); };
        b.luna = [this](const QString &uri, const QString &payload) { m_settings.call(uri, payload); };
        b.savePrefs = [this](const QJsonObject &obj) { m_settings.savePre(obj); };
        b.hide = [this] { host->notifyImInitiatedHiding(); };
        pre.setBridge(b);
        QObject::connect(&editor, &AbstractTextEditor::autoCorrected, &pre,
                         [this](const QString &typed, const QString &fixed) { pre.corrected(typed, fixed); });

        QObject::connect(&m_settings, &KeyboardSettings::preChanged, &pre, &Pre::Controller::setPrefs);
        pre.setPrefs(m_settings.pre());
        // The words deleted from the Pre Keyboard's bar (or put back in Text
        // Assist), for the language typed in: the word engine neither offers
        // nor corrects to them, whichever keys are typed on.
        // Asked again after: the bar may hold one just deleted, or the old
        // language's list (refreshCandidates() is connected before this).
        const auto reblock = [this] { blockRemovedWords(); editor.refreshCandidates(); };
        QObject::connect(&m_settings, &KeyboardSettings::preChanged, q, reblock);
        QObject::connect(q, &InputMethod::activeLanguageChanged, q, reblock);
        blockRemovedWords();

        // The bar: the word engine's candidates, and what space would put in.
        QObject::connect(&editor, &AbstractTextEditor::wordCandidatesChanged, &pre,
                         [this](const WordCandidateList &candidates) {
            QStringList words;
            for (const WordCandidate &c : candidates)
                if (c.source() != WordCandidate::SourceUser) words << c.word();
            Model::Text *t = editor.text();
            const QString primary = editor.isAutoCorrectEnabled() && t ? t->primaryCandidate() : QString();
            pre.setCandidates(t ? t->preedit() : QString(), primary, words);
        });
        // Words for a field that takes them: the editor's preedit is on for one.
        QObject::connect(&editor, &AbstractTextEditor::preeditEnabledChanged, &pre, &Pre::Controller::setWordsAllowed);
        pre.setWordsAllowed(editor.isPreeditEnabled());
        QObject::connect(&editor, &AbstractTextEditor::autoCapsActivated, &pre, &Pre::Controller::update);

        // The settings it follows: suggestions on, the key click, the languages.
        const auto sync = [this] {
            pre.setKeyboardState(editor.wordEngine()->isEnabled(), q->useAudioFeedback(),
                                 q->enabledLanguages(), q->activeLanguage());
        };
        QObject::connect(editor.wordEngine(), &Logic::AbstractWordEngine::enabledChanged, &pre, sync);
        QObject::connect(q, &InputMethod::useAudioFeedbackChanged, &pre, sync);
        QObject::connect(q, &InputMethod::enabledLanguagesChanged, &pre, sync);
        QObject::connect(q, &InputMethod::activeLanguageChanged, &pre, sync);
        sync();
        // Its height is the keyboard's size, Text Assist's Keyboard Size.
        QObject::connect(q, &InputMethod::keyboardSizeChanged, &pre, &Pre::Controller::setKeyboardSize);
        pre.setKeyboardSize(q->keyboardSize());
    }


    void blockRemovedWords()
    {
        editor.wordEngine()->setBlockedWords(Pre::Settings::removed(m_settings.pre(), q->activeLanguage()));
    }

    /*
     * register settings
     */
    void registerFeedbackSetting()
    {
        QObject::connect(&m_settings, SIGNAL(keyPressFeedbackChanged(bool)),
                         q, SIGNAL(useAudioFeedbackChanged()));
    }

    void registerAutoCorrectSetting()
    {
        QObject::connect(&m_settings, SIGNAL(autoCorrectionChanged(bool)),
                         q, SLOT(onAutoCorrectSettingChanged()));
        editor.setAutoCorrectEnabled(m_settings.autoCorrection());
    }

    void registerAutoCapsSetting()
    {
        QObject::connect(&m_settings, SIGNAL(autoCapitalizationChanged(bool)),
                         q, SLOT(updateAutoCaps()));
    }

    void registerWordEngineSetting()
    {
        QObject::connect(&m_settings, SIGNAL(predictiveTextChanged(bool)),
                         editor.wordEngine(), SLOT(setWordPredictionEnabled(bool)));
        editor.wordEngine()->setWordPredictionEnabled(m_settings.predictiveText());

        QObject::connect(&m_settings, SIGNAL(spellCheckingChanged(bool)),
                         editor.wordEngine(), SLOT(setSpellcheckerEnabled(bool)));
        editor.wordEngine()->setSpellcheckerEnabled(m_settings.spellchecking());
    }

    void registerActiveLanguage()
    {
        QObject::connect(&m_settings, SIGNAL(activeLanguageChanged(QString)),
                         q, SLOT(setActiveLanguage(QString)));

        activeLanguage = m_settings.activeLanguage();
        qDebug() << "inputmethod_p.h registerActiveLanguage(): activeLanguage is:" << activeLanguage;
        q->setActiveLanguage(activeLanguage);
    }
	
    void registerKeyboardSize()
    {
        QObject::connect(&m_settings, SIGNAL(keyboardSizeChanged(QString)),
                         q, SLOT(setKeyboardSize(QString)));

        keyboardSize = m_settings.keyboardSize();
        qDebug() << "inputmethod_p.h registerKeyboardSize(): keyboardSize is:" << keyboardSize;
        q->setKeyboardSize(keyboardSize);
    }

    void registerKeyboardLayout()
    {
        QObject::connect(&m_settings, SIGNAL(keyboardLayoutChanged(QString)),
                         q, SLOT(setKeyboardLayout(QString)));

        keyboardLayout = m_settings.keyboardLayout();
        qDebug() << "inputmethod_p.h registerKeyboardLayout(): keyboardLayout is:" << keyboardLayout;
        q->setKeyboardLayout(keyboardLayout);
    }

    void registerEnabledLanguages()
    {
        QObject::connect(&m_settings, SIGNAL(enabledLanguagesChanged(QStringList)),
                         q, SLOT(onEnabledLanguageSettingsChanged()));
        q->onEnabledLanguageSettingsChanged();

        //registerSystemLanguage();
        //q->setActiveLanguage(activeLanguage);
    }
    void closeOskWindow()
    {
        if (!view->isVisible())
            return;

        host->notifyImInitiatedHiding();

        m_geometry->setShown(false);

        editor.clearPreedit();
        // A key held as the panel goes is never released: the touch is
        // cancelled, not lifted. Its repeat would otherwise go on deleting
        // from whatever field comes next, with nothing on screen to stop it.
        editor.cancelRepeat();

        view->setVisible(false);

        applicationApiWrapper->reportOSKInvisible();

        m_settings.savePreferences(q);
    }

    //! Drops the preedit we hold, and with it the cursor position we were
    //! tracking it against. For the points where the application has already
    //! discarded its own preedit and is only telling us afterwards.
    void dropPreedit()
    {
        editor.resetPreedit();
        preeditCursorAnchor = -1;
        // The character T9 was cycling lived in that preedit. Forget it too,
        // or the next tap of the same key would resume the cycle and put the
        // *next* letter wherever the cursor has since gone. Deliberately does
        // not commit: every caller is a point where the text we were holding
        // is being abandoned, not accepted.
        resetT9();
    }

    //! Abandons any half-cycled T9 character. Touches no editor state, so it
    //! is safe to call from inside an editor operation.
    void resetT9()
    {
        if (t9Timer)
            t9Timer->stop();
        t9.end();
        t9Commits = false;
        t9Before.clear();
    }

    void truncateEnabledLanguageLocales(const QStringList& locales)
    {
        enabledLanguages.clear();
        foreach (QString locale, locales) {
            locale.truncate(2);
            enabledLanguages << locale;
        }
    }
};
