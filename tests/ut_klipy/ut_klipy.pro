TARGET = ut_klipy
include(../tests.pri)

# The Pre Keyboard's GIFs and stickers (src/pre/preklipy, premediapanel), with
# no network: KLIPY's answers, the link, the hourly limit, the grid's pictures.
QT += gui network

INCLUDEPATH += $$PWD/../../src/pre

SOURCES += \
    ut_klipy.cpp \
    $$PWD/../../src/pre/preklipy.cpp \
    $$PWD/../../src/pre/premediapanel.cpp

HEADERS += \
    $$PWD/../../src/pre/preklipy.h \
    $$PWD/../../src/pre/premediapanel.h
