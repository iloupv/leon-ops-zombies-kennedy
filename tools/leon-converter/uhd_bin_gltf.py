"""RE4 PC (UHD) model .bin -> skinned glTF 2.0 (mesh + skeleton, rest pose; no textures yet).

The UHD BIN layout follows JADERLINK's RE4-UHD-BIN-TPL-TOOLS (MIT, UhdBinDecoder.cs): a little-endian header
(bone list offset 0x40/0x50/0x60 = header size), float positions / normals / texcoords, bones {u8 id, u8 parent
(0xFF root), u16 pad, f32 x, y, z relative to the parent}, weight maps {u8 bone id x3, u8 count, u8 weight x3
(percent), u8 pad}, a u16 weight-map index per vertex, and materials (24 bytes each) followed by their face
strips: u32 buffer size, u32 count, then {u32 strip count; {u16 type, u16 count}...}. Strips walk the vertex
array in order: type 5 triangle list, 6 triangle strip (alternating winding), 7 fan, 8 quads.

Units: game millimetres, Y up (glTF is Y up too), written in metres.

usage: python -I uhd_bin_gltf.py <file.udas> <entry> <out.gltf>"""
import base64
import json
import struct
import sys

sys.argv_saved = list(sys.argv)
from udas_reader import entries  # noqa: E402


