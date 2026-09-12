TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
QT             += core5compat
INCLUDEPATH    += \
    $${TOP_SRCDIR}/src/ \
    $${TOP_SRCDIR}/src/lib/ \
    $${TOP_SRCDIR}/src/lib/logic/ \
    $${TOP_SRCDIR}/plugins/westernsupport

HEADERS         = \
    ukrainianplugin.h

TARGET          = $$qtLibraryTarget(ukrainianplugin)

EXAMPLE_FILES = ukrainianplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/uk/

lang_db_uk.path = $$PLUGIN_INSTALL_PATH
lang_db_uk.files += $$OUT_PWD/database_uk.db
lang_db_uk.commands += \
  rm -f $$lang_db_uk.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_uk.files $$PWD/tatoeba_uk.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_uk.files $$PWD/tatoeba_uk.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_uk.files $$PWD/tatoeba_uk.txt && \
  cp $$lang_db_uk.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_uk

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_uk

OTHER_FILES += \
    ukrainianplugin.json \
    tatoeba_uk.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

CONFIG += link_pkgconfig
PKGCONFIG += luna-service2 glib-2.0

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
