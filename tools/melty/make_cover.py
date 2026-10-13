"""The Leon mod's Melty cover: an in-game shot of Leon (a headless screenshot, tools/headless.ps1 -ShotsAtMs) graded
dark and smoky, with the title over it - "LEON" in white and "ZOMBIES KENNEDY" in a worn green banner. Only the
player's own render and the mod's name: no publisher artwork or logo.

usage: python -I make_cover.py <shot.jpg> <out.png> [--focus x,y]   (focus: Leon's place in the shot, 0..1 each)"""
import random
import sys

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageFont

W, H = 1920, 1080
FONTS = r'C:\Windows\Fonts'


def fit(img, focus):
    """Crop to 16:9 around the focus point and scale to the cover size."""
    iw, ih = img.size
    scale = max(W / iw, H / ih) * 1.15  # a little closer than the shot
    img = img.resize((round(iw * scale), round(ih * scale)), Image.LANCZOS)
    iw, ih = img.size
    x = min(max(0, round(focus[0] * iw - W / 2)), iw - W)
    y = min(max(0, round(focus[1] * ih - H / 2)), ih - H)
    return img.crop((x, y, x + W, y + H))


def smoke(seed):
    """Soft drifting smoke: random noise blurred at a few scales, brighter low in the frame."""
    rnd = random.Random(seed)
    layer = Image.new('L', (W, H), 0)
    for size, blur, weight in ((24, 60, 120), (60, 25, 70), (150, 10, 40)):
        small = Image.new('L', (W // size + 2, H // size + 2))
        small.putdata([rnd.randrange(256) for _ in range(small.size[0] * small.size[1])])
        big = small.resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(blur))
        layer = ImageChops.add(layer, big.point(lambda v, w=weight: v * w // 255))
    ramp = Image.linear_gradient('L').resize((W, H)).point(lambda v: 90 + v * 165 // 255)
    return ImageChops.multiply(layer, ramp)


def vignette():
    mask = Image.new('L', (W, H), 0)
    ImageDraw.Draw(mask).ellipse((-W * 0.15, -H * 0.25, W * 1.15, H * 1.2), fill=255)
    return mask.filter(ImageFilter.GaussianBlur(220))


def worn(mask, seed, amount=0.22):
    """Scuff a text mask: specks and scratches knocked out of it, as the banner's worn print."""
    rnd = random.Random(seed)
    wear = Image.new('L', mask.size, 255)
    d = ImageDraw.Draw(wear)
    for _ in range(int(mask.size[0] * mask.size[1] * amount / 900)):
        x, y = rnd.randrange(mask.size[0]), rnd.randrange(mask.size[1])
        r = rnd.choice((1, 1, 2, 3))
        d.ellipse((x - r, y - r, x + r, y + r), fill=0)
    for _ in range(40):
        x, y = rnd.randrange(mask.size[0]), rnd.randrange(mask.size[1])
        d.line((x, y, x + rnd.randrange(-60, 60), y + rnd.randrange(-6, 6)), fill=0, width=1)
    return ImageChops.multiply(mask, wear)


def text_mask(text, font, pad=0):
    box = font.getbbox(text)
    m = Image.new('L', (box[2] - box[0] + 2 * pad, box[3] - box[1] + 2 * pad), 0)
    ImageDraw.Draw(m).text((pad - box[0], pad - box[1]), text, font=font, fill=255)
    return m


def main():
    src, out = sys.argv[1], sys.argv[2]
    focus = (0.5, 0.45)
    if '--focus' in sys.argv:
        focus = tuple(float(v) for v in sys.argv[sys.argv.index('--focus') + 1].split(','))
    img = fit(Image.open(src).convert('RGB'), focus)

    # grade: less colour, a cold green-teal cast, darker, more contrast
    img = ImageEnhance.Color(img).enhance(0.55)
    img = ImageChops.multiply(img, Image.new('RGB', (W, H), (190, 225, 215)))
    img = ImageEnhance.Contrast(img).enhance(1.25)
    img = ImageEnhance.Brightness(img).enhance(0.85)
    img = Image.composite(img, Image.new('RGB', (W, H), (4, 8, 8)), vignette())
    img = Image.composite(Image.new('RGB', (W, H), (170, 185, 180)), img, smoke(4).point(lambda v: v * 75 // 255))

    # the title, centred low: LEON over the green banner
    draw = ImageDraw.Draw(img)
    big = ImageFont.truetype(f'{FONTS}\\impact.ttf', 260)
    leon = text_mask('LEON', big)
    lx, ly = (W - leon.size[0]) // 2, int(H * 0.50)
    shadow = leon.filter(ImageFilter.GaussianBlur(14))
    img.paste((0, 0, 0), (lx + 8, ly + 12), shadow)
    img.paste((0, 0, 0), (lx, ly), leon.filter(ImageFilter.MaxFilter(13)))  # outline
    img.paste((236, 236, 232), (lx, ly), worn(leon, 1, 0.08))

    sub = ImageFont.truetype(f'{FONTS}\\bahnschrift.ttf', 112)
    sub.set_variation_by_name('Bold Condensed')
    words = text_mask('ZOMBIES KENNEDY', sub)
    bw, bh = words.size[0] + 90, words.size[1] + 56
    bx, by = (W - bw) // 2, ly + leon.size[1] + 26
    draw.rounded_rectangle((bx, by, bx + bw, by + bh), radius=10, fill=(8, 10, 9), outline=(70, 80, 72), width=3)
    green = Image.new('RGB', words.size, (80, 200, 60))
    green = Image.composite(green, Image.new('RGB', words.size, (40, 130, 30)), Image.linear_gradient('L').resize(words.size).point(lambda v: 255 - v // 2))
    img.paste(green, (bx + 45, by + 28), worn(words, 2))
    img.save(out)
    print(f'wrote {out}')


if __name__ == '__main__':
    main()
