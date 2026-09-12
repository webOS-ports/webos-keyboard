# Desktop preview

Renders the real keyboard QML on a developer machine, with no Maliit, presage or
LunaNext installed.

    qmake && make && ./keyboard-test

or open `keyboard-test.pro` in Qt Creator and press Run.

## Why there is a C++ launcher

The keyboard reads `maliit_input_method`, `maliit_geometry`, `maliit_event_handler`,
`maliit_word_engine`, `maliit_wordribbon` and `audioFeedback` as **context
properties** on the QML engine's root context. `qml/keys/UI.qml` is a singleton, and
a singleton is instantiated in the root context - it cannot see objects declared with
an `id` somewhere inside `keyboard-test.qml`.

That is why the old `.qmlproject` run rendered a broken keyboard:
`maliit_input_method` was undefined inside `UI.qml`, so `keyboardSizeChoice` came
back undefined, every ratio lookup in `key_constants.js` missed, and each key was
`NaN` wide and tall.

`main.cpp` is only that wiring: it creates `Stubs.qml` and promotes each of its
properties to a context property of the same name. All the faked behaviour stays in
`Stubs.qml`, so it can be changed without rebuilding.

## Options

    --env <index>       device profile from SettingsStub.testEnvs:
                        0 mako, 1 a500, 2 gnexus, 3 grouper,
                        4 tenderloin (the TouchPad - the reference, default)
    --size <XS|S|M|L>   keyboard size
    --lang <code>       active language, e.g. de, ru, uk
    --layout <name>     alternative layout, e.g. Dvorak or Thumb
    --content <0..4>    0 text, 1 number, 2 telephone, 3 email, 4 url
    --languages <list>  comma-separated; more than one shows the language key,
                        which splits the symbol key from 2 units to 1 + 1
    --grab <file>       render one frame to a PNG and exit

`--grab` works headless, which makes layout changes reviewable without a device:

    ./keyboard-test -platform offscreen --lang de --grab de-tablet.png
    ./keyboard-test -platform offscreen --env 0 --lang fr --grab fr-phone.png
