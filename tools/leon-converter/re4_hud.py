"""RE4's player HUD images -> the runtime HUD file (.rlh) the BO1 engine draws the mod's HUD from
(gfx_d3d/r_runtime_xmodel.cpp R_RuntimeXModel_HudMaterial, cgame_mp/cg_re4hud_mp.cpp).

RE4 draws its cockpit (game/cockpit.cpp: LifeMeter, BulletInfo) from the shared texture pack 0x07000000 (its TPLs in
BIO4/Etc/core.udas all point there): the life meter's stone frame, a ring whose segmented channel the life fills
(image 268), RE4's italic outlined digits 0-9 (221-230: the loaded rounds), the "EMPTY" badge (262), a plain disc (309:
the heart monitor's screen inside the ring) and the characters' name plates (302 Leon, 299 Ada, 303 HUNK; Luis has
none: his is made of the plates' own letters, luis_plate). The layout is the engine's; only the images are taken.

File (little-endian): 'RLH1', u32 image count, then per image the .rlm texture record (leon_rlm.py): char name[32],
u16 width, u16 height, u8 mip count, u8 alpha masked (0: blended), u16 0, every mip level's BGRA8 pixels, largest first.

usage: python -I re4_hud.py <07000000.pack> <out.rlh>"""
import io
import math
import os
import struct
import sys
from collections import deque

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageStat

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from uhd_pack import images  # noqa: E402
from leon_rlm import capped, mips_bgra  # noqa: E402

# (name the engine asks for, image in pack 0x07000000)
HUD_IMAGES = ([('frame', 268), ('empty', 262), ('disc', 309), ('name_leon', 302), ('name_ada', 299),
               ('name_hunk', 303)]
              + [(f'digit{n}', 221 + n) for n in range(10)])
# the largest side kept: the HUD is drawn at most ~400 px across at 4K
MAX_SIDE = 1024

# Luis's plate (luis_plate) from the plates of Leon (302: L e o n), Krauser (271, the Mercenaries': K r a u s e r) and
# Ashley (306: A s h l e y) - their letters' places in them (left to right), and each one's e and baseline (a letter
# standing flat on it)
LEON, KRAUSER, ASHLEY = 302, 271, 306
LETTERS = {LEON: 4, KRAUSER: 7, ASHLEY: 6}
E_AT = {LEON: 1, KRAUSER: 5, ASHLEY: 4}
BASE_AT = {LEON: 0, KRAUSER: 0, ASHLEY: 2}
# the plates' outline round the lettering: 6 pixels out from the fill on Leon's 128-pixel-high plate (6 on them all)
OUTLINE = 6.0 / 128


