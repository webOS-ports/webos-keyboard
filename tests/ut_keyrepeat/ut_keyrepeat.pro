TARGET = ut_keyrepeat
include(../tests.pri)

# The editor sends QKeyEvents, so this one needs QtGui after all.
QT += gui

SOURCES += \
    ut_keyrepeat.cpp \
    $$PWD/../../src/view/abstracttexteditor.cpp \
    $$PWD/../../src/lib/logic/abstractwordengine.cpp \
    $$PWD/../../src/lib/models/area.cpp \
    $$PWD/../../src/lib/models/key.cpp \
    $$PWD/../../src/lib/models/text.cpp \
    $$PWD/../../src/lib/models/wordcandidate.cpp \
    $$PWD/../../src/lib/models/wordribbon.cpp

HEADERS += \
    $$PWD/../../src/view/abstracttexteditor.h \
    $$PWD/../../src/lib/logic/abstractwordengine.h \
    $$PWD/../../src/lib/models/wordribbon.h

INCLUDEPATH += $$PWD/../../src/view
