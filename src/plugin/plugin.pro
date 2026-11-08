include(../config.pri)
include(../config-plugin.pri)

TOP_BUILDDIR = $${OUT_PWD}/../../
TARGET = $${LUNEOS_KEYBOARD_PLUGIN_TARGET}
TEMPLATE = lib
LIBS += \
    $${TOP_BUILDDIR}/$${LUNEOS_KEYBOARD_VIEW_LIB} \
    $${TOP_BUILDDIR}/$${LUNEOS_KEYBOARD_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${LUNEOS_KEYBOARD_VIEW_LIB} $${TOP_BUILDDIR}/$${LUNEOS_KEYBOARD_LIB}
INCLUDEPATH += ../lib ../
DEFINES += MALIIT_DEFAULT_PROFILE=\\\"$$MALIIT_DEFAULT_PROFILE\\\"

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets quick qml
}

LIBS += -lluna-service2

CONFIG += \
    plugin \

HEADERS += \
    plugin.h \
    inputmethod.h \
    inputmethod_p.h \
    editor.h \
    hardwarekeyboard.h \
    keyboardgeometry.h \
    keyboardsettings.h \
    keyboardlogging.h \
    panelstate.h \
    t9multitap.h \
#    updatenotifier.h \
    luneosapplicationapiwrapper.h \
    ../pre/precomposer.h \
    ../pre/precontroller.h \
    ../pre/precorrector.h \
    ../pre/preemojipanel.h \
    ../pre/prekeyboardview.h \
    ../pre/prekeymap.h \
    ../pre/prelanguage.h \
    ../pre/prepainter.h \
    ../pre/presettings.h \
    ../pre/preswipedecoder.h \
    ../pre/prewords.h \

SOURCES += \
    plugin.cpp \
    inputmethod.cpp \
    editor.cpp \
    hardwarekeyboard.cpp \
    keyboardgeometry.cpp \
    keyboardsettings.cpp \
    keyboardlogging.cpp \
    panelstate.cpp \
    t9multitap.cpp \
#    updatenotifier.cpp \
    luneosapplicationapiwrapper.cpp \
    ../pre/precomposer.cpp \
    ../pre/precontroller.cpp \
    ../pre/precorrector.cpp \
    ../pre/preemojipanel.cpp \
    ../pre/prekeyboardview.cpp \
    ../pre/prekeymap.cpp \
    ../pre/prelanguage.cpp \
    ../pre/prepainter.cpp \
    ../pre/presettings.cpp \
    ../pre/preswipedecoder.cpp \
    ../pre/prewords.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target

include(../word-prediction.pri)
