TARGET = ut_dictation
include(../tests.pri)

INCLUDEPATH += $$PWD/../../src/pre

SOURCES += \
    ut_dictation.cpp \
    $$PWD/../../src/pre/predictation.cpp \
    $$PWD/../../src/pre/prevosk.cpp

HEADERS += \
    $$PWD/../../src/pre/predictation.h \
    $$PWD/../../src/pre/prevosk.h

LIBS += -lpulse-simple -lpulse
