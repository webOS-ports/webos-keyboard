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
    keyboardgeometry.h \
    keyboardsettings.h \
#    updatenotifier.h \
    luneosapplicationapiwrapper.h \

SOURCES += \
    plugin.cpp \
    inputmethod.cpp \
    editor.cpp \
    keyboardgeometry.cpp \
    keyboardsettings.cpp \
#    updatenotifier.cpp \
    luneosapplicationapiwrapper.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target

include(../word-prediction.pri)
