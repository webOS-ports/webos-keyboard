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
    russianplugin.h

TARGET          = $$qtLibraryTarget(russianplugin)

EXAMPLE_FILES = russianplugin.json

# generate database for presage:
PLUGIN_INSTALL_PATH = $${LUNEOS_KEYBOARD_LIB_DIR}/ru/

lang_db_ru.path = $$PLUGIN_INSTALL_PATH
lang_db_ru.files += $$OUT_PWD/database_ru.db
lang_db_ru.commands += \
  rm -f $$lang_db_ru.files && \
  text2ngram -n 1 -l -f sqlite -o $$lang_db_ru.files $$PWD/free_ebook.txt && \
  text2ngram -n 2 -l -f sqlite -o $$lang_db_ru.files $$PWD/free_ebook.txt && \
  text2ngram -n 3 -l -f sqlite -o $$lang_db_ru.files $$PWD/free_ebook.txt && \
  cp $$lang_db_ru.files \"$(INSTALL_ROOT)\"$$PLUGIN_INSTALL_PATH

QMAKE_EXTRA_TARGETS += lang_db_ru

target.path = $$PLUGIN_INSTALL_PATH
INSTALLS += target lang_db_ru

OTHER_FILES += \
    russianplugin.json \
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
