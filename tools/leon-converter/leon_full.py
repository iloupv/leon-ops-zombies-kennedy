"""The full Leon (body + costume, face, head, hair, eyes, hands) as one textured, skinned glTF on the body's
skeleton, from the player's own RE4 PC files.

The parts list is the decomp's character table (tools/motion/character.py LEON, from cPlLeon::setModel /
setRightHand / setLeftHand): (model entry, TPL entry). All parts are in the body's model space at the rest
pose and their bone ids are the body's parts, so each part's weights are remapped onto the body's joints.

usage: python -I leon_full.py <pl00.udas> <01000000.pack> <out dir>"""
import base64
import io
import json
import os
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from udas_reader import entries  # noqa: E402
from uhd_bin_gltf import read_bin  # noqa: E402
from uhd_pack import images  # noqa: E402
from leon_gltf import tpl_refs  # noqa: E402

LEON = [(0, 1, 'body'), (6, 1, 'costume'), (9, 1, 'face'), (4, 3, 'head'), (2, 3, 'hair'), (5, 3, 'eyes'),
        (14, 13, 'right_hand'), (17, 13, 'left_hand')]
# For Black Ops: no knife (on the PC archive the 'face' slot, entry 9, is Leon's knife in his right hand) and
# gripping hands for the BO1 weapons (right 15, left 19; 14 / 16 are relaxed).
LEON_BO1 = [(0, 1, 'body'), (6, 1, 'costume'), (4, 3, 'head'), (2, 3, 'hair'), (5, 3, 'eyes'),
            (15, 13, 'right_hand'), (19, 13, 'left_hand')]
SCALE = 0.001  # game millimetres -> metres


