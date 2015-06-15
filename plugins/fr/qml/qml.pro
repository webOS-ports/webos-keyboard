TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_fr.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/fr/"
lang_fr.files = *.qml *.js

INSTALLS += lang_fr

# for QtCreator
OTHER_FILES += \
    Keyboard_fr_tablet.qml \
    Keyboard_fr_phone.qml

