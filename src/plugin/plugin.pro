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
    QT = core gui widgets quick qml network
}

# The Pre Keyboard swipes the words hunspell knows (see src/pre/preswipewords.h).
DEFINES += HUNSPELL_DICT_PATH=\\\"$$HUNSPELL_DICT_PATH\\\"

# KLIPY's app key, for the Pre Keyboard's GIFs and stickers: qmake KLIPY_KEY=..., or the
# KLIPY_KEY environment variable at qmake time. Without one, their tabs say so. Whatever
# key is used ends up readable in the library.
isEmpty(KLIPY_KEY): KLIPY_KEY = $$(KLIPY_KEY)
!isEmpty(KLIPY_KEY): DEFINES += PRE_KLIPY_KEY=\\\"$$KLIPY_KEY\\\"

LIBS += -lluna-service2

# The Pre Keyboard's swipe reads presage's word databases with SQLite itself (preswipewords.cpp).
LIBS += -lsqlite3

# Dictation from the Pre Keyboard's period (src/pre/prevosk.cpp): the microphone
# through PulseAudio, and Vosk (libvosk.so.0) loaded when first used, with a model
# per language in VOSK_MODEL_DIR. Without Vosk or a model the period has no
# microphone. CONFIG+=no-dictation leaves it out.
!no-dictation {
    DEFINES += PRE_DICTATION
    isEmpty(VOSK_MODEL_DIR): VOSK_MODEL_DIR = $$PREFIX/share/vosk
    DEFINES += VOSK_MODEL_DIR=\\\"$$VOSK_MODEL_DIR\\\"
    HEADERS += ../pre/prevosk.h
    SOURCES += ../pre/prevosk.cpp
    LIBS += -lpulse-simple -lpulse
}

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
    periodhold.h \
    t9multitap.h \
#    updatenotifier.h \
    luneosapplicationapiwrapper.h \
    ../pre/precontroller.h \
    ../pre/predictation.h \
    ../pre/preemojipanel.h \
    ../pre/preklipy.h \
    ../pre/prekeyboardview.h \
    ../pre/prekeymap.h \
    ../pre/prelanguage.h \
    ../pre/premediapanel.h \
    ../pre/prepainter.h \
    ../pre/presettings.h \
    ../pre/preswipedecoder.h \
    ../pre/preemojiwords.h \
    ../pre/preswipewords.h \

SOURCES += \
    plugin.cpp \
    inputmethod.cpp \
    editor.cpp \
    hardwarekeyboard.cpp \
    keyboardgeometry.cpp \
    keyboardsettings.cpp \
    keyboardlogging.cpp \
    panelstate.cpp \
    periodhold.cpp \
    t9multitap.cpp \
#    updatenotifier.cpp \
    luneosapplicationapiwrapper.cpp \
    ../pre/precontroller.cpp \
    ../pre/predictation.cpp \
    ../pre/preemojipanel.cpp \
    ../pre/preklipy.cpp \
    ../pre/prekeyboardview.cpp \
    ../pre/prekeymap.cpp \
    ../pre/prelanguage.cpp \
    ../pre/prelanguages_generated.cpp \
    ../pre/premediapanel.cpp \
    ../pre/prepainter.cpp \
    ../pre/presettings.cpp \
    ../pre/preswipedecoder.cpp \
    ../pre/preemojiwords.cpp \
    ../pre/preswipewords.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target

include(../word-prediction.pri)
