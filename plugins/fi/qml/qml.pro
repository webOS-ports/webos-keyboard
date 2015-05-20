TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_fi.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/fi/"
lang_fi.files = *.qml *.js

INSTALLS += lang_fi

# for QtCreator
OTHER_FILES += \
    Keyboard_fi_tablet.qml \
    Keyboard_fi_tablet_email.qml \
    Keyboard_fi_tablet_url_search.qml \
    Keyboard_fi_phone.qml \
    Keyboard_fi_phone_email.qml \
    Keyboard_fi_phone_url_search.qml

