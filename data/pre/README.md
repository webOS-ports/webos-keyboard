# The Pre Keyboard's data

What the Pre Keyboard (`src/pre`) reads at run time, installed to
`/usr/share/maliit/plugins/org/luneos/pre`.

| File | What it is |
|---|---|
| `emoji.txt` | The emoji page: every emoji, by category |

The Pre Keyboard's words - prediction, spelling, autocorrect and the owner's
dictionary - are the language plugins' (`plugins/<lang>`), the same as for every
other LuneOS layout, and its key click is the keyboard's own.

## Source and licence

`emoji.txt` is generated from Unicode's `emoji-test.txt`
(https://unicode.org/Public/emoji/), Copyright (c) Unicode, Inc., under the
Unicode License v3 (https://www.unicode.org/license.txt). The generator is
`tools/build_emoji.py` in the Android Pre Keyboard
(https://github.com/alan-morford/prekeyboard).