def build(udas, pack_path, out_dir, parts=LEON):
    os.makedirs(out_dir, exist_ok=True)
    ent = {i: data for i, _tag, data in entries(open(udas, 'rb').read())}
    pack = {i: data for i, _k, data in images(open(pack_path, 'rb').read())}
    body = read_bin(ent[parts[0][0]])
    bones = body['bones']
    joint_of = {b[0]: j for j, b in enumerate(bones)}
    world = []
    for bone_id, parent, _, x, y, z in bones:
        base = world[joint_of[parent]] if parent != 0xFF and parent in joint_of else (0.0, 0.0, 0.0)
        world.append((base[0] + x, base[1] + y, base[2] + z))

    blob = bytearray()
    views, accessors = [], []

    def add(data, target=None):
        while len(blob) % 4:
            blob.append(0)
        views.append({'buffer': 0, 'byteOffset': len(blob), 'byteLength': len(data), **({'target': target} if target else {})})
        blob.extend(data)
        return len(views) - 1

    def accessor(data, ctype, count, atype, target=None, mins=None, maxs=None):
        acc = {'bufferView': add(data, target), 'componentType': ctype, 'count': count, 'type': atype}
        if mins is not None:
            acc['min'], acc['max'] = mins, maxs
        accessors.append(acc)
        return len(accessors) - 1

    gl_images, textures, materials, mat_index = [], [], [], {}
    primitives = []
    for model_entry, tpl_entry, role in parts:
        b = read_bin(ent[model_entry])
        refs = tpl_refs(ent[tpl_entry])
        n = len(b['pos'])
        pos = [(x * SCALE, y * SCALE, z * SCALE) for x, y, z in b['pos']]
        a_pos = accessor(b''.join(struct.pack('<3f', *v) for v in pos), 5126, n, 'VEC3', 34962,
                         [min(v[i] for v in pos) for i in range(3)], [max(v[i] for v in pos) for i in range(3)])
        a_nrm = accessor(b''.join(struct.pack('<3f', *v) for v in b['nrm']), 5126, n, 'VEC3', 34962)
        a_uv = accessor(b''.join(struct.pack('<2f', *v) for v in b['uv']), 5126, n, 'VEC2', 34962)
        joints, weights = bytearray(), bytearray()
        for v in range(n):
            jw = []
            if b['wmaps'] and b['wix'][v] < len(b['wmaps']):
                w = b['wmaps'][b['wix'][v]]
                jw = [(joint_of.get(w[k], 0), w[4 + k] / 100.0) for k in range(max(1, min(3, w[3])))]
            if not jw:  # an unweighted part follows its single bone
                jw = [(joint_of.get(b['bones'][0][0], 0) if b['bones'] else 0, 1.0)]
            total = sum(x for _, x in jw) or 1.0
            jw = [(j, x / total) for j, x in jw] + [(0, 0.0)] * (4 - len(jw))
            joints += struct.pack('<4H', *(j for j, _ in jw))
            weights += struct.pack('<4f', *(x for _, x in jw))
        a_j = accessor(bytes(joints), 5123, n, 'VEC4', 34962)
        a_w = accessor(bytes(weights), 5126, n, 'VEC4', 34962)

        for mat, tris in b['faces']:
            if not tris:
                continue
            key = (tpl_entry, mat[12], mat[14])
            if key not in mat_index:
                img = None
                if mat[12] < len(refs) and refs[mat[12]][1] in pack:
                    img = Image.open(io.BytesIO(pack[refs[mat[12]][1]])).convert('RGBA')
                masked = False
                if img is not None and mat[14] < len(refs) and mat[14] != mat[12] and refs[mat[14]][1] in pack:
                    alpha = Image.open(io.BytesIO(pack[refs[mat[14]][1]])).convert('L').resize(img.size)
                    img.putalpha(alpha)
                    masked = True
                m = {'name': f'{role}_t{tpl_entry}_d{mat[12]}', 'pbrMetallicRoughness': {'metallicFactor': 0, 'roughnessFactor': 0.85}}
                if img is not None:
                    name = f'tex_t{tpl_entry}_d{mat[12]}_o{mat[14]}.png'
                    img.save(os.path.join(out_dir, name))
                    gl_images.append({'uri': name})
                    textures.append({'source': len(gl_images) - 1, 'sampler': 0})
                    m['pbrMetallicRoughness']['baseColorTexture'] = {'index': len(textures) - 1}
                    if masked:
                        m.update(alphaMode='MASK', alphaCutoff=0.5, doubleSided=True)
                materials.append(m)
                mat_index[key] = len(materials) - 1
            idx = b''.join(struct.pack('<3I', *t) for t in tris)
            primitives.append({'attributes': {'POSITION': a_pos, 'NORMAL': a_nrm, 'TEXCOORD_0': a_uv,
                                              'JOINTS_0': a_j, 'WEIGHTS_0': a_w},
                               'indices': accessor(idx, 5125, 3 * len(tris), 'SCALAR', 34963),
                               'material': mat_index[key], 'extras': {'part': role}})

    nodes = [{'name': 'leon', 'mesh': 0, 'skin': 0}]
    joint_nodes = []
    for bone_id, parent, _, x, y, z in bones:
        joint_nodes.append(len(nodes))
        nodes.append({'name': f'bone_{bone_id:03d}', 'translation': [x * SCALE, y * SCALE, z * SCALE]})
    roots = []
    for j, (bone_id, parent, *_r) in enumerate(bones):
        if parent != 0xFF and parent in joint_of:
            nodes[joint_nodes[joint_of[parent]]].setdefault('children', []).append(joint_nodes[j])
        else:
            roots.append(joint_nodes[j])
    ibm = b''.join(struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -x * SCALE, -y * SCALE, -z * SCALE, 1)
                   for x, y, z in world)
    a_ibm = accessor(ibm, 5126, len(bones), 'MAT4')
    gl = {
        'asset': {'version': '2.0', 'generator': 'leon-tools leon_full'},
        'scene': 0, 'scenes': [{'nodes': [0] + roots}], 'nodes': nodes,
        'meshes': [{'name': 'leon', 'primitives': primitives}],
        'skins': [{'joints': joint_nodes, 'inverseBindMatrices': a_ibm, 'skeleton': roots[0]}],
        'materials': materials, 'textures': textures, 'images': gl_images,
        'samplers': [{'magFilter': 9729, 'minFilter': 9987}],
        'buffers': [{'byteLength': len(blob), 'uri': 'leon.bin'}],
        'bufferViews': views, 'accessors': accessors,
    }
    open(os.path.join(out_dir, 'leon.bin'), 'wb').write(blob)
    json.dump(gl, open(os.path.join(out_dir, 'leon.gltf'), 'w'))
    print(f'wrote {out_dir}/leon.gltf: {len(primitives)} primitives, {len(materials)} materials, '
          f'{len(gl_images)} textures, {len(bones)} joints')


if __name__ == '__main__':
    build(sys.argv[1], sys.argv[2], sys.argv[3], LEON_BO1 if '--bo1' in sys.argv[4:] else LEON)
