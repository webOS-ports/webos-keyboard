TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_es.path = "$${UBUNTU_KEYBOARD_LIB_DIR}/es/"
lang_es.files = *.qml *.js

INSTALLS += lang_es

# for QtCreator
OTHER_FILES += \
    Keyboard_es_tablet.qml \
    Keyboard_es_tablet_email.qml \
    Keyboard_es_tablet_url.qml \
    Keyboard_es_tablet_url_search.qml \
    Keyboard_es_phone.qml \
    Keyboard_es_phone_email.qml \
    Keyboard_es_phone_url.qml \
    Keyboard_es_phone_url_search.qml


