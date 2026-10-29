"""Leon -> runtime model file (.rlm) for the BO1 engine's runtime XModel loader (gfx_d3d/r_runtime_xmodel.cpp).

Version 2 is skinned: a tag_origin root, then Leon's body skeleton (RE4 bone id N -> BO1 bone N + 1, parents
first), every vertex weighted to up to 4 bones. The bind pose is the rest (T) pose the vertices are in.
Positions convert from RE4 (Y up, millimetres) to BO1 (Z up, inches) with (x, y, z) -> (z, x, y): the cyclic
axis map, a rotation (handedness and triangle winding kept) that puts Leon's front, RE4 +Z (his toes and his walk
cycle's root motion point that way), on BO1's forward axis +X.

--apose: a pose test. The upper arms' local rotations (RE4 bones 7 and 13) turn 60 degrees down while the bind
pose stays the T pose, so a working skinning path shows Leon with his arms lowered.

File (little-endian):
  'RLM1', u32 version (2), u32 texture count, u32 surface count, u32 bone count
  texture: char name[32], u16 width, u16 height, u8 mip count, u8 alpha masked, u16 0,
           then every mip level's BGRA8 pixels, largest first
  bone:    char name[32], i32 parent index (-1: root), f32 local translation[3] (inches),
           f32 local rotation quaternion[4] (x, y, z, w); bones are ordered parents first
  surface: u32 texture index, u32 vertex count, u32 triangle count,
           vertex count x {f32 pos[3], f32 normal[3], f32 uv[2], u16 bone[4], f32 weight[4]} (weights sum to 1,
           heaviest first, unused slots weight 0), triangle count x 3 u16 indices, zero pad to 4

--anims: bakes Leon's unarmed idle / walk / back step / run (wep00.udas, next to pl00.udas) with the game's own
motion code and IK (re4_motion.py) into the animation section; the engine picks one per entity from how it moves.

--armed: the same with the handgun's cycles (wep01.udas) and its gripping right hand, the gun
mounted on tag_weapon_right as RE4 mounts it.

usage: python -I leon_rlm.py <pl00.udas> <01000000.pack>[;<more packs>] <out.rlm> [--apose] [--anims | --armed]
       [--character leon|ada|hunk|luis]   (the archive given is that character's: pl00, pl0c, pl06, pl14)
       python -I leon_rlm.py <em20.udas> <00000020.pack> <out.rlm> --enemy salvador   (ENEMIES)
(the packs by the id in their headers: a TPL texture names its pack - the RE4 HD Project's Leon points his face at the
shared 07000000 pack)"""
import io
import math
import os
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from udas_reader import entries  # noqa: E402
from uhd_bin_gltf import read_bin  # noqa: E402
from uhd_pack import images  # noqa: E402
from leon_gltf import tpl_refs  # noqa: E402
from leon_full import LEON_BO1  # noqa: E402

MM_PER_INCH = 25.4
APOSE_BONES = {7: -60.0, 13: 60.0}  # RE4 upper arm bone id -> rotation about BO1 +X, degrees
# the model's size against RE4's (build's scale: an enemy made larger), applied to every position converted
_scale = 1.0


def to_bo1(v):
    x, y, z = v
    k = _scale / MM_PER_INCH
    return (z * k, x * k, y * k)


def to_bo1_dir(v):
    x, y, z = v
    return (z, x, y)


# the largest texture side kept: the RE4 HD Project's 4096 x 4096 ones are halved (uncompressed, with every character
# loaded at once, they cost the 32-bit game's memory what a third-person view never shows)
MAX_TEXTURE = 2048


def capped(img, limit=MAX_TEXTURE):
    if max(img.size) <= limit:
        return img
    scale = limit / max(img.size)
    return img.resize((max(1, int(img.size[0] * scale)), max(1, int(img.size[1] * scale))), Image.LANCZOS)


