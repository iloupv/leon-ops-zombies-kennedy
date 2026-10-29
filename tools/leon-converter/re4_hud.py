"""RE4's player HUD images -> the runtime HUD file (.rlh) the BO1 engine draws the mod's HUD from
(gfx_d3d/r_runtime_xmodel.cpp R_RuntimeXModel_HudMaterial, cgame_mp/cg_re4hud_mp.cpp).

RE4 draws its cockpit (game/cockpit.cpp: LifeMeter, BulletInfo) from the shared texture pack 0x07000000 (its TPLs in
BIO4/Etc/core.udas all point there): the life meter's stone frame, a ring whose segmented channel the life fills
(image 268), RE4's italic outlined digits 0-9 (221-230: the loaded rounds), the "EMPTY" badge (262), a plain disc (309:
the heart monitor's screen inside the ring) and the characters' name plates (302 Leon, 299 Ada, 303 HUNK; Luis has
none). The layout is the engine's; only the images are taken.

File (little-endian): 'RLH1', u32 image count, then per image the .rlm texture record (leon_rlm.py): char name[32],
u16 width, u16 height, u8 mip count, u8 alpha masked (0: blended), u16 0, every mip level's BGRA8 pixels, largest first.

usage: python -I re4_hud.py <07000000.pack> <out.rlh>"""
import io
import os
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from uhd_pack import images  # noqa: E402
from leon_rlm import capped, mips_bgra  # noqa: E402

# (name the engine asks for, image in pack 0x07000000)
HUD_IMAGES = ([('frame', 268), ('empty', 262), ('disc', 309), ('name_leon', 302), ('name_ada', 299),
               ('name_hunk', 303)]
              + [(f'digit{n}', 221 + n) for n in range(10)])
# the largest side kept: the HUD is drawn at most ~400 px across at 4K
MAX_SIDE = 1024


def build(pack_path, out_path):
    data = open(pack_path, 'rb').read()
    pack = {i: d for i, _k, d in images(data)}
    out = bytearray(b'RLH1' + struct.pack('<I', 0))
    count = 0
    for name, index in HUD_IMAGES:
        if index not in pack:
            print(f'{name}: no image {index} in the pack, left out')
            continue
        img = capped(Image.open(io.BytesIO(pack[index])).convert('RGBA'), MAX_SIDE)
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
