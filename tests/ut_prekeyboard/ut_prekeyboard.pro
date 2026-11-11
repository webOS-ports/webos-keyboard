TARGET = ut_prekeyboard
include(../tests.pri)

# The Pre Keyboard (src/pre): its languages and keys for every language plugin,
# its look settings, and the controller that hands its keys to LuneOS's editor -
# down to the editor itself, for backspace. The controller talks to the view, so
# the view and what it draws with are built in too: QtQuick and QtNetwork.
QT += gui qml quick network sql

DEFINES += PLUGINS_DIR=\\\"$$PWD/../../plugins\\\"
INCLUDEPATH += $$PWD/../../src/pre $$PWD/../../src/view

SOURCES += \
    ut_prekeyboard.cpp \
    $$PWD/../../src/pre/precontroller.cpp \
    $$PWD/../../src/pre/preemojipanel.cpp \
    $$PWD/../../src/pre/prekeyboardview.cpp \
    $$PWD/../../src/pre/prekeymap.cpp \
    $$PWD/../../src/pre/preklipy.cpp \
    $$PWD/../../src/pre/prelanguage.cpp \
    $$PWD/../../src/pre/prelanguages_generated.cpp \
    $$PWD/../../src/pre/premediapanel.cpp \
    $$PWD/../../src/pre/prepainter.cpp \
    $$PWD/../../src/pre/presettings.cpp \
    $$PWD/../../src/pre/preswipedecoder.cpp \
    $$PWD/../../src/pre/preswipewords.cpp \
    $$PWD/../../src/view/abstracttexteditor.cpp \
    $$PWD/../../src/lib/logic/abstractwordengine.cpp \
    $$PWD/../../src/lib/logic/eventhandler.cpp \
    $$PWD/../../src/lib/models/area.cpp \
    $$PWD/../../src/lib/models/key.cpp \
    $$PWD/../../src/lib/models/text.cpp \
    $$PWD/../../src/lib/models/wordcandidate.cpp \
    $$PWD/../../src/lib/models/wordribbon.cpp

HEADERS += \
    $$PWD/../../src/pre/precontroller.h \
    $$PWD/../../src/pre/preemojipanel.h \
    $$PWD/../../src/pre/prekeyboardview.h \
    $$PWD/../../src/pre/preklipy.h \
    $$PWD/../../src/pre/premediapanel.h \
    $$PWD/../../src/view/abstracttexteditor.h \
    $$PWD/../../src/lib/logic/abstractwordengine.h \
    $$PWD/../../src/lib/logic/eventhandler.h \
    $$PWD/../../src/lib/models/wordribbon.h
