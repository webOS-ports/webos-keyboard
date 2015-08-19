TOP_BUILDDIR = $$OUT_PWD/../../..
TOP_SRCDIR = $$PWD/../../..

include($${TOP_SRCDIR}/config.pri)

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
INCLUDEPATH    += \
    $${TOP_SRCDIR}/src/ \
    $${TOP_SRCDIR}/src/lib/ \
    $${TOP_SRCDIR}/src/lib/logic/ \
    $${TOP_SRCDIR}/plugins/westernsupport

HEADERS         = \
    frenchplugin.h

TARGET          = $$qtLibraryTarget(frenchplugin)

EXAMPLE_FILES = frenchplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/fr/

lang_db_fr.path = $$PLUGIN_INSTALL_PATH
lang_db_fr.files += $$OUT_PWD/database_fr.db
lang_db_fr.commands += \
  rm -f $$lang_db_fr.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_fr.files $$PWD/les_trois_mousquetaires.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_fr.files $$PWD/les_trois_mousquetaires.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_fr.files $$PWD/les_trois_mousquetaires.txt && \
  cp $$lang_db_fr.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_fr

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_fr


OTHER_FILES += \
    frenchplugin.json \
    les_trois_mousquetaires.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