def read_bin(d):
    bone_ofs, _, _, clr_ofs, tex_ofs, wmap_ofs = struct.unpack_from('<6I', d, 0)
    wmap_count, bone_count, mat_count = struct.unpack_from('<BBH', d, 0x18)
    mat_ofs, flags, tex_count = struct.unpack_from('<III', d, 0x1C)
    vscale, _, wmap2_count, morph_ofs = struct.unpack_from('<BBHI', d, 0x28)
    pos_ofs, nrm_ofs, n_pos, n_nrm, version = struct.unpack_from('<IIHHI', d, 0x30)
    wix_ofs = 0
    if bone_ofs >= 0x50:
        _bonepair, _adjacent, wix_ofs, _wix2 = struct.unpack_from('<4I', d, 0x40)
    if bone_ofs not in (0x40, 0x50, 0x60) or not flags & 0x80000000:
        raise ValueError('not a UHD model .bin')

    # materials and their faces; the vertex count is how far the strips walk
    faces = []  # (material, [(a, b, c)...])
    p = mat_ofs
    vtx = 0
    for m in range(mat_count):
        mat = d[p:p + 24]
        p += 24
        buf_size, _count = struct.unpack_from('<II', d, p)
        p += 8
        start = p
        n_strips, = struct.unpack_from('<I', d, p)
        q = p + 4
        tris = []
        for _ in range(n_strips):
            ftype, fcount = struct.unpack_from('<HH', d, q)
            q += 4
            a = vtx
            if ftype == 5:
                for i in range(fcount // 3):
                    tris.append((a + 3 * i, a + 3 * i + 1, a + 3 * i + 2))
                vtx += fcount - fcount % 3
            elif ftype == 6:
                for i in range(fcount - 2):
                    t = (a + i, a + i + 1, a + i + 2)
                    tris.append(t if i % 2 == 0 else (t[2], t[1], t[0]))
                vtx += fcount
            elif ftype == 7:
                for i in range(2, fcount):
                    tris.append((a, a + i - 1, a + i))
                vtx += fcount
            elif ftype == 8:
                for i in range(fcount // 4):
                    b = a + 4 * i
                    tris += [(b, b + 1, b + 2), (b, b + 2, b + 3)]
                vtx += fcount - fcount % 4
            else:
                raise ValueError(f'material {m}: unknown strip type {ftype}')
        faces.append((mat, [t for t in tris if len(set(t)) == 3]))
        p = start + buf_size

    n = vtx
    pos = [struct.unpack_from('<3f', d, pos_ofs + 12 * i) for i in range(n)]
    # the normals are stored scaled (lengths around 2^39, directions right): unit length here
    nrm = []
    for i in range(n):
        x, y, z = struct.unpack_from('<3f', d, nrm_ofs + 12 * i)
        length = (x * x + y * y + z * z) ** 0.5
        nrm.append((x / length, y / length, z / length) if length > 0 else (0.0, 1.0, 0.0))
    uv = [struct.unpack_from('<2f', d, tex_ofs + 8 * i) for i in range(n)]
    bones = [struct.unpack_from('<BBH3f', d, bone_ofs + 16 * i) for i in range(bone_count)]
    n_wmap = wmap2_count if wmap2_count > 255 else wmap_count
    wmaps = [struct.unpack_from('<8B', d, wmap_ofs + 8 * i) for i in range(n_wmap)] if wmap_ofs else []
    wix = [struct.unpack_from('<H', d, wix_ofs + 2 * i)[0] for i in range(n)] if wix_ofs else [0] * n
    return dict(pos=pos, nrm=nrm, uv=uv, bones=bones, wmaps=wmaps, wix=wix, faces=faces, version=version)


def to_gltf(b, name, scale=0.001):
    bones = b['bones']
    joint_of = {bone[0]: j for j, bone in enumerate(bones)}
    world = []
    for bone_id, parent, _, x, y, z in bones:
        if parent != 0xFF and parent in joint_of:
            px, py, pz = world[joint_of[parent]]
            world.append((px + x, py + y, pz + z))
        else:
            world.append((x, y, z))

    blob = bytearray()
    views, accessors = [], []

    def add(data, target=None):
        while len(blob) % 4:
            blob.append(0)
        views.append({'buffer': 0, 'byteOffset': len(blob), 'byteLength': len(data), **({'target': target} if target else {})})
        blob.extend(data)
        return len(views) - 1

    def accessor(view, ctype, count, atype, mins=None, maxs=None, normalized=False):
        acc = {'bufferView': view, 'componentType': ctype, 'count': count, 'type': atype}
        if mins is not None:
            acc['min'], acc['max'] = mins, maxs
        if normalized:
            acc['normalized'] = True
        accessors.append(acc)
        return len(accessors) - 1

    pos = [(x * scale, y * scale, z * scale) for x, y, z in b['pos']]
    a_pos = accessor(add(b''.join(struct.pack('<3f', *v) for v in pos), 34962), 5126, len(pos), 'VEC3',
                     [min(v[i] for v in pos) for i in range(3)], [max(v[i] for v in pos) for i in range(3)])
    a_nrm = accessor(add(b''.join(struct.pack('<3f', *v) for v in b['nrm']), 34962), 5126, len(pos), 'VEC3')
    a_uv = accessor(add(b''.join(struct.pack('<2f', *v) for v in b['uv']), 34962), 5126, len(pos), 'VEC2')

    joints, weights = bytearray(), bytearray()
    for v in range(len(pos)):
        jw = [(0, 0.0)] * 4
        if b['wmaps'] and b['wix'][v] < len(b['wmaps']):
            w = b['wmaps'][b['wix'][v]]
            count = max(1, min(3, w[3]))
            pairs = [(joint_of.get(w[k], 0), w[4 + k] / 100.0) for k in range(count)]
            total = sum(p[1] for p in pairs) or 1.0
            jw = [(j, wt / total) for j, wt in pairs] + [(0, 0.0)] * (4 - len(pairs))
        joints += struct.pack('<4H', *(j for j, _ in jw))
        weights += struct.pack('<4f', *(wt for _, wt in jw))
    a_joints = accessor(add(bytes(joints), 34962), 5123, len(pos), 'VEC4')
    a_weights = accessor(add(bytes(weights), 34962), 5126, len(pos), 'VEC4')

    primitives = []
    for mat_index, (_mat, tris) in enumerate(b['faces']):
        if not tris:
            continue
        idx = b''.join(struct.pack('<3I', *t) for t in tris)
        a_idx = accessor(add(idx, 34963), 5125, 3 * len(tris), 'SCALAR')
        primitives.append({'attributes': {'POSITION': a_pos, 'NORMAL': a_nrm, 'TEXCOORD_0': a_uv,
                                          'JOINTS_0': a_joints, 'WEIGHTS_0': a_weights},
                           'indices': a_idx, 'material': 0})

    nodes = [{'name': name, 'mesh': 0, 'skin': 0}]
    joint_nodes = []
    for j, (bone_id, parent, _, x, y, z) in enumerate(bones):
        joint_nodes.append(len(nodes))
        nodes.append({'name': f'bone_{bone_id:03d}', 'translation': [x * scale, y * scale, z * scale]})
    roots = []
    for j, (bone_id, parent, *_rest) in enumerate(bones):
        if parent != 0xFF and parent in joint_of:
            nodes[joint_nodes[joint_of[parent]]].setdefault('children', []).append(joint_nodes[j])
        else:
            roots.append(joint_nodes[j])
    ibm = b''.join(struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -x * scale, -y * scale, -z * scale, 1)
                   for x, y, z in world)
    a_ibm = accessor(add(ibm), 5126, len(bones), 'MAT4')

    return {
        'asset': {'version': '2.0', 'generator': 'leon-tools uhd_bin_gltf'},
        'scene': 0,
        'scenes': [{'nodes': [0] + roots}],
        'nodes': nodes,
        'meshes': [{'name': name, 'primitives': primitives}],
        'skins': [{'joints': joint_nodes, 'inverseBindMatrices': a_ibm, **({'skeleton': roots[0]} if roots else {})}],
        'materials': [{'name': 'untextured', 'pbrMetallicRoughness': {'baseColorFactor': [0.7, 0.7, 0.7, 1], 'metallicFactor': 0}}],
        'buffers': [{'byteLength': len(blob), 'uri': 'data:application/octet-stream;base64,' + base64.b64encode(bytes(blob)).decode()}],
        'bufferViews': views,
        'accessors': accessors,
    }


if __name__ == '__main__':
    path, entry, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    d = open(path, 'rb').read()
    data = next(data for i, tag, data in entries(d) if i == entry)
    b = read_bin(data)
    n_tris = sum(len(t) for _, t in b['faces'])
    print(f'entry {entry}: {len(b["pos"])} vertices, {n_tris} triangles, {len(b["faces"])} materials, '
          f'{len(b["bones"])} bones, {len(b["wmaps"])} weight maps, version 0x{b["version"]:08X}')
    json.dump(to_gltf(b, f'pl00_{entry:03d}'), open(out, 'w'))
    print('wrote', out)
