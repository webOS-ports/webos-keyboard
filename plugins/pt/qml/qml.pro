TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_pt.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/pt/"
lang_pt.files = *.qml *.js

INSTALLS += lang_pt

# for QtCreator
OTHER_FILES += \
    Keyboard_pt_tablet.qml \
    Keyboard_pt_tablet_email.qml \
    Keyboard_pt_tablet_url_search.qml \
    Keyboard_pt_phone.qml \
    Keyboard_pt_phone_email.qml \
    Keyboard_pt_phone_url_search.qml


