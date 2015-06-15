TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_hu.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/hu/"
lang_hu.files = *.qml *.js

INSTALLS += lang_hu

# for QtCreator
OTHER_FILES += \
    Keyboard_hu_tablet.qml \
    Keyboard_hu_phone.qml

