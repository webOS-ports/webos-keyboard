TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_da.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/da/"
lang_da.files = *.qml *.js

INSTALLS += lang_da

# for QtCreator
OTHER_FILES += \
    Keyboard_da_tablet.qml \
    Keyboard_da_phone.qml
