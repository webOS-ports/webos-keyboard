
#include "inputmethod.h"

#include "editor.h"
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
#include <qglobal.h>
#include <QDebug>

using namespace MaliitKeyboard;

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;
typedef QSharedPointer<MKeyOverride> SharedOverride;
typedef QMap<QString, SharedOverride>::const_iterator OverridesIterator;

QQuickView *createWindow(MAbstractInputMethodHost *host)
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
	QString keyboardSize;
    QString keyboardLayout;
    QStringList enabledLanguages;
    Qt::ScreenOrientation appsCurrentOrientation;

    KeyboardGeometry *m_geometry;
    KeyboardSettings m_settings;

    WordRibbon* wordRibbon;

    explicit InputMethodPrivate(InputMethod * const _q,
                                MAbstractInputMethodHost *host)
        : q(_q)
        , editor(EditorOptions(), new Model::Text, new Logic::WordEngine)
        , key_overrides()
        , event_handler()
        , host(host)
        , view(0)
        , applicationApiWrapper(new LuneOSApplicationApiWrapper)
        , autocapsEnabled(false)
        , wordEngineEnabled(false)
        , contentType(InputMethod::FreeTextContentType)
        , activeLanguage("en")
		, keyboardSize("M")
        , keyboardLayout("LuneOS")
        , enabledLanguages(activeLanguage)
        , appsCurrentOrientation(qGuiApp->primaryScreen()->orientation())
        , m_geometry(new KeyboardGeometry(q))
        , m_settings()
        , wordRibbon(new WordRibbon)
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

    Logic::LayoutHelper::Orientation screenToMaliitOrientation(Qt::ScreenOrientation screenOrientation) const
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

    void truncateEnabledLanguageLocales(QStringList locales)
    {
        enabledLanguages.clear();
        foreach (QString locale, locales) {
            locale.truncate(2);
            enabledLanguages << locale;
        }
    }
};
