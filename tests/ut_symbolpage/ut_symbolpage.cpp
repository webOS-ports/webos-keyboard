/*
 * This file is part of the LuneOS keyboard
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 */

/*
 * The keyboard's QML reads the objects the Maliit plugin puts on the engine's
 * root context. tests/keyboard-test/Stubs.qml fakes them for the desktop
 * preview; this hands the same stand-ins to the Quick Test engine before any
 * test file is loaded.
 */

#include <QtQuickTest/quicktest.h>

#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

class Setup : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        const QString stubDir = QStringLiteral(KEYBOARD_TEST_DIR);
        const QString qmlDir = QStringLiteral(KEYBOARD_QML_DIR);

        engine->addImportPath(stubDir);
        engine->addImportPath(qmlDir);

        QQmlComponent component(engine, QUrl::fromLocalFile(stubDir + QStringLiteral("/Stubs.qml")));
        QObject *const stubs = component.create();
        if (!stubs)
            qFatal("Stubs.qml failed to load: %s", qPrintable(component.errorString()));
        stubs->setParent(engine);

        static const char *const names[] = {
            "maliit_input_method",
            "maliit_geometry",
            "maliit_event_handler",
            "maliit_word_engine",
            "maliit_wordribbon",
            "audioFeedback",
        };
        for (const char *name : names)
            engine->rootContext()->setContextProperty(QLatin1String(name), stubs->property(name));
        engine->rootContext()->setContextProperty(QStringLiteral("stubs"), stubs);
        engine->rootContext()->setContextProperty(QStringLiteral("keyboardQmlUrl"),
                                                  QUrl::fromLocalFile(qmlDir + QStringLiteral("/Keyboard.qml")));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(ut_symbolpage, Setup)

#include "ut_symbolpage.moc"
