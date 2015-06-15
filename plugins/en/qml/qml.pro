TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_en.path = "$$LUNEOS_KEYBOARD_LIB_DIR/en/"
lang_en.files = *.qml *.js

INSTALLS += lang_en

# for QtCreator
OTHER_FILES += \
    Keyboard_en_tablet.qml \
	Keyboard_en_tablet_thumb.qml \
	Keyboard_en_tablet_dvorak.qml \
    Keyboard_en_phone.qml

