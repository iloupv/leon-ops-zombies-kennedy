"""Quick software preview of a UHD model .bin: front and side orthographic views, flat shaded, painter-sorted.

usage: python -I preview.py <file.udas> <entry> <out.png>"""
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, __file__.rsplit('\\', 1)[0] if '\\' in __file__ else '.')
from udas_reader import entries  # noqa: E402
from uhd_bin_gltf import read_bin  # noqa: E402


def render(b, yaw_deg, size=700):
    pos = np.array(b['pos'], dtype=np.float64)
    tris = np.array([t for _, ts in b['faces'] for t in ts], dtype=np.int64)
    yaw = np.radians(yaw_deg)
    rot = np.array([[np.cos(yaw), 0, np.sin(yaw)], [0, 1, 0], [-np.sin(yaw), 0, np.cos(yaw)]])
    p = pos @ rot.T
    lo, hi = p.min(0), p.max(0)
    s = (size * 0.9) / max(hi[0] - lo[0], hi[1] - lo[1])
    cx, cy = (lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2
    sx = size / 2 + (p[:, 0] - cx) * s
    sy = size / 2 - (p[:, 1] - cy) * s
    a, bb, c = p[tris[:, 0]], p[tris[:, 1]], p[tris[:, 2]]
    n = np.cross(bb - a, c - a)
    n /= np.linalg.norm(n, axis=1, keepdims=True) + 1e-9
    light = np.array([0.3, 0.5, 0.8])
    light /= np.linalg.norm(light)
    shade = np.clip(np.abs(n @ light), 0.15, 1.0)
    depth = (a[:, 2] + bb[:, 2] + c[:, 2]) / 3
    img = Image.new('RGB', (size, size), (30, 30, 34))
    draw = ImageDraw.Draw(img)
    for k in np.argsort(depth):
        t = tris[k]
        g = int(60 + 180 * shade[k])
        draw.polygon([(sx[t[0]], sy[t[0]]), (sx[t[1]], sy[t[1]]), (sx[t[2]], sy[t[2]])], fill=(g, g, int(g * 0.95)))
    return img


if __name__ == '__main__':
    path, entry, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    d = open(path, 'rb').read()
    data = next(data for i, tag, data in entries(d) if i == entry)
    b = read_bin(data)
    views = [render(b, yaw) for yaw in (0, 90, 180)]
    sheet = Image.new('RGB', (sum(v.width for v in views), views[0].height))
    x = 0
    for v in views:
        sheet.paste(v, (x, 0))
        x += v.width
    sheet.save(out)
    print('wrote', out)
