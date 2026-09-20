# Physical keyboard profiles

Phones with a hardware QWERTY print two or three characters on every key face.
The plain letter is what the kernel reports; the others are reached with the
**Alt** key (the legend printed on the key itself) and, on BlackBerry-style
keyboards, a second **Sym** key.

The kernel driver knows nothing about the legends — it reports one scancode per
physical key and that is all. A stock keymap therefore turns the KEY2's `$` key
into a `4`, and Alt+Q into a useless `Alt+q` shortcut. The files in this
directory tell the input method what each key face actually says, so those keys
produce the character printed on them.

A profile is pure data. On a device with no matching profile the input method
behaves exactly as it did before this directory existed.

## How a profile is chosen

At runtime `HardwareKeyboard` reads `/proc/bus/input/devices` and picks the
first profile whose `match.inputDeviceNames` contains one of the names it finds.
The scan is repeated while nothing matches, because the keyboard's input device
can appear well after `maliit-server` starts.

Two environment variables override that, for trying a profile out on a running
device without reinstalling anything:

| Variable | Effect |
|---|---|
| `LUNEOS_KEYBOARD_HW_LAYOUT` | Force this profile by name, or `none` to disable the whole mechanism |
| `LUNEOS_KEYBOARD_HW_LAYOUT_DIR` | Look for profiles here first, before the installed directory |

Set them in `/etc/maliit/maliit-env.conf` and restart `maliit-server`.

## The format

```json
{
    "name": "athena-qwerty",
    "description": "BlackBerry KEY2 / KEY2 LE physical keyboard, QWERTY variant",
    "match": { "inputDeviceNames": ["stmpe_keypad"] },
    "altKeys": [56],
    "symKeys": [100],
    "lockOnDoubleTap": true,
    "levels": {
        "base":  { "5": "$" },
        "shift": { "5": "€", "11": "0" },
        "alt":   { "16": "#", "17": "1" },
        "sym":   { "16": "~", "17": "`" }
    }
}
```

Every key is an **evdev scancode** written as a decimal string — `16` for
`KEY_Q`, `56` for `KEY_LEFTALT`, and so on from `linux/input-event-codes.h`.
Scancodes are used rather than keysyms because they are the one thing that does
not change with whatever xkb keymap the compositor happens to have loaded.

| Field | Meaning |
|---|---|
| `name` | Profile id; what `LUNEOS_KEYBOARD_HW_LAYOUT` matches |
| `match.inputDeviceNames` | Input device names, as `N: Name="…"` in `/proc/bus/input/devices` spells them |
| `altKeys` | Scancodes that select the `alt` level |
| `symKeys` | Scancodes that select the `sym` level |
| `lockOnDoubleTap` | Whether a second tap locks the level (default `true`) |
| `levels.base` | Text for a key whose unmodified character the keymap gets wrong |
| `levels.shift` | The same, with Shift held |
| `levels.alt` | The character printed on the key face, reached with Alt |
| `levels.sym` | The second alternate character, reached with Sym |

Only levels that differ from what a plain US keymap already produces need an
entry, so a profile stays as small as the keyboard is unusual: the Titan needs
nothing but a `sym` level.

`levels.base` and `levels.shift` exist for keys that are not really what the
keymap calls them. The KEY2's `$` key is the example — its driver reports
`KEY_4`, which would otherwise type a `4`.

## What the Alt and Sym keys do

The state machine is the familiar BlackBerry one, and is the same for both keys:

* **Tap** — latches the level for exactly one key, then releases.
* **Tap twice** — locks the level until it is tapped again (unless
  `lockOnDoubleTap` is `false`).
* **Tap a third time** — clears a lock.
* **Hold, and type** — those keys get the level; releasing ends it.

The level is resolved on key *press* and replayed on the matching release, so a
latch that is spent halfway through a keystroke cannot turn one key into two
different characters.

A key with no entry at the active level — Return or Backspace, typically — is
passed to the application unchanged rather than swallowed, but it does spend the
latch.

## Adding a device

`tools/make-hwkeyboard-profile.py` converts a vendor's own description of the
keyboard, so the mapping is transcribed by a program rather than by hand. It
reads either of the two formats vendors ship:

**Android** — a `.kl` (scancode → keycode name) and a `.kcm` (keycode name →
the character at each modifier level), from a stock ROM's `/system/usr/` or from
a LineageOS device tree:

```sh
./tools/make-hwkeyboard-profile.py --name athena-qwerty \
    --description 'BlackBerry KEY2 / KEY2 LE physical keyboard, QWERTY variant' \
    --device-name stmpe_keypad --alt-key 56 --sym-key 100 \
    --kl stmpe.kl --kcm stmpe.kcm > data/hwkeyboard/athena-qwerty.json
```

**xkb** — a symbols file using the usual `<AD01>` key names, with the alternate
character in the level-3 column. This is what the Ubuntu Touch ports ship:

```sh
./tools/make-hwkeyboard-profile.py --name titan \
    --description 'Unihertz Titan physical keyboard (aw9523 matrix), US layout' \
    --device-name aw9523-key --sym-key 100 \
    --xkb titan > data/hwkeyboard/titan.json
```

The converter refuses an xkb keysym name it does not know rather than guessing
at it: a keyboard layout that is quietly wrong is worse than one that is
missing.

To find the values you need on a device that boots:

```sh
grep -h '^N: Name' /proc/bus/input/devices   # the name to match on
evtest /dev/input/eventN                     # the scancode each key reports
```

## Devices

| Device | Profile | Source |
|---|---|---|
| BlackBerry KEY2 / KEY2 LE (`athena`) | `athena-qwerty`, `athena-azerty`, `athena-qwertz` | BlackBerry's own `stmpe{,_azerty,_qwertz}.{kl,kcm}` in `android_device_blackberry_sdm660-common` |
| Unihertz Titan | `titan` | The xkb symbols file the Ubuntu Touch `unihertz-titan` port ships (`unihertz_vndr/titan`) |
| Zinwa Q25 | none, and none wanted | Its `bbqX0kbd` driver resolves both levels in the kernel and reports the resulting keycode, so there is nothing left here to do. Confirm with `evtest`: Alt+Q should report `KEY_3` with Shift, not `KEY_Q` |
| Unihertz Titan Pocket, Titan Slim | not written yet | No authoritative layout found. Take the stock `.kl`/`.kcm` out of `/system/usr/` (or `/vendor/usr/`) on a stock ROM and run the converter |
| Minimal Phone MP01 | not written yet | Same: its AW9523B driver's key table is only partly characterised. Dump the stock `.kl`/`.kcm`, or read the keymap out of the vendor `.ko` |

The KEY2 driver renames its input device by keyboard variant
(`stmpe_keypad`, `stmpe_azerty_keypad`, `stmpe_qwertz_keypad`), so the right one
of the three profiles is picked automatically.
