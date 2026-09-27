# Common configuration for the unit tests.
#
# A test .pro includes this after setting TARGET, and gets: Qt Test, the include
# paths for the plugin sources, a "make check" hook that runs the binary in
# place, and an install rule.
#
# Deliberately NOT including ../config.pri. That file errors out when Maliit's
# maliit-defines.prf is not installed, and the whole point of this suite is that
# it builds and runs on a plain Qt machine - no Maliit, no presage, no hunspell,
# no LunaNext. Each test compiles the handful of sources it covers straight into
# its own binary rather than linking the plugin, which is a Maliit plugin and
# drags all of that in.
#
# The suite is built only when qmake is run without CONFIG+=notests. The OE
# recipe passes notests for target images, so nothing here reaches a device
# unless it is asked for explicitly (see tests/README.md).

TEMPLATE = app
CONFIG -= app_bundle
CONFIG += console c++17
QT += core testlib
QT -= gui

TOP_DIR = ../..

INCLUDEPATH += $$PWD/../src/plugin $$PWD/../src/lib $$PWD/../src

# profileDirectories() in hardwarekeyboard.cpp compiles this in. The directory
# does not have to exist: every test drives LUNEOS_KEYBOARD_HW_LAYOUT_DIR, and a
# path that is not there is skipped. It is pointed somewhere harmless rather
# than at the real install tree so that a machine which happens to have LuneOS
# keyboard profiles installed does not have them turn up in a test's results.
DEFINES += LUNEOS_KEYBOARD_DATA_DIR=\\\"/nonexistent/luneos-keyboard\\\"

DESTDIR = $$OUT_PWD

# Tests run headless: none of them wants a display, and the offscreen platform
# plugin keeps that true if one ever grows a QWindow.
check.target = check
check.commands = QT_QPA_PLATFORM=offscreen ./$$TARGET
check.depends = $$TARGET
QMAKE_EXTRA_TARGETS += check

# Standalone builds of tests/tests.pro have no config.pri to set this.
isEmpty(LUNEOS_KEYBOARD_TEST_DIR) {
    LUNEOS_KEYBOARD_TEST_DIR = /usr/share/maliit/tests/luneos-keyboard
}

target.path = $$LUNEOS_KEYBOARD_TEST_DIR/$$TARGET
INSTALLS += target
