TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_ru.path = "$${UBUNTU_KEYBOARD_LIB_DIR}/ru/"
lang_ru.files = *.qml *.js

INSTALLS += lang_ru

# for QtCreator
OTHER_FILES += \
    Keyboard_ru_tablet.qml \
    Keyboard_ru_tablet_email.qml \
    Keyboard_ru_tablet_url.qml \
    Keyboard_ru_tablet_url_search.qml \
    Keyboard_ru_phone.qml \
    Keyboard_ru_phone_email.qml \
    Keyboard_ru_phone_url.qml \
    Keyboard_ru_phone_url_search.qml


