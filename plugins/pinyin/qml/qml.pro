TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_zh.path = "$${LUNEOS_KEYBOARD_LIB_DIR}/zh/"
lang_zh.files = *.qml *.js

INSTALLS += lang_zh

# for QtCreator
OTHER_FILES += \
    Keyboard_zh_tablet.qml \
    Keyboard_zh_phone.qml

