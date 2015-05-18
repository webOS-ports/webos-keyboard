TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_nl.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/nl/"
lang_nl.files = *.qml *.js

INSTALLS += lang_nl

# for QtCreator
OTHER_FILES += \
    Keyboard_nl_tablet.qml \
    Keyboard_nl_tablet_email.qml \
    Keyboard_nl_tablet_url_search.qml \
    Keyboard_nl_phone.qml \
    Keyboard_nl_phone_email.qml \
    Keyboard_nl_phone_url_search.qml


