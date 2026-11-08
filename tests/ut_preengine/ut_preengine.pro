TARGET = ut_preengine
include(../tests.pri)

# The Pre Keyboard's engine, against the dictionaries that ship in data/pre: the
# Android Pre Keyboard's own tests, ported with the code.
DEFINES += PRE_DATA_DIR=\\\"$$PWD/../../data/pre\\\"
INCLUDEPATH += $$PWD/../../src/pre

SOURCES += \
    ut_preengine.cpp \
    $$PWD/../../src/pre/precomposer.cpp \
    $$PWD/../../src/pre/precorrector.cpp \
    $$PWD/../../src/pre/prekeymap.cpp \
    $$PWD/../../src/pre/prelanguage.cpp \
    $$PWD/../../src/pre/preswipedecoder.cpp \
    $$PWD/../../src/pre/prewords.cpp

HEADERS += \
    $$PWD/../../src/pre/precomposer.h \
    $$PWD/../../src/pre/precorrector.h \
    $$PWD/../../src/pre/prekeymap.h \
    $$PWD/../../src/pre/prelanguage.h \
    $$PWD/../../src/pre/preswipedecoder.h \
    $$PWD/../../src/pre/prewords.h
