include(../config.pri)

TARGET = dummy
TEMPLATE = lib

qml.path = $$UBUNTU_KEYBOARD_DATA_DIR
qml.files = *.qml *.js

qml_keys.path = "$$UBUNTU_KEYBOARD_DATA_DIR/keys"
qml_keys.files = keys/*.qml keys/qmldir keys/*.js

qml_languages.path = "$$UBUNTU_KEYBOARD_DATA_DIR/languages"
qml_languages.files = languages/*.qml languages/*.js

images.path = "$$UBUNTU_KEYBOARD_DATA_DIR/images"
images.files = images/*.png

images_phone.path = "$$UBUNTU_KEYBOARD_DATA_DIR/images/phone"
images_phone.files = images/phone/*.png

images_tablet.path = "$$UBUNTU_KEYBOARD_DATA_DIR/images/tablet"
images_tablet.files = images/tablet/*.png

INSTALLS += qml qml_keys qml_languages images images_phone images_tablet

# for QtCreator
OTHER_FILES += \
    maliit-keyboard.qml \
    maliit-keyboard-extended.qml \
    maliit-magnifier.qml \
    Keyboard.qml \
    KeyboardContainer.qml \
    OrientationHelper.qml \
    Popper.qml \
    WordRibbon.qml \
    keys/key_constants.js \
    keys/ActionKey.qml \
    keys/CharKey.qml \
    keys/BackspaceKey.qml \
    keys/DropShadow.qml \
    keys/ExtendedListSelector.qml \
    keys/LanguageKey.qml \
	keys/KeyPad.qml \
    keys/Magnifier.qml \
    keys/OneTwoKey.qml \
    keys/PressArea.qml \
    keys/ReturnKey.qml \
    keys/ShiftKey.qml \
    keys/SpaceKey.qml \
    keys/Spacer.qml \
    keys/SymbolShiftKey.qml \
	keys/TabKey.qml \
    keys/UrlKey.qml \
    keys/qmldir \
    languages/Keyboard_numbers.qml \
    languages/Keyboard_symbols.qml \
    languages/Keyboard_telephone.qml \

