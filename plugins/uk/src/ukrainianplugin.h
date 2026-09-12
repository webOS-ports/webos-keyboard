#ifndef UKRAINIANPLUGIN_H
#define UKRAINIANPLUGIN_H

#include <QObject>
#include "languageplugininterface.h"
#include "westernlanguagesplugin.h"

class UkrainianPlugin : public QObject, public WesternLanguagesPlugin
{
    Q_OBJECT
    Q_INTERFACES(LanguagePluginInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.UkrainianPlugin" FILE "ukrainianplugin.json")

public:
    explicit UkrainianPlugin(QObject* parent = 0)
        : QObject(parent)
        , WesternLanguagesPlugin()
    {
        _useDatabase("uk");
    }

    virtual ~UkrainianPlugin()
    {
    }
};

#endif // UKRAINIANPLUGIN_H
