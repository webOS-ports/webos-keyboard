TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_pl.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/pl/"
lang_pl.files = *.qml *.js

INSTALLS += lang_pl

# for QtCreator
OTHER_FILES += \
    Keyboard_pl_tablet.qml \
    Keyboard_pl_tablet_email.qml \
    Keyboard_pl_tablet_url_search.qml \
    Keyboard_pl_phone.qml \
    Keyboard_pl_phone_email.qml \
    Keyboard_pl_phone_url_search.qml

