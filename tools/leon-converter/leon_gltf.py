"""Leon (or any UHD player/enemy archive model) -> textured, skinned glTF, from the player's own RE4 PC files.

Inputs: an unpacked .udas (lfsunpack) and the unpacked HD texture pack it references (ImagePackHD/<id>.pack).
Material bytes (JADERLINK UhdBinDecoder.Get_Material): [11] flags, [12] diffuse, [13] bump, [14] opacity map,
each an index into the model's TPL entry; each TPL texture names {pack id, image id} (UhdTplDecoder). The
opacity map's luminance goes into the diffuse image's alpha (glTF MASK), as the hair needs.

usage: python -I leon_gltf.py <file.udas> <model entry> <tpl entry> <file.pack> <out dir>"""
import json
import os
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from udas_reader import entries  # noqa: E402
from uhd_bin_gltf import read_bin, to_gltf  # noqa: E402
from uhd_pack import images  # noqa: E402
import io  # noqa: E402


def tpl_refs(data):
    """[(pack id, image id)] per TPL texture."""
    _magic, n, off_area = struct.unpack_from('<III', data, 0)
    refs = []
    for k in range(n):
        img_ofs, _pal = struct.unpack_from('<II', data, off_area + 8 * k)
        second = struct.unpack_from('<I', data, img_ofs + 8)[0]
        refs.append(struct.unpack_from('<II', data, second))
    return refs


def main(udas, model_entry, tpl_entry, pack_path, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    d = open(udas, 'rb').read()
    ent = {i: data for i, tag, data in entries(d)}
    b = read_bin(ent[model_entry])
    refs = tpl_refs(ent[tpl_entry])
    pack = {i: data for i, _kind, data in images(open(pack_path, 'rb').read())}

    def pack_image(tex_index):
        if tex_index >= len(refs):
            return None
        _pack_id, image_id = refs[tex_index]
        data = pack.get(image_id)
        return Image.open(io.BytesIO(data)).convert('RGBA') if data else None

    gl = to_gltf(b, os.path.basename(out_dir))
    gl['images'], gl['textures'], gl['samplers'] = [], [], [{'magFilter': 9729, 'minFilter': 9987}]
    gl['materials'] = []
    made = {}
    for m, (mat, _tris) in enumerate(b['faces']):
        diffuse, opacity = mat[12], mat[14]
        key = (diffuse, opacity)
        if key not in made:
            img = pack_image(diffuse)
            masked = False
            if img is not None and opacity and opacity != diffuse:
                alpha = pack_image(opacity)
                if alpha is not None:
                    img.putalpha(alpha.convert('L').resize(img.size))
                    masked = True
            material = {'name': f'mat_d{diffuse}_o{opacity}', 'pbrMetallicRoughness': {'metallicFactor': 0, 'roughnessFactor': 0.8}}
            if img is not None:
                name = f'tex_d{diffuse}_o{opacity}.png'
                img.save(os.path.join(out_dir, name))
                gl['images'].append({'uri': name})
                gl['textures'].append({'source': len(gl['images']) - 1, 'sampler': 0})
                material['pbrMetallicRoughness']['baseColorTexture'] = {'index': len(gl['textures']) - 1}
                if masked:
                    material['alphaMode'], material['alphaCutoff'], material['doubleSided'] = 'MASK', 0.5, True
            gl['materials'].append(material)
            made[key] = len(gl['materials']) - 1
        b['faces'][m] = (mat, _tris)
    # one primitive per material, in face order (to_gltf skipped empty ones the same way)
    prims = gl['meshes'][0]['primitives']
    k = 0
    for mat, tris in b['faces']:
        if not tris:
            continue
        prims[k]['material'] = made[(mat[12], mat[14])]
        k += 1
    out = os.path.join(out_dir, os.path.basename(out_dir) + '.gltf')
    json.dump(gl, open(out, 'w'))
    print(f'wrote {out}: {len(gl["materials"])} materials, {len(gl["images"])} textures')
    return b, refs, pack


if __name__ == '__main__':
    main(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], sys.argv[5])
