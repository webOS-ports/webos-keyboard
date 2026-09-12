# Desktop preview for the keyboard layouts. Deliberately does not include
# ../../config.pri: this builds against plain Qt Quick, with no Maliit, presage or
# LunaNext, so it works on a developer machine that cannot build the plugin itself.
#
# Open this file in Qt Creator and press Run, or from a shell:
#   ~/Qt/<version>/gcc_64/bin/qmake && make && ./keyboard-test

QT       += quick gui
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE  = app
TARGET    = keyboard-test

SOURCES  += main.cpp

# main.cpp resolves Stubs.qml, keyboard-test.qml and ../../qml against this.
DEFINES  += KEYBOARD_TEST_DIR=\\\"$$PWD\\\"

OTHER_FILES += \
    keyboard-test.qml \
    Stubs.qml \
    LunaNext/Common/SettingsStub.qml \
    LunaNext/Common/UnitsStub.qml \
    LunaNext/Common/FontUtilsStub.qml \
    README.md
