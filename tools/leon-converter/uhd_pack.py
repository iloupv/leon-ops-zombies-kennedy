"""RE4 PC (UHD) HD texture pack (ImagePackHD/<id>.pack.yz2.lfs, after lfsunpack): {u32 pack id, u32 count,
u32 offset[count]} then per image {u32 length, u32 0xFFFFFFFF, u32 pack id, u32 type, data}, the data a DDS
('DDS ') or a TGA. Offset 0 = no image; a repeated offset = the same image (JADERLINK RE4-UHD-PACK-TOOL, MIT).

usage: python -I uhd_pack.py <file.pack> <out dir>   (writes NNNN.dds / NNNN.tga and NNNN.png)"""
import io
import os
import struct
import sys


def images(d):
    pack_id, count = struct.unpack_from('<II', d, 0)
    offs = struct.unpack_from(f'<{count}I', d, 8)
    for i, o in enumerate(offs):
        if not o or o >= len(d):
            continue
        length = struct.unpack_from('<I', d, o)[0]
        data = d[o + 16:o + 16 + length]
        kind = 'dds' if data[:4] == b'DDS ' else 'tga'
        yield i, kind, data


if __name__ == '__main__':
    from PIL import Image
    d = open(sys.argv[1], 'rb').read()
    out = sys.argv[2]
    os.makedirs(out, exist_ok=True)
    for i, kind, data in images(d):
        open(os.path.join(out, f'{i:04d}.{kind}'), 'wb').write(data)
        try:
            img = Image.open(io.BytesIO(data))
            img.save(os.path.join(out, f'{i:04d}.png'))
            print(f'{i:04d}: {kind} {img.size[0]}x{img.size[1]} {img.mode}')
        except Exception as e:
            print(f'{i:04d}: {kind}, {len(data)} bytes, not decoded: {e}')
