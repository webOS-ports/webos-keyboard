TARGET = ut_correction
include(../tests.pri)

INCLUDEPATH += $$PWD/../../src/lib/logic

SOURCES += \
    ut_correction.cpp \
    $$PWD/../../src/lib/logic/correction.cpp

HEADERS += $$PWD/../../src/lib/logic/correction.h
