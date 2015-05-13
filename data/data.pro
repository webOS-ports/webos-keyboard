include(../config.pri)

TARGET = dummy
TEMPLATE = lib

styles.path = $${UBUNTU_KEYBOARD_DATA_DIR}
styles.files = styles

INSTALLS += styles
