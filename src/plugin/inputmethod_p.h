
#include "inputmethod.h"

#include "editor.h"
#include "hardwarekeyboard.h"
#include "keyboardgeometry.h"
#include "keyboardsettings.h"

#include "logic/layoutupdater.h"
#include "logic/eventhandler.h"
#include "logic/wordengine.h"

#include "luneosapplicationapiwrapper.h"

#include <maliit/plugins/abstractinputmethodhost.h>
#include <maliit/plugins/abstractpluginsetting.h>

#include <QtQuick>
#include <QStringList>
#include <QTimer>
#include <QElapsedTimer>
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
    bool wordEngineEnabled;
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
    KeyboardSettings m_settings;
    //! Resolves the Alt and Sym levels of a physical keyboard, if this device
    //! has one we have a profile for. Inert otherwise.
    HardwareKeyboard hardwareKeyboard;

    WordRibbon* wordRibbon;

    //! Where the application's cursor sat when the preedit we are holding
    //! started, or -1 when we are not tracking one. See InputMethod::update().
    int preeditCursorAnchor;

    //! The caret position we last saw the application report, so update() can
    //! tell a caret move from the many other reasons it is called. -1 until we
    //! have seen one.
    int lastKnownCursorPosition;

    //! Hardware T9 multi-tap. A physical numeric keypad sends KEY_0..KEY_9,
    //! and in a text field those cycle through letters (2 -> a/b/c/2) in the
    //! preedit until t9Timer fires. t9Key is the key being cycled (0 = none),
    //! t9Index the position in its cycle.
    Qt::Key t9Key;
    int t9Index;
    QTimer *t9Timer;
    //! De-bounce: the compositor emits several KeyPress events for one
    //! physical keypad tap, so collapse those into a single press per tap.
    Qt::Key t9BurstKey;
    QElapsedTimer t9BurstTimer;

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
        , wordEngineEnabled(false)
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
        , lastKnownCursorPosition(-1)
        , t9Key(Qt::Key(0))
        , t9Index(0)
        , t9Timer(nullptr)
        , t9BurstKey(Qt::Key(0))
    {
        applicationApiWrapper->setGeometryItem(m_geometry);

        view = createWindow(host);

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
    //! is safe to call from inside an editor operation. Deliberately leaves
    //! the burst de-bounce alone: that collapses duplicate events from one
    //! physical tap and is independent of what the text is doing, so clearing
    //! it here would let the tail of a tap through as a second character.
    void resetT9()
    {
        if (t9Timer)
            t9Timer->stop();
        t9Key = Qt::Key(0);
        t9Index = 0;
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
