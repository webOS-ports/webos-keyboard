TARGET = ut_wordengine
include(../tests.pri)

QT += gui

# languageplugininterface.h includes <presage.h>, which the suite does not have.
INCLUDEPATH += $$PWD/stubs $$PWD/../../src/view $$PWD/../../src/lib/logic

SOURCES += \
    ut_wordengine.cpp \
    $$PWD/../../src/view/abstracttexteditor.cpp \
    $$PWD/../../src/lib/logic/abstractwordengine.cpp \
    $$PWD/../../src/lib/logic/correction.cpp \
    $$PWD/../../src/lib/logic/wordengine.cpp \
    $$PWD/../../src/lib/models/area.cpp \
    $$PWD/../../src/lib/models/key.cpp \
    $$PWD/../../src/lib/models/text.cpp \
    $$PWD/../../src/lib/models/wordcandidate.cpp \
    $$PWD/../../src/lib/models/wordribbon.cpp

HEADERS += \
    $$PWD/../../src/view/abstracttexteditor.h \
    $$PWD/../../src/lib/logic/abstractwordengine.h \
    $$PWD/../../src/lib/logic/wordengine.h \
    $$PWD/../../src/lib/models/wordribbon.h
