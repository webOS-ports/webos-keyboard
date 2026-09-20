#!/usr/bin/env python3
"""Turn a vendor keyboard description into a LuneOS hardware keyboard profile.

The profiles under data/hwkeyboard/ are keyed on evdev scancodes, because that
is the only thing the input method sees that does not depend on whatever xkb
keymap the compositor happens to have loaded.  Vendors describe the same
keyboard in one of two other formats, so this reads both:

  * Android:  a .kl (scancode -> keycode name) plus a .kcm (keycode name ->
              the character each modifier level produces).  This is what a
              stock ROM or a LineageOS device tree ships.
  * xkb:      a symbols file using the usual <AD01> key names, with the
              level-3 column holding the alternate character.  This is what
              the Ubuntu Touch ports ship.

Only levels that differ from what a plain US keymap already produces are
written out, so a profile stays as small as the device is unusual.

  ./tools/make-hwkeyboard-profile.py --name athena-qwerty \
      --description 'BlackBerry KEY2 (QWERTY)' --device-name stmpe_keypad \
      --alt-key 56 --sym-key 100 \
      --kl stmpe.kl --kcm stmpe.kcm > data/hwkeyboard/athena-qwerty.json
"""

import argparse
import json
import re
import sys

# What a plain US keymap already produces for these scancodes.  A level is only
# worth writing into a profile when it differs from this.
US_KEYMAP = {
    2: ("1", "!"), 3: ("2", "@"), 4: ("3", "#"), 5: ("4", "$"), 6: ("5", "%"),
    7: ("6", "^"), 8: ("7", "&"), 9: ("8", "*"), 10: ("9", "("), 11: ("0", ")"),
    12: ("-", "_"), 13: ("=", "+"),
    16: ("q", "Q"), 17: ("w", "W"), 18: ("e", "E"), 19: ("r", "R"),
    20: ("t", "T"), 21: ("y", "Y"), 22: ("u", "U"), 23: ("i", "I"),
    24: ("o", "O"), 25: ("p", "P"), 26: ("[", "{"), 27: ("]", "}"),
    30: ("a", "A"), 31: ("s", "S"), 32: ("d", "D"), 33: ("f", "F"),
    34: ("g", "G"), 35: ("h", "H"), 36: ("j", "J"), 37: ("k", "K"),
    38: ("l", "L"), 39: (";", ":"), 40: ("'", '"'), 41: ("`", "~"),
    43: ("\\", "|"),
    44: ("z", "Z"), 45: ("x", "X"), 46: ("c", "C"), 47: ("v", "V"),
    48: ("b", "B"), 49: ("n", "N"), 50: ("m", "M"), 51: (",", "<"),
    52: (".", ">"), 53: ("/", "?"),
    57: (" ", " "),
}

# Android keycode name -> the character it stands for unmodified.  Only the
# names a QWERTY-ish phone keyboard actually uses.
ANDROID_KEYCODE_CHAR = dict(
    [(chr(c), chr(c + 32)) for c in range(ord("A"), ord("Z") + 1)]
    + [(str(d), str(d)) for d in range(10)]
    + [("SPACE", " ")]
)

XKB_KEY_TO_SCANCODE = {
    "TLDE": 41, "AE01": 2, "AE02": 3, "AE03": 4, "AE04": 5, "AE05": 6,
    "AE06": 7, "AE07": 8, "AE08": 9, "AE09": 10, "AE10": 11, "AE11": 12,
    "AE12": 13, "BKSP": 14,
    "TAB": 15, "AD01": 16, "AD02": 17, "AD03": 18, "AD04": 19, "AD05": 20,
    "AD06": 21, "AD07": 22, "AD08": 23, "AD09": 24, "AD10": 25, "AD11": 26,
    "AD12": 27, "BKSL": 43, "RTRN": 28,
    "CAPS": 58, "AC01": 30, "AC02": 31, "AC03": 32, "AC04": 33, "AC05": 34,
    "AC06": 35, "AC07": 36, "AC08": 37, "AC09": 38, "AC10": 39, "AC11": 40,
    "LFSH": 42, "AB01": 44, "AB02": 45, "AB03": 46, "AB04": 47, "AB05": 48,
    "AB06": 49, "AB07": 50, "AB08": 51, "AB09": 52, "AB10": 53, "RTSH": 54,
    "LCTL": 29, "LALT": 56, "SPCE": 57, "RALT": 100, "RCTL": 97,
}

