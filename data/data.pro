include(../config.pri)

TARGET = dummy
TEMPLATE = lib

styles.path = $${LUNEOS_KEYBOARD_DATA_DIR}
styles.files = styles

# Per-device physical keyboard profiles; see hwkeyboard/README.md.
hwkeyboard.path = $${LUNEOS_KEYBOARD_DATA_DIR}
hwkeyboard.files = hwkeyboard

# The Pre Keyboard's emoji page, and the emoji it offers for words.
# See pre/README.md for where it comes from and its licence.
pre.path = $${LUNEOS_KEYBOARD_DATA_DIR}
pre.files = pre

INSTALLS += styles hwkeyboard pre