def mips_bgra(img):
    levels = []
    w, h = img.size
    while True:
        r, g, b, a = img.split()
        levels.append(Image.merge('RGBA', (b, g, r, a)).tobytes())
        if w == 1 and h == 1:
            break
        w, h = max(1, w // 2), max(1, h // 2)
        img = img.resize((w, h), Image.LANCZOS)
    return levels


# The BO1 names of Leon's main bones (RE4 bone id -> name), so code and scripts that look a player's bones up by
# the standard names find them; the rest keep j_re4_NNN. Leon faces RE4 +Z, so his right side is RE4 -X.
BO1_BONE_NAMES = {
    0: 'j_mainroot', 1: 'j_spinelower', 2: 'j_spine4', 3: 'j_neck', 4: 'j_head',
    5: 'j_clavicle_ri', 7: 'j_shoulder_ri', 8: 'j_elbow_ri', 9: 'j_wristtwist_ri', 10: 'j_wrist_ri',
    11: 'j_clavicle_le', 13: 'j_shoulder_le', 14: 'j_elbow_le', 15: 'j_wristtwist_le', 16: 'j_wrist_le',
    18: 'j_hip_ri', 19: 'j_knee_ri', 20: 'j_ankle_ri', 21: 'j_ball_ri',
    22: 'j_hip_le', 23: 'j_knee_le', 24: 'j_ankle_le', 25: 'j_ball_le',
}
# Tags (no vertices) after the skeleton: name, parent RE4 bone id, offset from it in RE4 millimetres, and the tag's
# axes (forward X, up Z) in the parent bone's RE4 frame. The eye sits between Leon's eyes. The weapon tags carry a
# BO1 weapon world model the way RE4 holds its handgun (wep01's model hangs on the right hand, cObjFn57::init): the
# barrel along the forearm (the hand's -X), the slide on the thumb side (+Z). A BO1 world weapon's geometry sits
# behind its origin (t5_weapon_1911_world: x -12.6 .. 3.2, the grip under it), as BO1 rigs put tag_weapon_right
# ahead of the palm, so the tags are ~290 mm out along the forearm and ~54 mm to the thumb side - set in game with a
# tuning pass (the pistol in the hands, the laser from its muzzle).
TAGS = [
    ('tag_eye', 4, (0.0, 76.0, 85.0), None),
    # (30 mm deeper than the first fit, -290, and 12 mm higher: the pistol sat short of the palm, below it; then 20 mm
    # further out along the forearm: the guns sat a little back in the hand)
    ('tag_weapon_right', 10, (-340.0, -24.0, 66.0), ((-1.0, 0.0, 0.0), (0.0, 0.0, 1.0))),
    ('tag_weapon_left', 16, (310.0, -24.0, 54.0), ((1.0, 0.0, 0.0), (0.0, 0.0, 1.0))),
    # BO1's thrown grenade (cg_main_mp's player DObj: the offhand's world model on tag_inhand) in the right hand, RE4's
    # throwing hand (pl_grenade.cpp), in the fist (at the gun's -290 it floated a hand's length ahead, at -185 still past the fingers)
    ('tag_inhand', 10, (-100.0, -24.0, 30.0), ((-1.0, 0.0, 0.0), (0.0, 0.0, 1.0))),
    # BO1's knife (cg_main_mp's player DObj: the melee weapon's world model, through RE4's slash) in the right fist,
    # the blade out along the forearm: its grip is ~100 mm behind the grenade's palm point (the knife's origin is
    # its butt; on tag_inhand the whole grip sat ahead of the fingers)
    ('tag_knife', 10, (-185.0, -24.0, 30.0), ((-1.0, 0.0, 0.0), (0.0, 0.0, 1.0))),
    # Moon's astronaut helmet (maps\leon\_helmet.gsc: a BO1 head model) on his head, in BO1's j_head frame (its X up
    # the head, Y forward): set in game from front and side views
    ('tag_helmet', 4, (0.0, -315.0, -30.0), ((0.0, 1.0, 0.0), (1.0, 0.0, 0.0))),
]


def quat_from_axes(forward, up):
    """(x, y, z, w) of the rotation taking BO1 X to `forward` and Z to `up` (unit, perpendicular, BO1 axes)."""
    fx, fy, fz = forward
    ux, uy, uz = up
    lx, ly, lz = uy * fz - uz * fy, uz * fx - ux * fz, ux * fy - uy * fx  # Y = Z x X
    m = ((fx, lx, ux), (fy, ly, uy), (fz, lz, uz))  # columns: the tag's X, Y, Z
    tr = m[0][0] + m[1][1] + m[2][2]
    if tr > 0:
        w = math.sqrt(1.0 + tr) / 2
        return ((m[2][1] - m[1][2]) / (4 * w), (m[0][2] - m[2][0]) / (4 * w), (m[1][0] - m[0][1]) / (4 * w), w)
    i = max(range(3), key=lambda k: m[k][k])
    j, k = (i + 1) % 3, (i + 2) % 3
    r = math.sqrt(1.0 + m[i][i] - m[j][j] - m[k][k])
    q = [0.0, 0.0, 0.0, 0.0]
    q[i] = r / 2
    q[j] = (m[j][i] + m[i][j]) / (2 * r)
    q[k] = (m[k][i] + m[i][k]) / (2 * r)
    q[3] = (m[k][j] - m[j][k]) / (2 * r)
    return tuple(q)


def tag_quat(axes):
    if axes is None:
        return (0.0, 0.0, 0.0, 1.0)
    return quat_from_axes(to_bo1_dir(axes[0]), to_bo1_dir(axes[1]))


def skeleton(body, apose, eyelid=True, tags=TAGS, names=BO1_BONE_NAMES):
    """[(name, parent index, local trans, local quat)]: tag_origin, then RE4 bone id N at index N + 1, then the tags.
    eyelid False: bone 0x1C is not one (HUNK's is on his knee), so it is not named as the engine's blink finds it.
    names: the BO1 names by RE4 bone id (a skeleton numbered as RE4's humans: Leon's, a Ganado's); the rest j_re4_NNN."""
    bones = [('tag_origin', -1, (0.0, 0.0, 0.0), (0.0, 0.0, 0.0, 1.0))]
    for bone_id, parent, _pad, x, y, z in body['bones']:
        quat = (0.0, 0.0, 0.0, 1.0)
        if apose and bone_id in APOSE_BONES:
            half = math.radians(APOSE_BONES[bone_id]) / 2
            quat = (math.sin(half), 0.0, 0.0, math.cos(half))
        name = names.get(bone_id, f'j_re4_{bone_id:03d}')
        if bone_id == 0x1C and not eyelid:
            name += '_not_eyelid'
        bones.append((name, 0 if parent == 0xFF else parent + 1, to_bo1((x, y, z)), quat))
    for name, parent_id, offset, axes in tags:
        bones.append((name, parent_id + 1, to_bo1(offset), tag_quat(axes)))
    return bones


def vertex_weights(b, v, bone_of):
    """[(bo1 bone, weight)] for a vertex, heaviest first, at most 4, summing to 1."""
    pairs = []
    if b['wmaps'] and b['wix'][v] < len(b['wmaps']):
        w = b['wmaps'][b['wix'][v]]
        pairs = [(bone_of(w[k]), w[4 + k]) for k in range(max(1, min(3, w[3]))) if w[4 + k] > 0]
    if not pairs:  # an unweighted part follows its first bone
        pairs = [(bone_of(b['bones'][0][0]) if b['bones'] else 0, 100)]
    merged = {}
    for bone, wt in pairs:
        merged[bone] = merged.get(bone, 0) + wt
    pairs = sorted(merged.items(), key=lambda p: -p[1])[:4]
    total = float(sum(wt for _, wt in pairs))
    return [(bone, wt / total) for bone, wt in pairs]


def held_matrix(rot):
    """RE4's model angle matrix (game/math_sub.cpp RotMatrix: Rz * Ry * Rx, radians), rows."""
    sx, sy, sz = (math.sin(a) for a in rot)
    cx, cy, cz = (math.cos(a) for a in rot)
    return ((cz * cy, cz * sx * sy - sz * cx, cz * cx * sy + sz * sx),
            (sz * cy, sz * sx * sy + cz * cx, sz * cx * sy - cz * sx),
            (-sy, cy * sx, cy * cx))


def build(udas, pack_path, out_path, parts=LEON_BO1, apose=False, motions=(), eyelid=True, tags=TAGS, held=(),
          max_texture=MAX_TEXTURE, root_move=(), scale=1.0, names=BO1_BONE_NAMES, max_bone=None):
    """parts: (model entry, TPL entry, role) of pl00.udas, or with a 4th field, the archive .udas of a model from
    another archive (a weapon's gripping hand; its TPL and images stay Leon's). motions: [(name, FCV bytes)].
    held: (model entry, TPL entry, RE4 bone id, position mm, angles radians) - a model RE4 hangs on a part (an enemy's
    weapon, cEmWep::setParentMatCalc: the part's matrix times the angles' and the position's), made rigid on it.
    root_move: the motions whose root travel is baked in (bake_motion). scale: its size against RE4's (its skeleton,
    vertices and motions' offsets all scaled). names: the bones' BO1 names (skeleton). max_bone: the last RE4 bone id
    kept; the ones past it fold into their kept ancestor."""
    global _scale
    _scale = scale
    try:
        build_scaled(udas, pack_path, out_path, parts, apose, motions, eyelid, tags, held, max_texture, root_move, names,
                     max_bone)
    finally:
        _scale = 1.0


def build_scaled(udas, pack_path, out_path, parts, apose, motions, eyelid, tags, held, max_texture, root_move, names,
                 max_bone):
    ent = {i: data for i, _t, data in entries(open(udas, 'rb').read())}
    packs = {}
    for path in pack_path.split(';'):
        data = open(path, 'rb').read()
        if len(data) >= 8:
            packs[struct.unpack_from('<I', data, 0)[0]] = {i: d for i, _k, d in images(data)}
    sources = {None: ent}
    for part in parts:
        if len(part) > 3 and part[3] not in sources:
            sources[part[3]] = {i: d for i, _t, d in entries(open(part[3], 'rb').read())}

    def image(refs, k):
        """The k-th TPL texture's image data from its own pack, or None."""
        if k >= len(refs):
            return None
        pack_id, image_id = refs[k]
        return packs.get(pack_id, {}).get(image_id)
    body = read_bin(ent[parts[0][0]])
    # the bones past max_bone (ones no motion keys: cloth, claws - stiff on their parents) folded into their kept
    # ancestor, their vertices with them: the engine's DObj holds 160 bones at most (DOBJ_MAX_PARTS)
    parent_of = {bone[0]: bone[1] for bone in body['bones']}
    kept = dict(body, bones=[bone for bone in body['bones'] if max_bone is None or bone[0] <= max_bone])
    bones = skeleton(kept, apose, eyelid, tags, names)
    body_ids = {bone[0] for bone in kept['bones']}

    def bone_of(re4_id):
        while re4_id not in body_ids and parent_of.get(re4_id, 0xFF) != 0xFF:
            re4_id = parent_of[re4_id]
        return re4_id + 1 if re4_id in body_ids else 0

    textures, tex_index, surfaces = [], {}, []
    for part in parts:
        model_entry, tpl_entry, role = part[:3]
        source = part[3] if len(part) > 3 else None
        part_ent = sources[source]
        b = read_bin(part_ent[model_entry])
        refs = tpl_refs(ent[tpl_entry])
        for mat, tris in b['faces']:
            if not tris:
                continue
            key = (tpl_entry, mat[12], mat[14])
            if key not in tex_index:
                img = None
                colour = image(refs, mat[12])
                if colour is not None:
                    img = Image.open(io.BytesIO(colour)).convert('RGBA')
                masked = False
                opacity = image(refs, mat[14]) if mat[14] != mat[12] else None
                if img is not None and opacity is not None:
                    img.putalpha(Image.open(io.BytesIO(opacity)).convert('L').resize(img.size))
                    masked = True
                if img is None:
                    img = Image.new('RGBA', (4, 4), (160, 160, 160, 255))
                tex_index[key] = len(textures)
                textures.append((f'leon_t{tpl_entry}_d{mat[12]}_o{mat[14]}', img, masked))
            used = sorted({i for t in tris for i in t})
            remap = {old: new for new, old in enumerate(used)}
            verts = [(to_bo1(b['pos'][i]), to_bo1_dir(b['nrm'][i]), b['uv'][i], vertex_weights(b, i, bone_of)) for i in used]
            # RE4 fronts wind counter-clockwise; the BO1 renderer's wind clockwise (Direct3D): reversed
            surfaces.append((tex_index[key], verts, [remap[i] for t0, t1, t2 in tris for i in (t0, t2, t1)]))

    # a held model: its vertices placed where RE4 hangs it on the part in the rest pose (the parts' rest rotations are
    # the identity: the part's matrix is the sum of the parent chain's offsets), all on that part
    world = {}
    for bone_id, parent, _pad, x, y, z in body['bones']:
        px, py, pz = world.get(parent, (0.0, 0.0, 0.0))
        world[bone_id] = (px + x, py + y, pz + z)
    for model_entry, tpl_entry, bone_id, offset, rot in held:
        b = read_bin(ent[model_entry])
        refs = tpl_refs(ent[tpl_entry])
        m = held_matrix(rot)
        base = tuple(world[bone_id][k] + offset[k] for k in range(3))

        def place(v, origin):
            return tuple(sum(m[r][c] * v[c] for c in range(3)) + origin[r] for r in range(3))
        for mat, tris in b['faces']:
            if not tris:
                continue
            key = (tpl_entry, mat[12], mat[14])
            if key not in tex_index:
                colour = image(refs, mat[12])
                img = Image.open(io.BytesIO(colour)).convert('RGBA') if colour is not None else \
                    Image.new('RGBA', (4, 4), (160, 160, 160, 255))
                tex_index[key] = len(textures)
                textures.append((f'leon_t{tpl_entry}_d{mat[12]}_o{mat[14]}', img, False))
            used = sorted({i for t in tris for i in t})
            remap = {old: new for new, old in enumerate(used)}
            verts = [(to_bo1(place(b['pos'][i], base)), to_bo1_dir(place(b['nrm'][i], (0.0, 0.0, 0.0))), b['uv'][i],
                      [(bone_id + 1, 1.0)]) for i in used]
            surfaces.append((tex_index[key], verts, [remap[i] for t0, t1, t2 in tris for i in (t0, t2, t1)]))

    # one surface per texture (fewer draw calls), split only past the u16 vertex limit
    merged = []
    for tex in range(len(textures)):
        cur_v, cur_i = [], []
        for t, verts, idx in surfaces:
            if t != tex:
                continue
            if len(cur_v) + len(verts) > 0xFFFF:
                merged.append((tex, cur_v, cur_i))
                cur_v, cur_i = [], []
            base = len(cur_v)
            cur_v += verts
            cur_i += [base + i for i in idx]
        if cur_v:
            merged.append((tex, cur_v, cur_i))
    surfaces = merged

    out = bytearray(b'RLM1' + struct.pack('<IIII', 2, len(textures), len(surfaces), len(bones)))
    for name, img, masked in textures:
        img = capped(img, max_texture)
        levels = mips_bgra(img)
        out += name.encode()[:31].ljust(32, b'\0')
        out += struct.pack('<HHBBH', img.size[0], img.size[1], len(levels), int(masked), 0)
        for level in levels:
            out += level
    for name, parent, trans, quat in bones:
        out += name.encode()[:31].ljust(32, b'\0') + struct.pack('<i3f4f', parent, *trans, *quat)
    for tex, verts, idx in surfaces:
        out += struct.pack('<III', tex, len(verts), len(idx) // 3)
        for p, n, uv, weights in verts:
            w = weights + [(0, 0.0)] * (4 - len(weights))
            out += struct.pack('<8f4H4f', *p, *n, *uv, *(bone for bone, _ in w), *(wt for _, wt in w))
        out += struct.pack(f'<{len(idx)}H', *idx)
        while len(out) % 4:
            out.append(0)
    if motions:
        import re4_motion
        model = re4_motion.uhd_model(body)
        rest = [part.pos for part in model.parts]
        # (name, data[, frame limit or (first frame, end frame - None: the motion's end)])
        def frames_of(m):
            if len(m) < 3:
                return 0, None
            return (m[2][0], m[2][1]) if isinstance(m[2], tuple) else (0, m[2])
        anims = [bake_motion(model, m[1], rest, m[0], first_frame=frames_of(m)[0], max_frames=frames_of(m)[1],
                             tags=tags, root_move=m[0] in root_move, part_count=len(kept['bones']))
                 for m in motions]
        out += anim_section(anims)
        print('baked', ', '.join(f'{n} ({len(fr)} frames, {sp:.0f} in/s)' for n, _f, sp, fr in anims))
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(out)
    n_tris = sum(len(i) // 3 for _t, _v, i in surfaces)
    print(f'wrote {out_path}: {len(textures)} textures, {len(surfaces)} surfaces, {n_tris} triangles, {len(bones)} bones'
          f'{" (A-pose test)" if apose else ""}, {len(out)} bytes')


# ---- animations (optional section after the surfaces) --------------------------------------------------------
#   'ANM2', u32 animation count, then per animation: char name[32], u32 frame count, f32 frames per second,
#   u32 bone count (the model's), f32 move speed (inches per second the motion covers at its own rate; 0 in place),
#   frame count x bone count x {f32 local rotation quaternion[4] (x, y, z, w), f32 local translation offset from
#   the rest pose[3] (inches)}. Bone 0 (tag_origin) stays at the origin; RE4 bone id N is bone N + 1. Baked with the
#   game's own motion code and IK (re4_motion.py, the decomp's host build); the cycles play in place (the model's
#   root motion is the move speed).
#
# Leon's unarmed set is the weapon archive wep00's (cObjHand::setMotion: m_MotTbl[0] idle = archive index 4,
# [2] walk = 5, [8] back step = 6, [6] run = 7). An archive index counts the 4-word header: sub-file (udas entry)
# index - 4.
WEP00_MOTIONS = [('idle', 0), ('walk', 1), ('back', 2), ('run', 3)]
# Armed: the handgun's set (wep01, cObjFn57::setMotion: [0] idle = 0x0B, [2] walk = 0x11, [8] back = 0x13, [6] run
# = 0x15), and wep01's right hand gripping the gun (model 6, textured with Leon's own hand texture, TPL 13).
WEP01_MOTIONS = [('idle', 7), ('walk', 13), ('back', 15), ('run', 17),
                 # the aim (wep02_r3_set00: 0x26 level, 0x27 up, 0x28 down, blended by the aim pitch)
                 ('aim', 34), ('aim_up', 35), ('aim_down', 36),
                 # a shot (wep02_r3_fire00: 0x29 level, 0x2A up, 0x2B down, by the aim pitch) and the reload
                 # (wep02_r2_reload: 0x2D, the base reload speed)
                 ('fire', 37), ('fire_up', 38), ('fire_down', 39), ('reload', 41),
                 # the quick turn (pl_R1_Turn180: m_MotTbl[0xB] = 0x17), its root yaw kept (ROOT_YAW_MOTIONS)
                 ('turn180', 19),
                 # the turn in place (pl_R1_Turn: m_MotTbl[0xD] left = 0x0D, [0xF] right = 0x0F), looped while he turns
                 # standing; its root yaw (175.6 degrees in 40 frames) left out: the engine turns him at that pace
                 ('turn_left', 9), ('turn_right', 11)]
# Motions whose root yaw (the root rotation keys about RE4's up axis) is baked into bone 0 (tag_origin), which the
# others leave at identity: the engine reads it back to turn the player along it (R_RuntimeXModel_RootYaw)
ROOT_YAW_MOTIONS = {'turn180'}
WEP01_HAND = (6, 13, 'right_hand_grip')
# The other playable characters (--character): their own archive's models - (model entry, TPL entry, role), from each
# one's setModel / setRightHand / setLeftHand in the RE4 decomp (tools/motion/character.py; Luis: pl14/pl14.cpp
# cPl14::setModel) - and Leon's motions on their bodies (every RE4 player shares the skeleton's numbering), but for
# their own knife, hit reactions and death (their archive's, at Leon's entries: the shared pl_knife / pl_dmg code).
# The kick is em10's for all. They keep their own hands: Leon's gripping hand is wep01's, in his hand texture.
CHARACTERS = {
    'leon': {'parts': LEON_BO1, 'grip': True},
    'ada': {'parts': [(0, 1, 'body'), (2, 3, 'hair'), (4, 3, 'head'), (5, 6, 'eyes'), (13, 1, 'right_hand'),
                      (14, 1, 'left_hand')], 'grip': False},
    'hunk': {'parts': [(0, 1, 'body'), (2, 3, 'mask'), (14, 13, 'right_hand'), (17, 13, 'left_hand')], 'grip': False,
             'eyelid': False,
             # (Leon's second kick, not his own neck-breaker variant: an archive named outright is not the character's)
             'actions': {'kick2': ('pl00.udas', 0x21)}},
    'luis': {'parts': [(0, 1, 'body'), (2, 3, 'head'), (4, 3, 'face'), (5, 1, 'coat'), (6, 1, 'right_hand'),
                      (7, 1, 'left_hand')], 'grip': False,
             # (a partner, never the player: his own fall to the floor, entry 36, as long as Leon's death; his
             # archive has no standing flinch - its hits knock him down, which played on the upper body flailed - nor
             # a knife: Leon's)
             'actions': {'death': 36, 'kick2': ('pl00.udas', 0x21)}},
}
# The other weapons' sets, baked next to the handgun's as "<set>:<name>" (the engine picks a set by the BO1 weapon's
# class, CG_RuntimeXModel_AnimSet, and falls back to the handgun's motion a set lacks): (set, archive, motions) with
# the same names as WEP01_MOTIONS, sub-file numbers from each weapon's setMotion and state code in the RE4 decomp.
# Actions any weapon set shares (unprefixed): (name, archive, sub-file)
ACTION_MOTIONS = [
    # the knife slash (wep/pl_knife.cpp: the player archive's 0x82 level / 0x84 up / 0x86 down, by the pitch)
    ('knife', 'pl00.udas', 126), ('knife_up', 'pl00.udas', 128), ('knife_down', 'pl00.udas', 130),
    # the hand grenade's throw (wep/pl_grenade.cpp fire00, archive wep19: 0x12 level)
    ('throw', 'wep19.udas', 14),
    # its ready pose (pl_grenade.cpp set00: 0x11 level), held while BO1's grenade is readied / cooked
    ('throw_ready', 'wep19.udas', 13),
    # hit reactions and the death (game/pl_dmg.cpp damageNormal: the player archive's 0x4A middle front, 0x4B middle
    # back; Pl_R0_Die 0x4C, its last frame held)
    ('hit_front', 'pl00.udas', 0x46), ('hit_back', 'pl00.udas', 0x47), ('death', 'pl00.udas', 0x48),
    # the kick on a staggered Ganado (em10/em10.cpp plem10Kick: the Ganado's archive 0x29D, Leon's roundhouse)
    ('kick', 'em10.udas', 0x299),
    # its other one: RE4 plays the player's own archive's 0x25 three kicks in four (plem10Kick: r_no_3 = Rnd() & 3) -
    # Leon's second roundhouse, Ada's fan kick, HUNK's own; the same hit window (frames 17 to 35) and turn as the
    # first, so the kick script's timing stands for both (maps\leon\_kick.gsc picks one each kick)
    ('kick2', 'pl00.udas', 0x21),
]
WEAPON_SETS = [
    # the magnum: the Handcannon (wep15: weapon 0xF, item 0x37 - game/item.cpp; cObjMagnum::setMotion: the handgun's
    # motion table and state code, PlHandgunMove, from its own archive: the heavy shot and its round-by-round reload)
    ('magnum', 'wep15.udas', list(WEP01_MOTIONS) + [
        # its reload in BO1's stages for a round-by-round reload (bSegmentedReload): the left hand opens and empties
        # the cylinder and fetches the first round (to 68), seats a round cylinder to cylinder (67 to 80: its ends
        # match), then the last and the close. Start / loop / end (R_RuntimeXModel_PoseDObj, RT_ACTION_RELOAD_*)
        ('reload_start', 41, (0, 68)), ('reload_loop', 41, (67, 80)), ('reload_end', 41, (79, None))]),
    # the Broken Butterfly, a revolver (wep05: cObjCivilian::setMotion wep05/objCivilian.cpp, the handgun's states):
    # the two-handed magnum stance, its round-by-round reload staged like the Handcannon's (one round 67 to 79)
    ('revolver', 'wep05.udas', list(WEP01_MOTIONS) + [
        ('reload_start', 41, (0, 68)), ('reload_loop', 41, (67, 79)), ('reload_end', 41, (78, None))]),
    # the Killer7 (wep06: cObjGovernment::setMotion wep06/wep06.cpp, the handgun's table): the two-handed stance, a 23f
    # shot, a magazine reload - the Colt Government (BO1's M1911)
    ('government', 'wep06.udas', list(WEP01_MOTIONS)),
    # the Striker (wep08: cObjStriker::setMotion wep08/wep08.cpp, pl_shotgun.cpp's states; its shot ends at 32f): an
    # automatic shotgun loaded shell by shell - shells in at ~26, 31, 41 (start / one shell 30 to 41 / end)
    ('striker', 'wep08.udas', [('idle', 6), ('walk', 12), ('back', 14), ('run', 16), ('turn180', 18),
                               ('turn_left', 8), ('turn_right', 10),
                               ('aim', 22), ('aim_up', 28), ('aim_down', 30),
                               ('fire', 26, 32), ('fire_up', 29, 32), ('fire_down', 31, 32), ('reload', 38),
                               ('reload_start', 38, (0, 31)), ('reload_loop', 38, (30, 41)),
                               ('reload_end', 38, (40, None))]),
    # the semi-auto rifle (wep10: cObjHkSniper::setMotion wep10/objHkSniper.cpp, pl_rifle.cpp's states: the scope idle
    # 0x15 for the aim, a shot restarts it; reload 0x17, a magazine)
    ('semi', 'wep10.udas', [('idle', 10), ('walk', 13), ('back', 14), ('run', 15), ('turn180', 20),
                            ('turn_left', 11), ('turn_right', 12),
                            ('aim', 17), ('aim_up', 17), ('aim_down', 17),
                            ('fire', 17, 8), ('fire_up', 17, 8), ('fire_down', 17, 8), ('reload', 19)]),
    # the TMP (wep11: cObjMachinegun::setMotion wep/objMachinegun.cpp, pl_machine.cpp: aim 0x1B/0x1F/0x21, fire
    # 0x1C/0x20/0x22, reload 0x1E, a magazine)
    ('tmp', 'wep11.udas', [('idle', 9), ('walk', 14), ('back', 16), ('run', 18), ('turn180', 20),
                           ('turn_left', 10), ('turn_right', 12),
                           ('aim', 23), ('aim_up', 27), ('aim_down', 29),
                           ('fire', 24), ('fire_up', 28), ('fire_down', 30), ('reload', 26)]),
    # the mine thrower (wep14: cObjMine::setMotion wep14/objMine.cpp, wep14/wep14.cpp: aim 0x13/0x17/0x19, fire
    # 0x14/0x18/0x1A with the left hand racking it, reload 0x16)
    ('mine', 'wep14.udas', [('idle', 7), ('walk', 10), ('back', 11), ('run', 12), ('turn180', 13),
                            ('turn_left', 8), ('turn_right', 9),
                            ('aim', 15), ('aim_up', 19), ('aim_down', 21),
                            ('fire', 16), ('fire_up', 20), ('fire_down', 22), ('reload', 18)]),
    # the shotgun (cObjShotgun::setMotion, pl_shotgun.cpp: aim set00 0x1A/0x20/0x22, fire00 0x1E/0x21/0x23 - recoil and
    # pump in one - reload 0x2A)
    ('shotgun', 'wep07.udas', [('idle', 6), ('walk', 12), ('back', 14), ('run', 16), ('turn180', 18),
                               ('turn_left', 8), ('turn_right', 10),
                               ('aim', 22), ('aim_up', 28), ('aim_down', 30),
                               ('fire', 26), ('fire_up', 29), ('fire_down', 31), ('reload', 38),
                               # its reload in BO1's stages (shell by shell: Stakeout, SPAS, HS10): the gun up and a
                               # shell fetched and loaded (to 26), a shell fetched and loaded (26 to 44), then the
                               # close: the left hand back on the fore-end (54 to 62), the pump - pulled 64 to 68,
                               # pushed 68 to 74 (the hands' distance, measured in the motion) - and the hold. The
                               # close started at 68 once: only the push showed, half a pump.
                               ('reload_start', 38, (0, 27)), ('reload_loop', 38, (26, 45)),
                               ('reload_end', 38, (54, None))]),
    # the bolt-action rifle (cObjSniper::setMotion, pl_rifle.cpp): its aim is the scope's single idle 0x15 (no pitch
    # blend: the aim correction turns it) and a shot only restarts it, so it stands for all of them; reload 0x17
    ('rifle', 'wep09.udas', [('idle', 10), ('walk', 13), ('back', 14), ('run', 15), ('turn180', 20),
                             ('turn_left', 11), ('turn_right', 12),
                             ('aim', 17), ('aim_up', 17), ('aim_down', 17),
                             # (a shot: its first 8 frames, so the laser - off through a shot - is back soon)
                             ('fire', 17, 8), ('fire_up', 17, 8), ('fire_down', 17, 8), ('reload', 19)]),
    # the Chicago Typewriter (cObjTompson::setMotion, pl_machine.cpp as the TMP's: aim 0x1B/0x1F/0x21, fire - one
    # round - 0x1C/0x20/0x22, reload 0x1E)
    ('smg', 'wep12.udas', [('idle', 9), ('walk', 14), ('back', 16), ('run', 18), ('turn180', 20),
                           ('turn_left', 10), ('turn_right', 12),
                           ('aim', 23), ('aim_up', 27), ('aim_down', 29),
                           ('fire', 24), ('fire_up', 28), ('fire_down', 30), ('reload', 26)]),
    # the Death Machine (BO1's minigun, held at the hip): the shotgun's stance - its left hand on the fore-end sits on
    # the minigun's grip; the Typewriter's left the hand up in the air - with the Typewriter's shot (the shotgun's
    # pumps); two entries of one set: the engine takes a motion's first by name
    ('minigun', 'wep07.udas', [('idle', 6), ('walk', 12), ('back', 14), ('run', 16), ('turn180', 18),
                               ('turn_left', 8), ('turn_right', 10),
                               ('aim', 22), ('aim_up', 28), ('aim_down', 30)]),
    ('minigun', 'wep12.udas', [('fire', 24), ('fire_up', 28), ('fire_down', 30)]),
    # the rocket launcher (cObjLauncher::setMotion, pl_rocket.cpp: aim 0x0F/0x12/0x14, fire 0x11/0x13/0x15); single
    # shot in RE4, so no reload: shouldering it (0x18) stands in for BO1's
    ('launcher', 'wep13.udas', [('idle', 4), ('walk', 5), ('back', 6), ('run', 7), ('turn180', 8),
                                ('turn_left', 9), ('turn_right', 10),
                                ('aim', 11), ('aim_up', 14), ('aim_down', 16),
                                ('fire', 13), ('fire_up', 15), ('fire_down', 17), ('reload', 20)]),
]

# a double barrel (BO1's Olympia): the shotgun's set with its shot's kick only (to frame 18: the pump, 18 to 30, is not a
# break action's), its blend out bringing the gun back down
WEAPON_SETS.append(('dbshotgun', 'wep07.udas',
                    [m for m in next(ms for n, _a, ms in WEAPON_SETS if n == 'shotgun') if not m[0].startswith('fire')]
                    + [('fire', 26, 18), ('fire_up', 29, 18), ('fire_down', 31, 18)]))

# The enemies (--enemy, the archive given is the enemy's): their own model, what they hold and their own motions (sub-file
# numbers: RE4's archive index - 4). Super Salvador, the island chainsaw carrier (em20/em20_set.cpp Em20Set type 22: one
# model with the sack, BIN 0x1E0 / TPL 0x1E1; Em20WeaponSet's chainsaw, Wep_type 4 - em10WeaponInit - BIN 0x104 / TPL
# 0x105 on the right hand, part 0xA, em10WeaponSet's place for it). His motions are em10.cpp's for the chainsaw: the
# idle (em10SetWaitMotion 0xE6); for every step he takes, the Mercenaries' chainsaw walk, the saw swung round and round
# (CSawWalkAtk 0x2BC); the kill (C_SawCriHit 0x100), the flinch (Dm_Small's chainsaw 0xF7) and the standing death
# (Die_Normal 0x67, its fall forward baked in). A quarter larger than RE4's, to stand out; tags on the saw's engine and
# its blade for his smoke and sparks (maps\leon\_salvador.gsc). The textures at half the characters' cap: an enemy seen
# from a distance.
ENEMIES = {
    'salvador': {'parts': [(0x1DC, 0x1DD, 'body')],
                 'held': [(0x100, 0x101, 10, (-304.24, -17.86, -57.43), (1.4835298, 0.0, -1.5707964))],
                 'motions': [('idle', 0xE2), ('walk', 0x2B8), ('kill', 0xFC), ('stagger', 0xF3), ('death', 0x63)],
                 'tags': [('tag_saw_engine', 10, (-92.0, -45.0, -31.0), None),
                          ('tag_saw_blade', 10, (-700.0, -100.0, 0.0), None)],
                 'root_move': {'death'}, 'max_texture': 1024, 'scale': 1.25},
    # Verdugo, the castle's insect boss (em2c/em2c.cpp: cEmMgr's "INSECT BOSS", id 0x2C): his body, BIN 0x005 / TPL
    # 0x006, carries his tail (parts 0x39..0x4C; BIN 0x086 is the tail alone, for its hiding in the water); his skeleton
    # is no human's, so no bone takes a BO1 name, and of his 201 bones those past 90 (his cloth and claws: no motion
    # keys them) fold into their parents. His motions: the idle (Wait 0x007), walk (Walk 0x008) and the dash
    # (Dash 0x00A), the claw slash (Atk 0x015, its hit frames 16 to 25), the tail sweep (TailAtk 0x01B: its dodge
    # window, RE4's action button, frames 5 to 30, its hit frames 44 to 78), the roar he taunts with (Threat 0x044),
    # the flinch (Dm_Normal 0x01D) and the standing death (Die_Normal 0x090, its fall forward baked in).
    'verdugo': {'parts': [(0x001, 0x002, 'body')], 'held': [], 'names': {}, 'max_bone': 90,
                'motions': [('idle', 0x003), ('walk', 0x004), ('run', 0x006), ('claw', 0x011), ('tail', 0x017),
                            ('taunt', 0x040), ('stagger', 0x019), ('death', 0x08C)],
                'root_move': {'death'}, 'max_texture': 1024},
}


def quat_to_bo1(q):
    x, y, z, w = q
    return (z, x, y, w)


def bake_motion(model, motion_bytes, rest, name, fps=30.0, max_frames=None, first_frame=0, tags=TAGS, root_move=False,
                part_count=None):
    """root_move: the root keys' travel (RE4 moves the whole model by them) baked into the root part's offset - a
    motion that leaves the entity where it is (an enemy's fall, played where he dropped). part_count: the parts the
    model keeps (the first ones: build's max_bone), all of them evaluated."""
    import re4_motion
    p = re4_motion.player(model, motion_bytes, loop=True)
    frames = int(p.lib.mot_model_max_frame(p.handle))
    if max_frames:
        frames = min(frames, max_frames)
    out, root = [], []
    for f in range(first_frame, frames):
        pose = p.frame(f)
        root.append(pose.root_pos)
        bones = [((0.0, 0.0, 0.0, 1.0), (0.0, 0.0, 0.0))]  # tag_origin
        if name.split(':')[-1] in ROOT_YAW_MOTIONS:
            # RE4's up axis Y is BO1's Z (to_bo1 cycles the axes, so the turn's sense is kept)
            yaw = pose.root_rot[1]
            bones[0] = ((0.0, 0.0, math.sin(yaw * 0.5), math.cos(yaw * 0.5)), (0.0, 0.0, 0.0))
        for i in range(part_count or model.n_parts):
            m = pose.l_mat[i]
            # (the root rotation keys are not in the root part's matrix - RE4 applies them on top, as the engine turns
            # the player here; taking them out of it, R^T m, turned him back against the view through the whole turn)
            q = quat_to_bo1(p.quat_from_matrix(m))
            local = (m[3], m[7], m[11])
            travel = pose.root_pos if root_move and i == 0 else (0.0, 0.0, 0.0)
            offset = to_bo1(tuple(local[k] - rest[i][k] + travel[k] for k in range(3)))
            bones.append((q, offset))
        bones += [(tag_quat(axes), (0.0, 0.0, 0.0)) for _n, _p, _o, axes in tags]  # fixed on their parents
        out.append(bones)
    # the root keys' distance over the cycle (the last frame is one step short of the loop)
    frames = len(out)
    dist = math.dist(root[0], root[-1]) * frames / max(1, frames - 1) if frames > 1 else 0.0
    speed = dist * _scale / MM_PER_INCH / (frames / fps) if frames else 0.0
    return name, fps, speed, out


def anim_section(anims):
    out = bytearray(b'ANM2' + struct.pack('<I', len(anims)))
    for name, fps, speed, frames in anims:
        out += name.encode()[:31].ljust(32, b'\0') + struct.pack('<IfIf', len(frames), fps, len(frames[0]), speed)
        for bones in frames:
            for q, t in bones:
                out += struct.pack('<7f', *q, *t)
    return bytes(out)


def build_enemy(udas, pack_path, out_path, name):
    spec = ENEMIES[name]
    arc = {i: data for i, _t, data in entries(open(udas, 'rb').read())}
    build(udas, pack_path, out_path, parts=spec['parts'], motions=[(m, arc[entry]) for m, entry in spec['motions']],
          eyelid=False, tags=spec.get('tags', ()), held=spec['held'], max_texture=spec['max_texture'],
          root_move=spec['root_move'], scale=spec.get('scale', 1.0), names=spec.get('names', BO1_BONE_NAMES),
          max_bone=spec.get('max_bone'))


def main():
    # (a return, not sys.exit: leon_setup.py runs this in its own process - runpy - and an exit ended the whole setup
    # after the first enemy, its sounds and its ready file never written)
    if '--enemy' in sys.argv:
        build_enemy(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[sys.argv.index('--enemy') + 1])
        return
    character = sys.argv[sys.argv.index('--character') + 1] if '--character' in sys.argv else 'leon'
    spec = CHARACTERS[character]
    parts, motions = spec['parts'], ()
    here = os.path.dirname(os.path.abspath(sys.argv[1]))
    own = os.path.basename(sys.argv[1]) # (the character's own archive: its knife, hits and death)
    if '--armed' in sys.argv[4:]:
        wep = os.path.join(here, 'wep01.udas')
        if spec['grip']:
            hand = WEP01_HAND + (wep,)
            parts = [hand if role == 'right_hand' else (entry, tpl, role) for entry, tpl, role in parts]
        arc = {i: data for i, _t, data in entries(open(wep, 'rb').read())}
        motions = [(name, arc[entry]) for name, entry in WEP01_MOTIONS]
        archives = {}
        for name, archive, entry in ACTION_MOTIONS:
            archive = own if archive == 'pl00.udas' else archive
            entry = spec.get('actions', {}).get(name, entry)
            if isinstance(entry, tuple):  # (archive, entry): that archive itself, Leon's own included
                archive, entry = entry
            path = os.path.join(here, archive)
            if not os.path.exists(path):
                print(f'{name}: no {archive}, skipped')
                continue
            if archive not in archives:
                archives[archive] = {i: data for i, _t, data in entries(open(path, 'rb').read())}
            if entry not in archives[archive] and archive == own != 'pl00.udas':
                # (not in the character's own archive - Luis's has no player actions: Leon's)
                archive = 'pl00.udas'
                if archive not in archives:
                    archives[archive] = {i: data for i, _t, data in entries(open(os.path.join(here, archive), 'rb').read())}
                print(f"{name}: not in {own}, Leon's")
            motions.append((name, archives[archive][entry]))
        for set_name, archive, set_motions in WEAPON_SETS:
            path = os.path.join(here, archive)
            if not os.path.exists(path):
                print(f'{set_name}: no {archive} next to {sys.argv[1]}, skipped (the handgun set stands in)')
                continue
            set_arc = {i: data for i, _t, data in entries(open(path, 'rb').read())}
            motions += [(f'{set_name}:{m[0]}', set_arc[m[1]]) + tuple(m[2:]) for m in set_motions]
    elif '--anims' in sys.argv[4:]:
        arc = {i: data for i, _t, data in entries(open(os.path.join(here, 'wep00.udas'), 'rb').read())}
        motions = [(name, arc[entry]) for name, entry in WEP00_MOTIONS]
    build(sys.argv[1], sys.argv[2], sys.argv[3], parts=parts, apose='--apose' in sys.argv[4:], motions=motions,
          eyelid=spec.get('eyelid', True))


if __name__ == '__main__':
    main()