# xkb keysym names this converter understands.  Anything else is an error
# rather than a guess -- a keyboard layout that is quietly wrong is worse than
# one that is missing.
XKB_KEYSYM_CHAR = {
    "space": " ", "exclam": "!", "quotedbl": '"', "numbersign": "#",
    "dollar": "$", "percent": "%", "ampersand": "&", "apostrophe": "'",
    "parenleft": "(", "parenright": ")", "asterisk": "*", "plus": "+",
    "comma": ",", "minus": "-", "period": ".", "slash": "/",
    "colon": ":", "semicolon": ";", "less": "<", "equal": "=", "greater": ">",
    "question": "?", "at": "@", "bracketleft": "[", "backslash": "\\",
    "bracketright": "]", "asciicircum": "^", "underscore": "_", "grave": "`",
    "braceleft": "{", "bar": "|", "braceright": "}", "asciitilde": "~",
    "multiply": "×", "division": "÷", "plusminus": "±",
    "sterling": "£", "yen": "¥", "EuroSign": "€",
    "cent": "¢", "currency": "¤", "degree": "°",
    "exclamdown": "¡", "questiondown": "¿",
    "guillemotleft": "«", "guillemotright": "»",
    "periodcentered": "·", "section": "§",
}
for _c in range(ord("a"), ord("z") + 1):
    XKB_KEYSYM_CHAR[chr(_c)] = chr(_c)
    XKB_KEYSYM_CHAR[chr(_c).upper()] = chr(_c).upper()
for _d in range(10):
    XKB_KEYSYM_CHAR[str(_d)] = str(_d)


def unescape_kcm(literal):
    """Decode one .kcm character literal: 'x', '\\u00F7', '\\'', '\\\\'."""
    body = literal[1:-1]
    if body.startswith("\\u"):
        return chr(int(body[2:6], 16))
    if body.startswith("\\"):
        return {"n": "\n", "t": "\t", "\\": "\\", "'": "'", '"': '"'}.get(
            body[1], body[1]
        )
    return body


def parse_kl(path):
    """scancode -> Android keycode name."""
    out = {}
    for line in open(path, encoding="utf-8"):
        m = re.match(r"\s*key\s+(\d+)\s+(\S+)", line)
        if m:
            out[int(m.group(1))] = m.group(2)
    return out


def strip_kcm_comment(line):
    """Drop a trailing # comment -- but '#' is also a character literal here."""
    in_quote = False
    for i, c in enumerate(line):
        if c == "\\" and in_quote:
            continue
        if c == "'" and (i == 0 or line[i - 1] != "\\"):
            in_quote = not in_quote
        elif c == "#" and not in_quote:
            return line[:i]
    return line


def parse_kcm(path):
    """Android keycode name -> {level: character}.

    Multi-modifier lines ('lalt+lshift') are skipped: they describe vendor
    easter eggs, not a level this profile format has.
    """
    blocks = {}
    current = None
    for line in open(path, encoding="utf-8"):
        line = strip_kcm_comment(line).strip()
        if not line:
            continue
        m = re.match(r"key\s+(\S+)\s*\{", line)
        if m:
            current = blocks.setdefault(m.group(1), {})
            continue
        if line == "}":
            current = None
            continue
        if current is None:
            continue
        m = re.match(r"([A-Za-z0-9_,+\s]+?)\s*:\s*(.+)$", line)
        if not m:
            continue
        value = m.group(2).strip()
        if not (value.startswith("'") and value.endswith("'")):
            continue  # 'none', 'fallback FOO', string literals
        char = unescape_kcm(value)
        for name in (n.strip() for n in m.group(1).split(",")):
            if "+" in name:
                continue
            current[name] = char
    return blocks


