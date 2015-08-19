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
    germanplugin.h

TARGET          = $$qtLibraryTarget(germanplugin)

EXAMPLE_FILES = germanplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/de/

lang_db_de.path = $$PLUGIN_INSTALL_PATH
lang_db_de.files += $$OUT_PWD/database_de.db
lang_db_de.commands += \
  rm -f $$lang_db_de.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_de.files $$PWD/buddenbrooks.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_de.files $$PWD/buddenbrooks.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_de.files $$PWD/buddenbrooks.txt && \
  cp $$lang_db_de.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_de

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_de

OTHER_FILES += \
    germanplugin.json \
    buddenbrooks.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
