"""The Leon mod's exe icon (src/win32/leon.ico, the resource the game window and Explorer show): the mod's cover
lettering - white "LEON" over a green "ZOMBIES" plate, on dark - with RE4's red laser dot in the O. Drawn here, from
nothing of either game. Small sizes (48 and under) are a bare "L" and the dot, the lettering being unreadable there.

usage: python -I make_icon.py <out.ico> [preview.png]   (needs Pillow and Windows' Impact font)"""
import math
import os
import random
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

FONT = os.path.join(os.environ.get('WINDIR', r'C:\Windows'), 'Fonts', 'impact.ttf')
WHITE = (242, 242, 238, 255)
GREEN = (82, 214, 74, 255)
RED = (255, 38, 24)


def background(size):
    """A dark rounded square, faintly lit from the top."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    grad = Image.new('RGBA', (size, size))
    px = grad.load()
    for y in range(size):
        t = y / (size - 1)
        c = (int(30 - 16 * t), int(33 - 17 * t), int(37 - 19 * t), 255)
        for x in range(size):
            px[x, y] = c
    mask = Image.new('L', (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, size - 1, size - 1), radius=size * 0.16, fill=255)
    img.paste(grad, (0, 0), mask)
    return img, mask


def laser_dot(img, cx, cy, r):
    """RE4's dot: a hot core with a red bloom."""
    glow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(glow)
    d.ellipse((cx - r * 3, cy - r * 3, cx + r * 3, cy + r * 3), fill=RED + (150,))
    glow = glow.filter(ImageFilter.GaussianBlur(r * 1.4))
    img.alpha_composite(glow)
    d = ImageDraw.Draw(img)
    d.ellipse((cx - r, cy - r, cx + r, cy + r), fill=RED + (255,))
    k = r * 0.45
    d.ellipse((cx - k, cy - k, cx + k, cy + k), fill=(255, 190, 170, 255))


def scratches(layer, box, count, seed, width):
    """Thin worn strokes cut into the lettering, as the cover's (only where the letters are)."""
    rnd = random.Random(seed)
    cut = Image.new('L', layer.size, 0)
    d = ImageDraw.Draw(cut)
    x0, y0, x1, y1 = box
    for _ in range(count):
        x = rnd.uniform(x0, x1)
        y = rnd.uniform(y0, y1)
        a = rnd.uniform(-0.45, 0.45)
        n = rnd.uniform(0.04, 0.12) * (x1 - x0)
        d.line((x, y, x + math.cos(a) * n, y + math.sin(a) * n), fill=255, width=width)
    layer.putalpha(ImageChops.subtract(layer.split()[3], cut))


def large(size=256):
    img, mask = background(size)
    s = size / 256.0
    text = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(text)
    font = ImageFont.truetype(FONT, int(112 * s))
    box = d.textbbox((0, 0), 'LEON', font=font)
    w, h = box[2] - box[0], box[3] - box[1]
    tx, ty = (size - w) / 2 - box[0], 34 * s - box[1]
    d.text((tx, ty), 'LEON', font=font, fill=WHITE)
    scratches(text, (tx + box[0], ty + box[1], tx + box[2], ty + box[3]), 12, 7, max(1, int(2 * s)))
    shadow = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    shadow.paste((0, 0, 0, 170), (0, 0), text.split()[3])
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(3 * s)), (0, int(3 * s)))
    img.alpha_composite(text)
    # the laser dot inside the O
    o = d.textbbox((tx, ty), 'LEO', font=font)[2] - (d.textbbox((0, 0), 'O', font=font)[2] - d.textbbox((0, 0), 'O', font=font)[0]) / 2
    laser_dot(img, o - 1 * s, ty + box[1] + h / 2, 7 * s)
    # the green plate
    d = ImageDraw.Draw(img)
    pfont = ImageFont.truetype(FONT, int(40 * s))
    pb = d.textbbox((0, 0), 'ZOMBIES', font=pfont)
    pw, ph = pb[2] - pb[0], pb[3] - pb[1]
    px0, py0 = (size - pw) / 2 - 14 * s, 168 * s
    d.rounded_rectangle((px0, py0, px0 + pw + 28 * s, py0 + ph + 22 * s), radius=6 * s, fill=(14, 20, 15, 255),
                        outline=(78, 84, 78, 255), width=max(1, int(2 * s)))
    d.text((px0 + 14 * s - pb[0], py0 + 11 * s - pb[1]), 'ZOMBIES', font=pfont, fill=GREEN)
    out = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    out.paste(img, (0, 0), mask)
    return out


def small(size):
    """48 and under: an "L" and the dot (drawn at 4x, then reduced)."""
    big = size * 4
    img, mask = background(big)
    d = ImageDraw.Draw(img)
    font = ImageFont.truetype(FONT, int(big * 0.86))
    box = d.textbbox((0, 0), 'L', font=font)
    w, h = box[2] - box[0], box[3] - box[1]
    tx, ty = (big - w) / 2 - box[0] - big * 0.08, (big - h) / 2 - box[1]
    d.text((tx, ty), 'L', font=font, fill=WHITE)
    laser_dot(img, tx + box[2] + big * 0.06, ty + box[3] - big * 0.18, big * 0.085)
    out = Image.new('RGBA', (big, big), (0, 0, 0, 0))
    out.paste(img, (0, 0), mask)
    return out.resize((size, size), Image.LANCZOS)


def main():
    sizes = [16, 20, 24, 32, 40, 48, 64, 96, 128, 256]
    big = large(256)
    images = {}
    for n in sizes:
        images[n] = small(n) if n <= 48 else (big if n == 256 else large(n * 2).resize((n, n), Image.LANCZOS))
    # Pillow writes the ICO from the largest image, resampling it for the others: each size goes in as its own frame
    big.save(sys.argv[1], format='ICO', sizes=[(n, n) for n in sizes],
             append_images=[images[n] for n in sizes if n != 256])
    if len(sys.argv) > 2:
        sheet = Image.new('RGBA', (sum(sizes) + 10 * len(sizes) + 10, 276), (200, 200, 200, 255))
        x = 10
        for n in sizes:
            sheet.alpha_composite(images[n], (x, 266 - n))
            x += n + 10
        sheet.save(sys.argv[2])


if __name__ == '__main__':
    main()
