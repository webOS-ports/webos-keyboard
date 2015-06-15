TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_he.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/he/"
lang_he.files = *.qml *.js

INSTALLS += lang_he

# for QtCreator
OTHER_FILES += \
    Keyboard_he_tablet.qml \
    Keyboard_he_phone.qml

