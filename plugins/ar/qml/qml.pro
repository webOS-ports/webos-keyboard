TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_ar.path = "$${UBUNTU_KEYBOARD_LIB_DIR}/ar/"
lang_ar.files = *.qml *.js

INSTALLS += lang_ar

# for QtCreator
OTHER_FILES += \
    Keyboard_ar_tablet.qml \
    Keyboard_ar_tablet_email.qml \
    Keyboard_ar_tablet_url.qml \
    Keyboard_ar_tablet_url_search.qml \
    Keyboard_ar_phone.qml \
    Keyboard_ar_phone_email.qml \
    Keyboard_ar_phone_url.qml \
    Keyboard_ar_phone_url_search.qml

