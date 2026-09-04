QT       -= gui

contains(QT_MAJOR_VERSION, 6) {
QT       += core5compat
}
TARGET = westernsupport
TEMPLATE = lib
CONFIG += staticlib

DEFINES += WESTERNSUPPORT_LIBRARY

TOP_BUILDDIR = $$OUT_PWD/../..
TOP_SRCDIR = $$PWD/../..
include($${TOP_SRCDIR}/config.pri)

INCLUDEPATH    += \
    $${TOP_SRCDIR}/src/ \
    $${TOP_SRCDIR}/src/lib/ \
    $${TOP_SRCDIR}/src/lib/logic/

DESTDIR         = $${TOP_BUILDDIR}/plugins/plugins

SOURCES += \
    westernsupport.cpp \
    westernlanguagefeatures.cpp \
    westernlanguagesplugin.cpp \
    candidatescallback.cpp \
    spellchecker.cpp

HEADERS += \
    westernsupport.h \
    westernsupport_global.h \
    westernlanguagefeatures.h \
    westernlanguagesplugin.h \
    candidatescallback.h \
    spellchecker.h


target.path = $${LUNEOS_KEYBOARD_LIB_DIR}
INSTALLS += target

# for plugins
API_HEADERS = westernlanguagesplugin.h

api_headers.files = $$API_HEADERS
api_headers.path = $$LUNEOS_KEYBOARD_HEADERS_DIR
INSTALLS += api_headers


# hunspell
CONFIG += link_pkgconfig
PKGCONFIG += hunspell
DEFINES += HAVE_HUNSPELL

# presage
LIBS += -lpresage
DEFINES += HUNSPELL_DICT_PATH=\\\"$$HUNSPELL_DICT_PATH\\\"

# db8, for SpellChecker's user dictionary - see spellchecker.cpp.
# luna-service2.pc does not expose glib-2.0's cflags to a plain pkg-config
# query even though lunaservice.h #includes <glib.h> directly - presage hit
# the identical "fatal error: glib.h: No such file or directory" building
# Db8Predictor, and needed glib-2.0 requested explicitly there too.
CONFIG += link_pkgconfig
PKGCONFIG += luna-service2 glib-2.0