def fill_of(img):
    """A plate's lettering, as RE4 draws it: a flat light fill inside a dark outline drawn round the whole word (the
    outlines of letters near each other run together). Its fill as a soft mask (the outline and the backdrop out:
    lightness from the outline's to the fill's), and the fill's and the outline's colours."""
    rgb = img.convert('RGB')
    lum = rgb.convert('L')
    alpha = img.split()[3]
    solid = alpha.point(lambda a: 255 if a > 250 else 0)
    hist = lum.histogram(mask=solid)
    total = sum(hist)

    def share(p):
        seen = 0
        for value, n in enumerate(hist):
            seen += n
            if seen >= total * p:
                return value
        return 255
    dark, light = share(0.10), share(0.90)
    fill = ImageChops.multiply(lum.point([max(0, min(255, (v - dark) * 255 // max(1, light - dark)))
                                          for v in range(256)]), alpha)
    fill_px = ImageChops.multiply(fill.point(lambda v: 255 if v > 240 else 0), solid)
    line_px = ImageChops.multiply(lum.point(lambda v: 255 if v <= dark + 8 else 0), solid)
    return (fill, tuple(int(c) for c in ImageStat.Stat(rgb, fill_px).median),
            tuple(int(c) for c in ImageStat.Stat(rgb, line_px).median))


def letters(fill, count):
    """The fill's letters left to right (its separate parts: the fills stand apart inside the outline), each as
    (box, its own soft mask in that box). A plate with another count of them (another pack's) gives none."""
    w, h = fill.size
    solid = fill.point(lambda v: 255 if v >= 128 else 0)
    px = solid.load()
    label = [-1] * (w * h)
    parts = []
    for y in range(h):
        for x in range(w):
            if label[y * w + x] >= 0 or not px[x, y]:
                continue
            n = len(parts)
            label[y * w + x] = n
            queue, box, size = deque([(x, y)]), [x, y, x + 1, y + 1], 0
            while queue:
                cx, cy = queue.popleft()
                size += 1
                box = [min(box[0], cx), min(box[1], cy), max(box[2], cx + 1), max(box[3], cy + 1)]
                for nx, ny in ((cx - 1, cy), (cx + 1, cy), (cx, cy - 1), (cx, cy + 1),
                               (cx - 1, cy - 1), (cx + 1, cy - 1), (cx - 1, cy + 1), (cx + 1, cy + 1)):
                    if 0 <= nx < w and 0 <= ny < h and label[ny * w + nx] < 0 and px[nx, ny]:
                        label[ny * w + nx] = n
                        queue.append((nx, ny))
            parts.append((box, size))
    keep = sorted((i for i, (_box, size) in enumerate(parts) if size >= 40), key=lambda i: parts[i][0][0])
    if len(keep) != count:
        raise ValueError(f'{len(keep)} letters, not {count}')
    out = []
    for i in keep:
        box = parts[i][0]
        own = Image.new('L', (w, h))
        own.putdata([255 if v == i else 0 for v in label])
        # its soft edge: the fill within 2 pixels of it
        own = ImageChops.multiply(own.filter(ImageFilter.MaxFilter(5)), fill)
        grown = (max(0, box[0] - 2), max(0, box[1] - 2), min(w, box[2] + 2), min(h, box[3] + 2))
        out.append((grown, own.crop(grown)))
    return out


def lines(mask):
    """A mask's rows' inked spans: (left, right) or None, the soft edge counted from half on."""
    solid = mask.point(lambda v: 255 if v >= 128 else 0)
    spans = []
    for y in range(mask.size[1]):
        box = solid.crop((0, y, mask.size[0], y + 1)).getbbox()
        spans.append((box[0], box[2]) if box else None)
    return spans


def after(left, left_at, right, right_at_y, gap):
    """Where the right letter goes (its x) for the nearest its fill comes to the left one's, row by row, to be gap.
    left_at: (x, y) of the left one's mask, right_at_y: the right one's y."""
    a, b = lines(left), lines(right)
    need = None
    for y, span in enumerate(a):
        ry = int(round(y + left_at[1] - right_at_y))
        if span and 0 <= ry < len(b) and b[ry]:
            x = left_at[0] + span[1] + gap - b[ry][0]
            need = x if need is None else max(need, x)
    return need if need is not None else left_at[0] + left.size[0] + gap


def spacing(parts):
    """A plate's own gaps: the nearest each letter's fill comes to the next's (as after)."""
    gaps = []
    for (box_a, a), (box_b, b) in zip(parts, parts[1:]):
        x = after(a, box_a[:2], b, box_b[1], 0)
        gaps.append(box_b[0] - x)
    return gaps


def dilate(mask, radius, ss=4):
    """The mask grown by radius (pixels) round, as an outline drawn round it: worked at ss times the size, with room
    round it (the moves wrap round the image's edges), smoothed back."""
    w, h = mask.size
    pad = int(math.ceil(radius)) + 1
    room = Image.new('L', (w + 2 * pad, h + 2 * pad))
    room.paste(mask, (pad, pad))
    big = room.resize((room.size[0] * ss, room.size[1] * ss), Image.BICUBIC).point(lambda v: 255 if v >= 128 else 0)
    grown = big.copy()
    r = radius * ss
    for dy in range(-int(r), int(r) + 1):
        reach = int(math.sqrt(max(0.0, r * r - dy * dy)))
        # this row of the disc: the mask moved dy down, spread reach either way (in doubling steps)
        row = ImageChops.offset(big, 0, dy)
        spread, done = row, 0
        while done < reach:
            step = min(done + 1, reach - done)
            spread = ImageChops.lighter(spread, ImageChops.offset(spread, step, 0))
            spread = ImageChops.lighter(spread, ImageChops.offset(spread, -step, 0))
            done += step
        grown = ImageChops.lighter(grown, spread)
    return grown.resize(room.size, Image.BOX).crop((pad, pad, pad + w, pad + h))


def shifted(mask, dx, dy, size):
    """The mask moved by (dx, dy) (fractions too) onto a clear image of size."""
    out = Image.new('L', size)
    out.paste(mask, (0, 0))
    return out.transform(size, Image.AFFINE, (1, 0, -dx, 0, 1, -dy), Image.BICUBIC)


def luis_plate(pack):
    """Luis has no name plate in RE4 (he never plays): his is made of RE4's plates' own letters, in their font, fill,
    outline and colours - the L of Leon's, the u and the s of Krauser's, and an i from the l of Ashley's, its stem cut
    to the x-height under a round dot. The plates are each stretched their own way (Leon's letters wider than
    Krauser's for their height): the letters are brought to Leon's plate's (by their e's), set on its baseline as far
    apart as its own, and the outline drawn round them again (OUTLINE: the plates' own, measured on them)."""
    plates = {i: Image.open(io.BytesIO(pack[i])).convert('RGBA') for i in (LEON, KRAUSER, ASHLEY)}
    leon = plates[LEON]
    fills = {i: fill_of(img) for i, img in plates.items()}
    parts = {i: letters(fills[i][0], LETTERS[i]) for i in plates}
    e = {i: parts[i][E_AT[i]][1].getbbox() for i in plates}
    base = {i: parts[i][BASE_AT[i]][0][1] + parts[i][BASE_AT[i]][1].getbbox()[3] for i in plates}
    target_base = base[LEON]

    def brought(i, n):
        """Letter n of plate i at Leon's plate's scale: (mask, its y on Leon's plate)"""
        box, mask = parts[i][n]
        sx = (e[LEON][2] - e[LEON][0]) / (e[i][2] - e[i][0])
        sy = (e[LEON][3] - e[LEON][1]) / (e[i][3] - e[i][1])
        size = (max(1, round(mask.size[0] * sx)), max(1, round(mask.size[1] * sy)))
        return mask.resize(size, Image.BICUBIC), target_base + (box[1] - base[i]) * sy

    l_box, l_mask = parts[LEON][0]
    u, u_y = brought(KRAUSER, 3)
    s, s_y = brought(KRAUSER, 4)
    ell, ell_y = brought(ASHLEY, 3)

    # the i: the l's top (its flag, and the stem under it) let down along the stem's slant to the x-height (the u's
    # top), over the l's bottom (its foot, and the stem over it); a round dot over it on the same slant
    spans = lines(ell)
    inked = [y for y, span in enumerate(spans) if span]
    top, bottom = inked[0], inked[-1] + 1
    tall = bottom - top
    stem = [(y, (spans[y][0] + spans[y][1]) / 2, spans[y][1] - spans[y][0])
            for y in range(top + int(tall * 0.35), top + int(tall * 0.70)) if spans[y]]
    my = sum(y for y, _c, _w in stem) / len(stem)
    mc = sum(c for _y, c, _w in stem) / len(stem)
    slant = (sum((y - my) * (c - mc) for y, c, _w in stem) / sum((y - my) ** 2 for y, _c, _w in stem))
    thick = sorted(w for _y, _c, w in stem)[len(stem) // 2]
    x_height = target_base - u_y - u.getbbox()[1]
    drop = tall - x_height
    flag_rows = int(tall * 0.42)
    foot_rows = int(x_height - flag_rows + thick * 0.5)
    margin = int(thick * 2)
    size = (ell.size[0] + 2 * margin, ell.size[1] + 2 * margin)
    flag = ell.crop((0, 0, ell.size[0], top + flag_rows))
    foot = ell.crop((0, bottom - foot_rows, ell.size[0], ell.size[1]))
    i_mask = ImageChops.lighter(shifted(foot, margin, margin + bottom - foot_rows, size),
                                shifted(flag, margin + slant * drop, margin + drop, size))
    # the dot: a little wider than the stem, half the stem's width over it
    dot, over = thick * 1.15, thick * 0.5
    dot_y = margin + top + drop - over - dot / 2
    dot_x = margin + mc + slant * (dot_y - (margin + my))
    ss = 4
    disc = Image.new('L', (size[0] * ss, size[1] * ss))
    ImageDraw.Draw(disc).ellipse(((dot_x - dot / 2) * ss, (dot_y - dot / 2) * ss,
                                  (dot_x + dot / 2) * ss, (dot_y + dot / 2) * ss), fill=255)
    i_mask = ImageChops.lighter(i_mask, disc.resize(size, Image.BOX))
    i_box = i_mask.getbbox()
    i_mask, i_y = i_mask.crop(i_box), ell_y + i_box[1] - margin

    # set on Leon's baseline: the L where it is on his plate, each next letter as near as his letters come (the L to
    # the u as his L to his e; the others the median of his plate's and Krauser's gaps)
    own = spacing(parts[LEON])
    gaps = sorted(own[1:] + [g * (e[LEON][2] - e[LEON][0]) / (e[KRAUSER][2] - e[KRAUSER][0])
                             for g in spacing(parts[KRAUSER])])
    gap = gaps[len(gaps) // 2]
    placed = [(l_mask, l_box[0], l_box[1])]
    for mask, y, g in ((u, u_y, own[0]), (i_mask, i_y, gap), (s, s_y, gap)):
        prev, px_, py_ = placed[-1]
        placed.append((mask, after(prev, (px_, py_), mask, y, g), y))

    # the plate: Leon's height, as wide as the lettering with his plate's margin at the right; the outline round
    # the fills, the fills over it
    right = max(x + mask.getbbox()[2] for mask, x, _y in placed)
    radius = OUTLINE * leon.size[1]
    leon_right = leon.size[0] - leon.split()[3].point(lambda a: 255 if a > 40 else 0).getbbox()[2]
    w = int(math.ceil(right + radius + leon_right))
    lettering = Image.new('L', (w, leon.size[1]))
    for mask, x, y in placed:
        lettering = ImageChops.lighter(lettering, shifted(mask, x, y, lettering.size))
    _fill, fill_colour, line_colour = fills[LEON]
    plate = Image.new('RGBA', lettering.size, line_colour + (0,))
    plate.putalpha(dilate(lettering, radius))
    face = Image.new('RGBA', lettering.size, fill_colour + (0,))
    face.putalpha(lettering)
    return Image.alpha_composite(plate, face)


def build(pack_path, out_path):
    data = open(pack_path, 'rb').read()
    pack = {i: d for i, _k, d in images(data)}
    out = bytearray(b'RLH1' + struct.pack('<I', 0))
    count = 0
    made = []
    if all(i in pack for i in (LEON, KRAUSER, ASHLEY)):
        try:
            made.append(('name_luis', luis_plate(pack)))
        except Exception as e:  # noqa: BLE001 - another pack's plates: the rest of the HUD is built without it
            print(f'name_luis: not made ({e!r}): Luis keeps the HUD\'s font')
    for name, index in HUD_IMAGES + made:
        if isinstance(index, int) and index not in pack:
            print(f'{name}: no image {index} in the pack, left out')
            continue
        img = index if not isinstance(index, int) else Image.open(io.BytesIO(pack[index])).convert('RGBA')
        img = capped(img, MAX_SIDE)
        levels = mips_bgra(img)
        out += name.encode()[:31].ljust(32, b'\0')
        out += struct.pack('<HHBBH', img.size[0], img.size[1], len(levels), 0, 0)
        for level in levels:
            out += level
        count += 1
    struct.pack_into('<I', out, 4, count)
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(out)
    print(f'wrote {out_path}: {count} images, {len(out)} bytes')


if __name__ == '__main__':
    build(sys.argv[1], sys.argv[2])
