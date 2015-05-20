TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += \
    $${TOP_SRCDIR}/src/ \
    $${TOP_SRCDIR}/src/lib/ \
    $${TOP_SRCDIR}/src/lib/logic/
    $${TOP_SRCDIR}/plugins/westernsupport

HEADERS         = \
    polishplugin.h

TARGET          = $$qtLibraryTarget(polishplugin)

EXAMPLE_FILES = polishplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/pl/

lang_db_pl.path = $$PLUGIN_INSTALL_PATH
lang_db_pl.files += $$OUT_PWD/database_pl.db
lang_db_pl.commands += \
  rm -f $$lang_db_pl.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_pl.files $$PWD/ziemia_obiecana_tom_pierwszy_4.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_pl.files $$PWD/ziemia_obiecana_tom_pierwszy_4.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_pl.files $$PWD/ziemia_obiecana_tom_pierwszy_4.txt && \
  cp $$lang_db_pl.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_pl

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_pl

OTHER_FILES += \
    polishplugin.json \
    ziemia_obiecana_tom_pierwszy_4.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
