TARGET = ut_spellchecker
include(../tests.pri)

# Stand-ins for luna-service2, hunspell and Qt's QTextCodec (core5compat), none
# of which the suite has: what the test looks at is what goes to db8.
INCLUDEPATH += $$PWD/stubs $$PWD/../../plugins/westernsupport $$PWD/../../src/lib/logic
CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0

DEFINES += HAVE_HUNSPELL HUNSPELL_DICT_PATH=\\\"$$PWD/dict\\\"

SOURCES += \
    ut_spellchecker.cpp \
    $$PWD/../../plugins/westernsupport/spellchecker.cpp

HEADERS += $$PWD/../../plugins/westernsupport/spellchecker.h

OTHER_FILES += dict/en_US.aff dict/en_US.dic
