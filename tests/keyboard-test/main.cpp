/*
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 * Desktop preview for the keyboard layouts.
 *
 * All the faking lives in Stubs.qml; this only promotes those objects to context
 * properties on the engine's root context, which is where the Maliit plugin puts
 * the real ones. That detail is the whole reason a plain .qmlproject run cannot
 * work: UI.qml is a singleton, so it is created in the root context and cannot see
 * objects declared with an id in keyboard-test.qml.
 *
 * Needs nothing but Qt Quick - no Maliit, no presage, no LunaNext.
 */

#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickView>
#include <QQuickItem>
#include <QCommandLineParser>
#include <QDebug>
#include <QImage>
#include <QTimer>

namespace {

const char *const kStubNames[] = {
    "maliit_input_method",
    "maliit_geometry",
    "maliit_event_handler",
    "maliit_word_engine",
    "maliit_wordribbon",
    "audioFeedback",
};

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("luneos-keyboard-preview"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Desktop preview for the LuneOS keyboard layouts."));
    parser.addHelpOption();
    const QCommandLineOption grabOption(
        QStringList() << QStringLiteral("grab"),
        QStringLiteral("Render one frame to <file> and exit. Useful headless, with "
                       "-platform offscreen."),
        QStringLiteral("file"));
    parser.addOption(grabOption);
    const QCommandLineOption envOption(
        QStringLiteral("env"),
        QStringLiteral("Device profile index from SettingsStub.testEnvs: 0 mako, "
                       "1 a500, 2 gnexus, 3 grouper, 4 tenderloin (the TouchPad, and "
                       "the default)."),
        QStringLiteral("index"));
    parser.addOption(envOption);
    const QCommandLineOption sizeOption(
        QStringLiteral("size"), QStringLiteral("Keyboard size: XS, S, M or L."),
        QStringLiteral("size"));
    parser.addOption(sizeOption);
    const QCommandLineOption langOption(
        QStringLiteral("lang"), QStringLiteral("Active language, e.g. de or ru."),
        QStringLiteral("code"));
    parser.addOption(langOption);
    const QCommandLineOption layoutOption(
        QStringLiteral("layout"),
        QStringLiteral("Alternative layout, e.g. Dvorak or Thumb. Default LuneOS."),
        QStringLiteral("name"));
    parser.addOption(layoutOption);
    const QCommandLineOption contentOption(
        QStringLiteral("content"),
        QStringLiteral("Content type: 0 text, 1 number, 2 telephone, 3 email, 4 url."),
        QStringLiteral("n"));
    parser.addOption(contentOption);
    const QCommandLineOption languagesOption(
        QStringLiteral("languages"),
        QStringLiteral("Comma-separated enabled languages. More than one makes the "
                       "language key appear, which splits the symbol key."),
        QStringLiteral("list"));
    parser.addOption(languagesOption);
    const QCommandLineOption candidateOption(
        QStringLiteral("candidate"),
        QStringLiteral("Word for the space bar to show, as the reference paints the "
                       "auto-select candidate there."),
        QStringLiteral("word"));
    parser.addOption(candidateOption);
    const QCommandLineOption enterOption(
        QStringLiteral("enter-label"),
        QStringLiteral("Label the application asks for on Return, e.g. Go or Search."),
        QStringLiteral("text"));
    parser.addOption(enterOption);
    parser.process(app);

    const QString here = QStringLiteral(KEYBOARD_TEST_DIR);

    QQuickView view;
    view.engine()->addImportPath(here);
    view.engine()->addImportPath(here + QStringLiteral("/../../qml"));

    QQmlComponent stubs(view.engine(), QUrl::fromLocalFile(here + QStringLiteral("/Stubs.qml")));
    QObject *const stubRoot = stubs.create();
    if (!stubRoot) {
        qCritical().noquote() << "Stubs.qml failed to load:" << stubs.errorString();
        return 1;
    }
    stubRoot->setParent(&app); // outlive the view

    for (const char *name : kStubNames) {
        const QVariant value = stubRoot->property(name);
        if (!value.isValid()) {
            qCritical().noquote() << "Stubs.qml has no property named" << name;
            return 1;
        }
        view.engine()->rootContext()->setContextProperty(QLatin1String(name), value);
    }
    view.engine()->rootContext()->setContextProperty(QStringLiteral("stubs"), stubRoot);

    if (parser.isSet(envOption))
        stubRoot->setProperty("startEnv", parser.value(envOption).toInt());

    QObject *const im = stubRoot->property("maliit_input_method").value<QObject *>();
    if (parser.isSet(sizeOption))
        im->setProperty("keyboardSize", parser.value(sizeOption));
    if (parser.isSet(langOption))
        im->setProperty("activeLanguage", parser.value(langOption));
    if (parser.isSet(layoutOption))
        im->setProperty("keyboardLayout", parser.value(layoutOption));
    if (parser.isSet(contentOption))
        im->setProperty("contentType", parser.value(contentOption).toInt());
    if (parser.isSet(candidateOption))
        im->setProperty("primaryCandidate", parser.value(candidateOption));
    if (parser.isSet(enterOption))
        im->setProperty("actionKeyLabel", parser.value(enterOption));
    if (parser.isSet(languagesOption))
        im->setProperty("enabledLanguages",
                        parser.value(languagesOption).split(QLatin1Char(',')));

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl::fromLocalFile(here + QStringLiteral("/keyboard-test.qml")));
    if (!view.rootObject()) {
        qCritical() << "keyboard-test.qml failed to load";
        return 1;
    }

    view.setTitle(QStringLiteral("LuneOS keyboard preview"));
    const int wantedW = view.rootObject()->property("wantedWidth").toInt();
    const int wantedH = view.rootObject()->property("wantedHeight").toInt();
    view.resize(wantedW > 0 ? wantedW : 1024, wantedH > 0 ? wantedH : 800);
    view.show();

    if (parser.isSet(grabOption)) {
        const QString path = parser.value(grabOption);
        // Wait for the show transition in Keyboard.qml to finish: grabbing on the
        // second frame catches the keyboard still translated off the bottom.
        QTimer::singleShot(1500, &view, [&view, path]() {
            const QImage image = view.grabWindow();
            if (image.isNull() || !image.save(path))
                qCritical().noquote() << "could not write" << path;
            else
                qInfo().noquote() << "wrote" << path;
            QCoreApplication::quit();
        });
    }

    return app.exec();
}
