TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_cs.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/cs/"
lang_cs.files = *.qml *.js

INSTALLS += lang_cs

# for QtCreator
OTHER_FILES += \
    Keyboard_cs_tablet.qml \
    Keyboard_cs_tablet_email.qml \
    Keyboard_cs_tablet_url_search.qml \
    Keyboard_cs_phone.qml \
    Keyboard_cs_phone_email.qml \
    Keyboard_cs_phone_url_search.qml

