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
    hebrewplugin.h

TARGET          = $$qtLibraryTarget(hebrewplugin)

EXAMPLE_FILES = hebrewplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/he/

lang_db_he.path = $$PLUGIN_INSTALL_PATH
lang_db_he.files += $$OUT_PWD/database_he.db
lang_db_he.commands += \
  rm -f $$lang_db_he.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_he.files $$PWD/free_ebook.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_he.files $$PWD/free_ebook.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_he.files $$PWD/free_ebook.txt && \
  cp $$lang_db_he.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_he

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_he

OTHER_FILES += \
    hebrewplugin.json \
    free_ebook.txt

LIBS += $${TOP_BUILDDIR}/plugins/plugins/libwesternsupport.a -lpresage

enable-hunspell {
    # hunspell
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

INCLUDEPATH += $$PWD/../../westernsupport
DEPENDPATH += $$PWD/../../westernsupport
