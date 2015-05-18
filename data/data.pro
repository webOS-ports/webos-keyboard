include(../config.pri)

TARGET = dummy
TEMPLATE = lib

styles.path = $${LUNEOS_KEYBOARD_DATA_DIR}
styles.files = styles

INSTALLS += styles
