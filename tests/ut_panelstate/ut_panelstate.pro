TARGET = ut_panelstate
include(../tests.pri)

SOURCES += \
    ut_panelstate.cpp \
    $$PWD/../../src/plugin/panelstate.cpp

HEADERS += $$PWD/../../src/plugin/panelstate.h
