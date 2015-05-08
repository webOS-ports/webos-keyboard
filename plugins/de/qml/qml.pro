TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_de.path = "$$UBUNTU_KEYBOARD_LIB_DIR/de/"
lang_de.files = *.qml *.js

INSTALLS += lang_de

# for QtCreator
OTHER_FILES += \
    Keyboard_de_tablet.qml \
    Keyboard_de_tablet_email.qml \
    Keyboard_de_tablet_url.qml \
    Keyboard_de_tablet_url_search.qml \
    Keyboard_de_phone.qml \
    Keyboard_de_phone_email.qml \
    Keyboard_de_phone_url.qml \
    Keyboard_de_phone_url_search.qml

