"""Preview a glTF written by leon_full.py straight from the file: per-triangle colour from the base texture at
the triangle's UV centre (alpha-masked triangles dropped, as alphaCutoff would), flat lit, painter-sorted.

usage: python -I gltf_preview.py <file.gltf> <out.png> [head]"""
import json
import os
import struct
import sys

import numpy as np
from PIL import Image, ImageDraw


def load(path):
    gl = json.load(open(path))
    base = os.path.dirname(path)
    blob = open(os.path.join(base, gl['buffers'][0]['uri']), 'rb').read()
    sizes = {5126: ('f', 4), 5125: ('I', 4), 5123: ('H', 2)}
    comps = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}

    def acc(i):
        a = gl['accessors'][i]
        v = gl['bufferViews'][a['bufferView']]
        fmt, size = sizes[a['componentType']]
        k = comps[a['type']]
        data = struct.unpack_from(f'<{a["count"] * k}{fmt}', blob, v['byteOffset'])
        return np.array(data).reshape(a['count'], k)

    texs = [np.asarray(Image.open(os.path.join(base, gl['images'][t['source']]['uri'])).convert('RGBA')).astype(float)
            for t in gl['textures']]
    tris, cols = [], []
    pos_all = []
    for p in gl['meshes'][0]['primitives']:
        pos = acc(p['attributes']['POSITION'])
        uv = acc(p['attributes']['TEXCOORD_0'])
        idx = acc(p['indices']).reshape(-1, 3).astype(int)
        m = gl['materials'][p['material']]
        tex_i = m.get('pbrMetallicRoughness', {}).get('baseColorTexture', {}).get('index')
        t = texs[tex_i] if tex_i is not None else None
        off = sum(len(x) for x in pos_all)
        pos_all.append(pos)
        for tri in idx:
            if t is None:
                c = (170, 170, 170, 255)
            else:
                u, v = uv[tri].mean(0)
                h, w = t.shape[:2]
                c = t[int((v % 1) * h) % h, int((u % 1) * w) % w]
                if m.get('alphaMode') == 'MASK' and c[3] < 128:
                    continue
            tris.append(tri + off)
            cols.append(c[:3])
    return np.vstack(pos_all), np.array(tris), np.array(cols)


def render(pos, tris, cols, yaw, size=700, focus=None):
    y = np.radians(yaw)
    rot = np.array([[np.cos(y), 0, np.sin(y)], [0, 1, 0], [-np.sin(y), 0, np.cos(y)]])
    p = pos @ rot.T
    if focus is None:
        lo, hi = p.min(0), p.max(0)
    else:
        sel = (pos[:, 1] > focus)
        lo, hi = p[sel].min(0), p[sel].max(0)
    s = size * 0.9 / max(hi[0] - lo[0], hi[1] - lo[1])
    sx = size / 2 + (p[:, 0] - (lo[0] + hi[0]) / 2) * s
    sy = size / 2 - (p[:, 1] - (lo[1] + hi[1]) / 2) * s
    a, b, c = p[tris[:, 0]], p[tris[:, 1]], p[tris[:, 2]]
    n = np.cross(b - a, c - a)
    n /= np.linalg.norm(n, axis=1, keepdims=True) + 1e-12
    light = np.array([0.3, 0.5, 0.8]) / np.linalg.norm([0.3, 0.5, 0.8])
    shade = np.clip(np.abs(n @ light), 0.4, 1.0)
    depth = (a[:, 2] + b[:, 2] + c[:, 2]) / 3
    img = Image.new('RGB', (size, size), (30, 30, 34))
    draw = ImageDraw.Draw(img)
    for k in np.argsort(depth):
        t = tris[k]
        col = tuple(int(min(255, x * shade[k] * 1.5)) for x in cols[k])
        draw.polygon([(sx[t[0]], sy[t[0]]), (sx[t[1]], sy[t[1]]), (sx[t[2]], sy[t[2]])], fill=col)
    return img


if __name__ == '__main__':
    pos, tris, cols = load(sys.argv[1])
    head = len(sys.argv) > 3
    views = [render(pos, tris, cols, yaw, focus=1.45 if head else None) for yaw in ((0, 35, 90) if head else (0, 90, 180))]
    sheet = Image.new('RGB', (700 * len(views), 700))
    for i, v in enumerate(views):
        sheet.paste(v, (700 * i, 0))
    sheet.save(sys.argv[2])
    print('wrote', sys.argv[2], len(tris), 'triangles')
