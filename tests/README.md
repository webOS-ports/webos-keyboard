# Unit tests and hardening harness

The suite covers the parts of the keyboard plugin that take input from
somewhere it does not control — a physical keyboard's scancodes, the JSON
profiles that say what those scancodes mean, and what procfs says is plugged in
— plus the state machines whose behaviour is invisible on screen.

It is deliberately not a coverage exercise: a test is here because the code it
drives is either a boundary or something that cannot be checked any other way.
`HardwareKeyboard` is both. The compositor hands every key to whoever holds the
input method grab, so `evtest` sees nothing whatever is pressed; what a key
produced is only observable as text arriving in a field; and the devices it is
written for — a Unihertz Titan, a Titan Pocket, a Zinwa Q25, an MP01, a
BlackBerry KEY2 — are not on the desk of whoever is changing it.

## Building and running

The tests compile the sources they cover straight into each binary rather than
linking the plugin. That means the suite needs **nothing but Qt** — no Maliit,
no presage, no hunspell, no LunaNext — so it runs on a plain developer machine
and in CI without a LuneOS sysroot.

With sanitizers on:

```sh
scripts/run-tests.sh
```

That configures `tests/tests.pro` into `build-tests/`, builds it, and runs each
test binary through `make check`. `--no-sanitizers` gives a plain build,
`--full` configures the whole project instead (which does need Maliit, presage
and hunspell), `--build-dir DIR` puts it somewhere else, `--keep` reuses an
existing build.

By hand:

```sh
mkdir build-tests && cd build-tests
qmake6 ../tests/tests.pro
make -j"$(nproc)"
make check
```

A single test, with Qt Test's own options:

```sh
QT_QPA_PLATFORM=offscreen ./ut_hardwarekeyboard/ut_hardwarekeyboard -v2
QT_QPA_PLATFORM=offscreen ./ut_hardwarekeyboard/ut_hardwarekeyboard testAltDoubleTapLocks
```

In an in-tree build of the whole project the tests are a subdirectory like any
other, unless `CONFIG+=notests` is passed:

```sh
qmake6 ../luneos-keyboard.pro && make -j"$(nproc)" && make -C tests check
```

### On a device

The OE recipe passes `CONFIG+=notests`, so nothing here ships. To build the
suite for a target, drop that flag from the recipe's `EXTRA_QMAKEVARS_PRE`; the
binaries land in `${LUNEOS_KEYBOARD_TEST_DIR}`
(`/usr/share/maliit/tests/luneos-keyboard` by default).

Note that this also needs somewhere for them to go in the recipe. Without it
`do_package` stops with

```
QA Issue: Files/directories were installed but not shipped in any package
```

because the default `FILES` do not cover that directory. Add a `-tests` package
in the same change that turns the tests on.

## Sanitizers

`run-tests.sh` builds with ASan and UBSan by default. Most of what the suite
asserts is about reading things that something else wrote — a JSON profile that
may be truncated, a scancode that may be out of range, a capability bitmap
whose word width has to be inferred — and that is exactly the shape of bug a
passing test will happily miss on an uninstrumented build.

`UBSAN_OPTIONS=halt_on_error=1` is set so an overflow fails the run instead of
printing and passing. ASan's leak detection is off: Qt keeps allocations alive
to exit by design and the reports are noise, while its use-after-free and
overflow detection is the point.

## Static analysis

```sh
scripts/static-analysis.sh
```

Runs cppcheck, and clang-tidy when a compilation database is available. The
quickest database to generate is the tests' own, since they need only Qt:

```sh
mkdir build-tidy && cd build-tidy
qmake6 ../tests/tests.pro
bear -- make -j"$(nproc)"
cd .. && scripts/static-analysis.sh --tidy-only --compile-commands build-tidy
```

The clang-tidy selection lives in `.clang-tidy` at the top of the tree. It is
the same selection maliit-framework-webos uses, and narrow on purpose — defect
classes and mechanical style only — so that a finding is worth reading. The
tree is expected to come out clean under both analysers, so a finding is a
finding to fix rather than a category to switch off.

clang-tidy exits 0 for findings unless `WarningsAsErrors` is set, and setting
that in `.clang-tidy` would also fail the build for anyone running it from
their editor. The script therefore judges it by what it printed, which is why
its exit status can be non-zero while clang-tidy's was not.

## What each test covers

| Test | Covers |
| --- | --- |
| `ut_hardwarekeyboard` | Loading the per-device JSON profiles (truncated, nameless, wrong shape, unknown level names, non-numeric scancodes — none of which may take MaliitServer down); selecting one, including the most-specific-wins rule that tells a Titan from a Titan Pocket when both register `aw9523-key`; the Alt and Sym level state machine — held, tapped to latch, double-tapped to lock, third tap to unlock — and that a release replays what its own press decided; the Shift latch; `reset()` on focus leaving a field; and that the six profiles this package ships all parse and load. |

## Not part of the suite

`InputMethod` is not covered here. It derives from `MAbstractInputMethod` and
owns a `QQuickView` the framework registers, so a test of it needs Maliit
installed and gives up the property that makes this suite worth running — that
it needs nothing but Qt. Its two decisions that matter for a hardware keyboard,
`setState()` recording the active input source and `show()` refusing while that
source is physical, are a handful of lines each and are verified on a device. A
Maliit-dependent second tier of tests would be the place for them, and does not
exist yet.

`tests/keyboard-test/` is a desktop preview of the on-screen layouts: a Qt
Quick application for looking at the keyboard, not an automated test. It wants
a display and is built by opening it in Qt Creator, so it is deliberately left
out of `tests/tests.pro`. See its own README.

## Adding a test

```sh
mkdir tests/ut_thing
cat > tests/ut_thing/ut_thing.pro <<'PRO'
TARGET = ut_thing
include(../tests.pri)
SOURCES += ut_thing.cpp $$PWD/../../src/plugin/thing.cpp
PRO
```

List the sources the test needs directly, as above — `tests.pri` deliberately
does not link the plugin, so that the suite keeps building without Maliit
installed. Add `ut_thing` to `SUBDIRS` in `tests/tests.pro`, keeping the list
alphabetical.

End the test source with `QTEST_GUILESS_MAIN` unless it needs `QWindow`, in
which case use `QTEST_MAIN`; the `check` target already forces the offscreen
platform plugin.

## Driving a device by hand

`tools/vkbd.py` is a uinput keyboard, to be run on the device. It advertises
EV_REP, so holding a key produces the input core's own auto-repeat - the same
events a real keyboard's driver produces - and Qt's evdevkeyboard plugin finds it
by discovery, so the compositor picks it up with no configuration.

It is here because the questions that matter in this area are not answerable by
reading the code: whether a held key repeats, whether a modifier survives the
input method's keyboard grab, whether a shortcut reaches the application. Each is
settled by injecting and counting what comes out the other end - maliit's own
`key press` lines are usually the most direct place to count.

    adb push tests/tools/vkbd.py /tmp/
    adb shell 'cd /tmp && python3 -c "
    import vkbd
    fd = vkbd.open_device()
    vkbd.hold(fd, \"c\", 1.5)
    vkbd.close_device(fd)"'
    adb shell 'journalctl -u maliit-server@0 --since "-20s" | grep -c "key press 0x43"'

The virtual device lands under `/devices/virtual/input`, which
`MImHwKeyboardTracker` deliberately ignores, so it is not mistaken for a real
keyboard and does not take the on-screen keyboard away while you test.

Not part of `make check`: it needs `/dev/uinput`, root, and a running compositor.
