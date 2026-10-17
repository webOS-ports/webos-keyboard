TARGET = ut_t9multitap
include(../tests.pri)

SOURCES += \
    ut_t9multitap.cpp \
    $$PWD/../../src/plugin/t9multitap.cpp

HEADERS += $$PWD/../../src/plugin/t9multitap.h
