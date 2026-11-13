TARGET = ut_periodhold
include(../tests.pri)

SOURCES += \
    ut_periodhold.cpp \
    $$PWD/../../src/plugin/periodhold.cpp

HEADERS += $$PWD/../../src/plugin/periodhold.h
