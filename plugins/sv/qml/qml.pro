TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TARGET = dummy
TEMPLATE = lib

lang_sv.path = "$${UBUNTU_KEYBOARD_LIB_DIR}/sv/"
lang_sv.files = *.qml *.js

INSTALLS += lang_sv

# for QtCreator
OTHER_FILES += \
    Keyboard_sv_tablet.qml \
    Keyboard_sv_tablet_email.qml \
    Keyboard_sv_tablet_url.qml \
    Keyboard_sv_tablet_url_search.qml \
    Keyboard_sv_tablet.qml \
    Keyboard_sv_tablet_email.qml \
    Keyboard_sv_tablet_url.qml \
    Keyboard_sv_tablet_url_search.qml

