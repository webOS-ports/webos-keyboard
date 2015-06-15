TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_it.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/it/"
lang_it.files = *.qml *.js

INSTALLS += lang_it

# for QtCreator
OTHER_FILES += \
    Keyboard_it_tablet.qml \
    Keyboard_it_phone.qml

