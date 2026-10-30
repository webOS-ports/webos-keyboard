TARGET = ut_symbolpage
include(../tests.pri)

# A Qt Quick Test: it drives the keyboard's own QML (qml/) with the stand-ins
# for the plugin's context properties from tests/keyboard-test, so like the
# rest of the suite it needs nothing but Qt - here Qt Quick, Qt Quick Test and
# the QtMultimedia QML module, which KeyboardContainer.qml imports.
QT += gui quick qml qmltest

SOURCES += ut_symbolpage.cpp

DEFINES += \
    QUICK_TEST_SOURCE_DIR=\\\"$$PWD\\\" \
    KEYBOARD_TEST_DIR=\\\"$$PWD/../keyboard-test\\\" \
    KEYBOARD_QML_DIR=\\\"$$PWD/../../qml\\\"

OTHER_FILES += tst_symbolpage.qml