def levels_from_android(kl_path, kcm_path):
    kl = parse_kl(kl_path)
    kcm = parse_kcm(kcm_path)
    levels = {"base": {}, "shift": {}, "alt": {}, "sym": {}}

    for scancode, keycode in sorted(kl.items()):
        block = kcm.get(keycode)
        if not block:
            continue
        natural = US_KEYMAP.get(scancode)

        base = block.get("base")
        if base and base.isprintable() and (natural is None or base != natural[0]):
            levels["base"][scancode] = base

        shift = block.get("shift")
        if shift and shift.isprintable() and (natural is None or shift != natural[1]):
            levels["shift"][scancode] = shift

        alt = block.get("alt") or block.get("lalt")
        if alt:
            levels["alt"][scancode] = alt

        sym = block.get("sym")
        if sym:
            levels["sym"][scancode] = sym

    return levels


def parse_xkb(path):
    """<AD01> { [ q, Q, colon, colon ] } -> scancode -> [level1..level4]."""
    text = open(path, encoding="utf-8").read()
    out = {}
    for m in re.finditer(r"key\s*<(\w+)>\s*\{\s*\[([^\]]*)\]", text):
        name = m.group(1)
        if name not in XKB_KEY_TO_SCANCODE:
            continue
        syms = [s.strip() for s in m.group(2).split(",")]
        out[XKB_KEY_TO_SCANCODE[name]] = syms
    return out


def levels_from_xkb(path):
    levels = {"base": {}, "shift": {}, "alt": {}, "sym": {}}
    for scancode, syms in sorted(parse_xkb(path).items()):
        natural = US_KEYMAP.get(scancode)
        for index, level in ((0, "base"), (1, "shift"), (2, "sym")):
            if index >= len(syms):
                continue
            name = syms[index]
            if name in ("NoSymbol", "VoidSymbol", ""):
                continue
            if name not in XKB_KEYSYM_CHAR:
                if index == 2:
                    sys.exit(
                        "unknown keysym %r on <%s>: add it to XKB_KEYSYM_CHAR, "
                        "or drop the key" % (name, scancode)
                    )
                continue
            char = XKB_KEYSYM_CHAR[name]
            if natural and index < 2 and char == natural[index]:
                continue
            levels[level][scancode] = char
    return levels


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--name", required=True)
    p.add_argument("--description", required=True)
    p.add_argument("--device-name", action="append", default=[], required=True,
                   help="input device name to match, as /proc/bus/input/devices "
                        "reports it; repeatable")
    p.add_argument("--alt-key", type=int, action="append", default=[],
                   help="evdev scancode of an Alt (first alternate level) key")
    p.add_argument("--sym-key", type=int, action="append", default=[],
                   help="evdev scancode of a Sym (second alternate level) key")
    p.add_argument("--no-lock", action="store_true",
                   help="do not lock the level on a double tap")
    p.add_argument("--kl")
    p.add_argument("--kcm")
    p.add_argument("--xkb")
    args = p.parse_args()

    if args.xkb:
        levels = levels_from_xkb(args.xkb)
    elif args.kl and args.kcm:
        levels = levels_from_android(args.kl, args.kcm)
    else:
        p.error("give either --xkb, or both --kl and --kcm")

    profile = {
        "name": args.name,
        "description": args.description,
        "match": {"inputDeviceNames": args.device_name},
        "altKeys": args.alt_key,
        "symKeys": args.sym_key,
        "lockOnDoubleTap": not args.no_lock,
        "levels": {
            level: {str(k): v for k, v in sorted(chars.items())}
            for level, chars in levels.items()
            if chars
        },
    }
    print(json.dumps(profile, indent=4, ensure_ascii=False))


if __name__ == "__main__":
    main()
