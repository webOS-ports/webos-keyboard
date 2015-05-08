TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_he.path = "$${UBUNTU_KEYBOARD_LIB_DIR}/he/"
lang_he.files = *.qml *.js

INSTALLS += lang_he

# for QtCreator
OTHER_FILES += \
    Keyboard_he_tablet.qml \
    Keyboard_he_tablet_email.qml \
    Keyboard_he_tablet_url.qml \
    Keyboard_he_tablet_url_search.qml \
    Keyboard_he_phone.qml \
    Keyboard_he_phone_email.qml \
    Keyboard_he_phone_url.qml \
    Keyboard_he_phone_url_search.qml


