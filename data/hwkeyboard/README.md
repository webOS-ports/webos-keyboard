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

At runtime `HardwareKeyboard` reads `/proc/bus/input/devices` and takes the
profile whose `match.inputDeviceNames` contains one of the names it finds and
whose `match.requireKeys` the device advertises. Where several match, the one
naming the most required keys wins. The scan is repeated while nothing matches,
because the keyboard's input device can appear well after `maliit-server`
starts.

A device name alone is not always enough. The Unihertz Titan and Titan Pocket
both register `aw9523-key`, with different characters on their key faces, so
the key set is what tells them apart: the Titan's driver registers
`KEY_LEFTMETA` (125) and the Pocket's `KEY_COMPOSE` (127), and neither has the
other's. That is what `match.requireKeys` is for.

Three environment variables override all of this, for trying a profile out on a
running device without reinstalling anything:

| Variable | Effect |
|---|---|
| `LUNEOS_KEYBOARD_HW_LAYOUT` | Force this profile by name, or `none` to disable the whole mechanism |
| `LUNEOS_KEYBOARD_HW_LAYOUT_DIR` | Look for profiles here first, before the installed directory |
| `LUNEOS_KEYBOARD_HW_INPUT_DEVICES` | Read the device list from this file instead of `/proc/bus/input/devices`, so a copy taken off a phone can be replayed on a desktop |

Set them in `/etc/maliit/maliit-env.conf` and restart `maliit-server`.

## The format

```json
{
    "name": "athena-qwerty",
    "description": "BlackBerry KEY2 / KEY2 LE physical keyboard, QWERTY variant",
    "match": { "inputDeviceNames": ["stmpe_keypad"], "requireKeys": [] },
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
| `match.requireKeys` | Scancodes the device must advertise, for telling apart two keyboards that share a name. Empty matches anything |
| `altKeys` | Scancodes that select the `alt` level |
| `symKeys` | Scancodes that select the `sym` level |
| `lockOnDoubleTap` | Whether a second tap locks the level (default `true`) |
| `levels.base` | Text for a key whose unmodified character the keymap gets wrong |
| `levels.shift` | The same, with Shift held |
| `levels.alt` | The character printed on the key face, reached with Alt |
| `levels.sym` | The second alternate character, reached with Sym |

Only levels that differ from what a plain US keymap already produces need an
entry, so a profile stays as small as the keyboard is unusual: the Titan needs
nothing but an `alt` level.

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
    --device-name aw9523-key --require-key 125 --alt-key 100 \
    --xkb titan > data/hwkeyboard/titan.json
```

An xkb file's third column becomes the `alt` level by default, since that is
the one printed on the key faces; pass `--xkb-level3 sym` for a keyboard whose
third column really is a separate Sym layer.

Two more options exist for vendor files that describe more keyboard than the
device has: `--baseline-kl`/`--baseline-kcm` keep only the difference from the
AOSP `Generic` pair the vendor edited, and `--drop <scancode>` removes a key
that survives that but still is not real -- the MP01 needs both.

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
| Unihertz Titan Pocket, Titan Slim | `titanpocket` | The xkb symbols file the Ubuntu Touch `unihertz-titanslim-pocket` port ships (`unihertz_vndr/titanpocket`) |
| Minimal Phone MP01 | `mp01` | The stock `aw9523b-key.{kl,kcm}` out of the device's own `system.img`, diffed against the `Generic.{kl,kcm}` beside them |
| Zinwa Q25 | none, and none wanted | Its `bbqX0kbd` driver resolves both levels in the kernel and reports the resulting keycode, so there is nothing left here to do. Confirm with `evtest`: Alt+Q should report `KEY_3` with Shift, not `KEY_Q` |

The KEY2 driver renames its input device by keyboard variant
(`stmpe_keypad`, `stmpe_azerty_keypad`, `stmpe_qwertz_keypad`), so the right one
of the three profiles is picked automatically.

### The three keyboards that report `aw9523-key`

The Titan, Titan Pocket and Titan Slim all register the same input device name,
so their key sets were read out of the drivers to see what really differs:

* **Titan** — vendor source, `drivers/misc/mediatek/aw9523/aw9523_key.c` on the
  Ubuntu Touch `unihertz-titan` kernel. Carries `KEY_LEFTCTRL`, `KEY_LEFTMETA`
  and `KEY_RIGHTALT`; no Sym key at all. The key that reaches the printed
  alternates is the one labelled Alt, on `KEY_RIGHTALT` (100).
* **Titan Pocket** and **Titan Slim** — the `KEY_STATE key_map[]` table
  recovered from each of the two prebuilt kernels the
  `unihertz-titanslim-pocket` port ships. Both carry `KEY_COMPOSE` (the Sym
  key), `KEY_F13`, `KEY_BACK`, `KEY_APPSELECT` and `KEY_RIGHTALT`, and no
  `KEY_LEFTCTRL` or `KEY_LEFTMETA`.

**The Pocket's and the Slim's tables are identical** — same 35 keys, same
scancodes, differing only in which matrix row and column each key is wired to,
which never leaves the driver. From userspace the two devices cannot be told
apart, and the Ubuntu Touch port ships one layout for both, so they share the
`titanpocket` profile here. If the Slim's key faces ever turn out to print
something different, nothing automatic can separate them and the device would
need `LUNEOS_KEYBOARD_HW_LAYOUT` set explicitly.

Their Sym key (`KEY_COMPOSE`, 127) has no characters behind it in any vendor
file, on either device -- on stock Android it opens the emoji panel. It is left
alone here rather than swallowed, so the shell can bind it.
