#!/usr/bin/env python3
"""Draw the phone keyboard's key and deck artwork in the style of the Palm Pre
hardware keyboard: glossy black caps sitting in slightly larger dark recesses on
a charcoal deck.

Keys are 48x48 nine-patches with 23px borders (CharKey's phone BorderImage), so
everything that has a shape - the recess, the cap's corners, the dome's top glow
and bottom rim - lives in the outer 23px, and the 2px centre is flat cap colour
that stretches. Drawn at 8x and scaled down for antialiasing.

    python3 tools/gen_pre_keys.py qml/images/phone
"""
import sys
from PIL import Image, ImageDraw, ImageFilter

SS = 8
SIZE = 48


def lerp(a, b, t):
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(len(a)))


def vgrad(size, stops):
    """Vertical gradient image from (pos 0..1, rgba) stops."""
    w, h = size
    im = Image.new("RGBA", size)
    px = im.load()
    for y in range(h):
        t = y / (h - 1)
        for i in range(len(stops) - 1):
            p0, c0 = stops[i]
            p1, c1 = stops[i + 1]
            if p0 <= t <= p1:
                c = lerp(c0, c1, (t - p0) / (p1 - p0) if p1 > p0 else 0)
                break
        for x in range(w):
            px[x, y] = c
    return im


def rrect_mask(size, box, radius):
    m = Image.new("L", size, 0)
    ImageDraw.Draw(m).rounded_rectangle(box, radius=radius, fill=255)
    return m


def key(pressed=False):
    S = SIZE * SS
    out = Image.new("RGBA", (S, S), (0, 0, 0, 0))

    # The recess the cap sits in: a dark rounded well a little larger than it.
    well_pad, well_r = 3 * SS, 13 * SS
    well = rrect_mask((S, S), (well_pad, well_pad, S - well_pad - 1, S - well_pad - 1), well_r)
    well = well.filter(ImageFilter.GaussianBlur(SS * 0.6))
    out.paste((8, 9, 10, 255), (0, 0), well)

    # The cap: near-black, a soft glow under the top edge, darkest through the
    # middle and lifting a little towards the bottom rim.
    cap_pad, cap_r = 5 * SS, 11 * SS
    box = (cap_pad, cap_pad, S - cap_pad - 1, S - cap_pad - 1)
    lift = 26 if pressed else 0
    def c(r, g, b):
        return (min(255, r + lift), min(255, g + lift), min(255, b + lift), 255)
    body = vgrad((S, S), [
        (0.00, c(64, 71, 76)),
        (0.18, c(40, 46, 51)),
        (0.40, c(22, 27, 30)),
        (0.60, c(20, 24, 27)),
        (0.84, c(25, 29, 32)),
        (1.00, c(38, 42, 46)),
    ])
    capm = rrect_mask((S, S), box, cap_r)
    out.paste(body, (0, 0), capm)

    # Specular highlight: a thin bright line just inside the top edge, fading
    # out towards the corners.
    hl = Image.new("L", (S, S), 0)
    d = ImageDraw.Draw(hl)
    y = cap_pad + int(1.4 * SS)
    d.rounded_rectangle((cap_pad + 3 * SS, y, S - cap_pad - 3 * SS, y + int(1.1 * SS)),
                        radius=SS, fill=235 if not pressed else 150)
    hl = hl.filter(ImageFilter.GaussianBlur(SS * 0.5))
    hl = Image.composite(hl, Image.new("L", (S, S), 0), capm)
    out.paste((220, 228, 232, 255), (0, 0), hl)

    # A faint lighter rim along the bottom edge of the cap.
    rim = Image.new("L", (S, S), 0)
    ImageDraw.Draw(rim).rounded_rectangle(box, radius=cap_r, outline=70, width=int(0.9 * SS))
    rim_mask = vgrad((S, S), [(0, (0, 0, 0, 0)), (0.7, (0, 0, 0, 0)), (1, (255, 255, 255, 255))]).split()[3]
    rim = Image.composite(rim, Image.new("L", (S, S), 0), rim_mask)
    out.paste((120, 128, 134, 255), (0, 0), rim)

    return out.resize((SIZE, SIZE), Image.LANCZOS)


def deck():
    # 3px wide, tiled horizontally and stretched to the panel's height.
    return vgrad((3, 200), [(0, (74, 81, 86, 255)), (1, (58, 64, 68, 255))])


def border_top():
    im = Image.new("RGBA", (2, 6))
    for y, col in enumerate([(14, 16, 17), (14, 16, 17), (30, 34, 36), (30, 34, 36), (86, 94, 99), (86, 94, 99)]):
        for x in range(2):
            im.putpixel((x, y), col + (255,))
    return im


def border_bottom():
    im = Image.new("RGBA", (2, 5))
    for y in range(5):
        for x in range(2):
            im.putpixel((x, y), (58, 64, 68, 255))
    return im


if __name__ == "__main__":
    d = sys.argv[1]
    up, down = key(False), key(True)
    for name in ("white", "black", "grey", "shift", "shift_on"):
        up.save(f"{d}/key_bg_{name}.png")
        down.save(f"{d}/key_bg_{name}_active.png")
    deck().save(f"{d}/keyboard-bg.png")
    border_top().save(f"{d}/border_top.png")
    border_bottom().save(f"{d}/border_bottom.png")
    # The function-key icons are white but drawn at ~80% alpha, which reads as
    # grey on a black cap; the Pre's printed arrows are solid white.
    for name in ("shift", "backspace"):
        im = Image.open(f"{d}/{name}.png").convert("RGBA")
        a = im.split()[3]
        peak = max(a.getdata()) or 255
        im.putalpha(a.point(lambda v: min(255, v * 255 // peak)))
        im.save(f"{d}/{name}.png")
