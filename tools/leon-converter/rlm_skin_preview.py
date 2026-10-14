"""Offline check of a .rlm v2 file: skins the model the way the BO1 engine does (bind pose = the bones' rest
offsets, pose = an ANM2 frame's local rotation + rest offset + baked offset, linear blend skinning) and renders a
z-buffered, alpha-tested front view (BO1 +X faces the viewer) of the region around the head.

usage: python -I rlm_skin_preview.py <file.rlm> <out.png> [anim index] [frame] [--rest] [--back]"""
import struct
import sys

import numpy as np
from PIL import Image


def read_rlm(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'RLM1'
    version, tex_count, surf_count, bone_count = struct.unpack_from('<IIII', d, 4)
    o = 20
    textures = []
    for _ in range(tex_count):
        w, h, mips, masked, _pad = struct.unpack_from('<HHBBH', d, o + 32)
        o += 40
        px = np.frombuffer(d, np.uint8, w * h * 4, o).reshape(h, w, 4)[:, :, [2, 1, 0, 3]]
        textures.append((px, masked))
        for m in range(mips):
            o += max(1, w >> m) * max(1, h >> m) * 4
    bones = []
    for _ in range(bone_count):
        parent, = struct.unpack_from('<i', d, o + 32)
        trans = struct.unpack_from('<3f', d, o + 36)
        bones.append((parent, np.array(trans)))
        o += 32 + 4 + 28
    surfaces = []
    for _ in range(surf_count):
        tex, nv, nt = struct.unpack_from('<III', d, o)
        o += 12
        vt = np.dtype([('pos', '<3f4'), ('nrm', '<3f4'), ('uv', '<2f4'), ('bone', '<4u2'), ('w', '<4f4')])
        verts = np.frombuffer(d, vt, nv, o)
        o += nv * vt.itemsize
        idx = np.frombuffer(d, '<u2', nt * 3, o).reshape(-1, 3)
        o += nt * 6
        o += (-o) % 4
        surfaces.append((tex, verts, idx))
    anims = []
    if d[o:o + 4] == b'ANM2':
        n, = struct.unpack_from('<I', d, o + 4)
        o += 8
        for _ in range(n):
            name = d[o:o + 32].rstrip(b'\0').decode()
            fc, fps, bc, speed = struct.unpack_from('<IfIf', d, o + 32)
            o += 48
            frames = np.frombuffer(d, '<f4', fc * bc * 7, o).reshape(fc, bc, 7)
            o += fc * bc * 28
            anims.append((name, frames))
    return textures, bones, surfaces, anims


def quat_mat(q):
    x, y, z, w = q / np.linalg.norm(q)
    return np.array([[1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)],
                     [2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
                     [2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)]])


def skin_mats(bones, frame):
    """Per bone 4x4 skinning matrix: pose(bone) * inverse(bind(bone))."""
    n = len(bones)
    pose, bind = [None] * n, [None] * n
    for b, (parent, rest) in enumerate(bones):
        local = np.eye(4)
        bl = np.eye(4)
        bl[:3, 3] = rest
        if frame is not None:
            local[:3, :3] = quat_mat(frame[b, :4])
            local[:3, 3] = rest + frame[b, 4:]
        else:
            local[:3, 3] = rest
        pose[b] = (pose[parent] if parent >= 0 else np.eye(4)) @ local
        bind[b] = (bind[parent] if parent >= 0 else np.eye(4)) @ bl
    return [pose[b] @ np.linalg.inv(bind[b]) for b in range(n)]


def main():
    path, out = sys.argv[1], sys.argv[2]
    rest = '--rest' in sys.argv
    back = '--back' in sys.argv  # the viewer behind (-X) instead of in front
    args = [a for a in sys.argv[3:] if not a.startswith('--')]
    anim, frame_no = (int(args[0]) if args else 0), (int(args[1]) if len(args) > 1 else 0)
    textures, bones, surfaces, anims = read_rlm(path)
    frame = None if rest or not anims else anims[anim][1][frame_no]
    mats = skin_mats(bones, frame)
    S = 500
    col = np.full((S, S, 3), (60, 90, 60), np.float32)
    zb = np.full((S, S), -1e9)
    world = []
    for tex, verts, idx in surfaces:
        p = np.zeros((len(verts), 3))
        for k in range(4):
            w = verts['w'][:, k]
            m = np.stack([mats[b] for b in verts['bone'][:, k]])
            hp = np.concatenate([verts['pos'], np.ones((len(verts), 1))], 1)
            p += w[:, None] * np.einsum('nij,nj->ni', m, hp)[:, :3]
        if back:
            p = p * np.array([-1.0, -1.0, 1.0])  # turned 180 degrees about Z
        world.append(p)
    allp = np.vstack(world)
    top = allp[:, 2].max()
    head = allp[allp[:, 2] > top - 14]  # the top 14 inches
    cy_, cz = head[:, 1].mean(), top - 7
    s = S / 18.0
    for (tex, verts, idx), p in zip(surfaces, world):
        px, masked = textures[tex]
        X = S / 2 + (p[:, 1] - cy_) * s
        Y = S / 2 - (p[:, 2] - cz) * s
        uv = verts['uv']
        for t in idx:
            x, y = X[t], Y[t]
            x0, x1 = int(max(0, np.floor(x.min()))), int(min(S - 1, np.ceil(x.max())))
            y0, y1 = int(max(0, np.floor(y.min()))), int(min(S - 1, np.ceil(y.max())))
            if x1 < x0 or y1 < y0:
                continue
            gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            d = (y[1] - y[2]) * (x[0] - x[2]) + (x[2] - x[1]) * (y[0] - y[2])
            if abs(d) < 1e-9:
                continue
            w0 = ((y[1] - y[2]) * (gx - x[2]) + (x[2] - x[1]) * (gy - y[2])) / d
            w1 = ((y[2] - y[0]) * (gx - x[2]) + (x[0] - x[2]) * (gy - y[2])) / d
            w2 = 1 - w0 - w1
            ok = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not ok.any():
                continue
            z = w0 * p[t[0], 0] + w1 * p[t[1], 0] + w2 * p[t[2], 0]
            u = w0 * uv[t[0], 0] + w1 * uv[t[1], 0] + w2 * uv[t[2], 0]
            v = w0 * uv[t[0], 1] + w1 * uv[t[1], 1] + w2 * uv[t[2], 1]
            h, w = px.shape[:2]
            ti = np.floor(v % 1 * h).astype(int) % h
            tj = np.floor(u % 1 * w).astype(int) % w
            texel = px[ti, tj]
            if masked:
                ok &= texel[..., 3] >= 128
            sub = zb[y0:y1 + 1, x0:x1 + 1]
            ok &= z > sub
            sub[ok] = z[ok]
            col[y0:y1 + 1, x0:x1 + 1][ok] = texel[..., :3][ok]
    Image.fromarray(col.clip(0, 255).astype(np.uint8)).save(out)
    print('rendered', 'rest pose' if frame is None else f'{anims[anim][0]} frame {frame_no}')


if __name__ == '__main__':
    main()
