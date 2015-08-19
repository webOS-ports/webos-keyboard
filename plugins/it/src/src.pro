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
    italianplugin.h

TARGET          = $$qtLibraryTarget(italianplugin)

EXAMPLE_FILES = italianplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/it/

lang_db_it.path = $$PLUGIN_INSTALL_PATH
lang_db_it.files += $$OUT_PWD/database_it.db
lang_db_it.commands += \
  rm -f $$lang_db_it.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_it.files $$PWD/la_francia_dal_primo_impero.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_it.files $$PWD/la_francia_dal_primo_impero.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_it.files $$PWD/la_francia_dal_primo_impero.txt && \
  cp $$lang_db_it.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_it

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_it

OTHER_FILES += \
    italianplugin.json \
    la_francia_dal_primo_impero.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
