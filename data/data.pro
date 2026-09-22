include(../config.pri)

TARGET = dummy
TEMPLATE = lib

styles.path = $${LUNEOS_KEYBOARD_DATA_DIR}
styles.files = styles

# Per-device physical keyboard profiles; see hwkeyboard/README.md.
hwkeyboard.path = $${LUNEOS_KEYBOARD_DATA_DIR}
hwkeyboard.files = hwkeyboard

INSTALLS += styles hwkeyboard
