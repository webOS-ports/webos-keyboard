include(../config.pri)

TARGET = dummy
TEMPLATE = lib

styles.path = $${LUNEOS_KEYBOARD_DATA_DIR}
styles.files = styles

# Per-device physical keyboard profiles; see hwkeyboard/README.md.
hwkeyboard.path = $${LUNEOS_KEYBOARD_DATA_DIR}
hwkeyboard.files = hwkeyboard

# The Pre Keyboard's words: dictionaries, next words, emoji, and its click.
# See pre/README.md for where they come from and their licences.
pre.path = $${LUNEOS_KEYBOARD_DATA_DIR}
pre.files = pre

INSTALLS += styles hwkeyboard pre
