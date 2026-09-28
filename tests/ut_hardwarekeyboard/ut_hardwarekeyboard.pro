TARGET = ut_hardwarekeyboard
include(../tests.pri)

# The profiles the package ships are checked as data, out of the source tree.
DEFINES += LUNEOS_KEYBOARD_SOURCE_DATA_DIR=\\\"$$PWD/../../data\\\"

SOURCES += \
    ut_hardwarekeyboard.cpp \
    $$PWD/../../src/plugin/hardwarekeyboard.cpp \
    $$PWD/../../src/plugin/keyboardlogging.cpp \

HEADERS += \
    $$PWD/../../src/plugin/hardwarekeyboard.h \
    $$PWD/../../src/plugin/keyboardlogging.h \
