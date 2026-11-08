# The Pre Keyboard's words

What the Pre Keyboard (`src/pre`) reads at run time, installed to
`/usr/share/maliit/plugins/org/luneos/pre`. The files are the Android Pre Keyboard's
(https://github.com/alan-morford/prekeyboard), which this keyboard was ported from; its
`tools/` hold the scripts that generate them, and its README the commands and sources.

| File | What it is |
|---|---|
| `dictionary.txt` | US English: each word and its log2 frequency score |
| `en_GB`, `en_CA`, `en_AU/dictionary.txt` | How each regional English differs from the US one (`colour`, `-color`) |
| `fr`, `de`, `es`, `it/dictionary.txt` | Each language's words |
| `next_words.txt`, `*/next_words.txt` | What commonly follows each word, for the bar before a word is typed |
| `emoji_words.txt`, `*/emoji_words.txt` | The emoji the bar offers for a word (`pizza` → 🍕) |
| `contractions.txt` | How often an ambiguous English word (`ill`, `its`) is its contraction after another word |
| `emoji.txt` | The emoji page: every emoji, by category |
| `key_click.wav` | The click for "Sound on keys" |

## Sources and licences

- `dictionary.txt` and the regional English dictionaries are generated from
  - SCOWL (Spell Checker Oriented Word Lists), http://wordlist.aspell.net/,
    Copyright 2000-2018 by Kevin Atkinson. Permission to use, copy, modify, distribute and
    sell these word lists, the associated scripts, the output created from the scripts, and
    its documentation for any purpose is hereby granted without fee, provided that the above
    copyright notice appears in all copies and that both that copyright notice and this
    permission notice appear in supporting documentation. Kevin Atkinson makes no
    representations about the suitability of this array for any purpose. It is provided
    "as is" without express or implied warranty.
  - FrequencyWords by Hermit Dave, https://github.com/hermitdave/FrequencyWords
    (OpenSubtitles 2018), CC BY-SA 4.0.

  The generated dictionaries are therefore available under CC BY-SA 4.0.
- The French, German, Spanish and Italian dictionaries are generated from FrequencyWords
  (CC BY-SA 4.0) and Tatoeba's sentences in each language (https://tatoeba.org,
  CC BY 2.0 FR), and are available under CC BY-SA 4.0.
- `next_words.txt` and `contractions.txt`, and each language's `next_words.txt`, are
  generated from Tatoeba's sentences, CC BY 2.0 FR
  (https://creativecommons.org/licenses/by/2.0/fr/), and are available under the same.
- `emoji.txt` is generated from Unicode's `emoji-test.txt`, and the `emoji_words.txt` files
  from the Unicode CLDR annotations (https://github.com/unicode-org/cldr), Copyright (c)
  Unicode, Inc., under the Unicode License v3 (https://www.unicode.org/license.txt).
- `key_click.wav` is Alan Morford's own recording, under the Apache License 2.0 like the
  keyboard's code.
