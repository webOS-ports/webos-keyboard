TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_en.path = "$$UBUNTU_KEYBOARD_LIB_DIR/en/"
lang_en.files = *.qml *.js

INSTALLS += lang_en

# for QtCreator
OTHER_FILES += \
    Keyboard_en_tablet.qml \
    Keyboard_en_tablet_email.qml \
    Keyboard_en_tablet_url.qml \
    Keyboard_en_tablet_url_search.qml \
    Keyboard_en_phone.qml \
    Keyboard_en_phone_email.qml \
    Keyboard_en_phone_url.qml \
    Keyboard_en_phone_url_search.qml


