TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_sv.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/sv/"
lang_sv.files = *.qml *.js

INSTALLS += lang_sv

# for QtCreator
OTHER_FILES += \
    Keyboard_sv_tablet.qml \
    Keyboard_sv_phone.qml
