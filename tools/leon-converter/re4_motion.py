"""Evaluate RE4 motions with the game's own motion code (the decomp's tools/motion/host library, built for
Windows with winshim/) on Leon's PC (UHD) skeleton, and bake them for the BO1 runtime model.

The decomp's evalhost.Player wants a model with .n_parts, .parts (.parent, .pos) and .blend_table: the UHD
body bones give exactly that (id = parts number, parent 0xFF = the model, pos relative to the parent, mm).
Motions are the PC archive's FCV entries, little-endian as the host build reads them."""
import ctypes
import os
import sys
from dataclasses import dataclass

if getattr(sys, 'frozen', False):
    # the Melty setup program (leon_setup.py, PyInstaller): the decomp's motion tools and host DLL are in the bundle
    HOST_DLL = os.path.join(sys._MEIPASS, 'motion_host', 'libmotion_host.dll')
else:
    # the RE4 decomp checkout (https://github.com/adonis-singh/re4) with its motion host built (README.md here)
    DECOMP = os.environ.get('RE4_DECOMP', r'C:\dev\refs\re4')
    sys.path.insert(0, os.path.join(DECOMP, 'tools'))
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    msys = os.environ.get('MSYS2_UCRT64_BIN', r'C:\msys64\ucrt64\bin')
    if os.path.isdir(msys):
        os.add_dll_directory(msys)  # a host DLL built against MSYS2's shared libstdc++ / libgcc
    HOST_DLL = os.path.join(DECOMP, 'tools', 'motion', 'host', 'build', 'libmotion_host.so')
from motion import evalhost, fcv  # noqa: E402
from udas_reader import entries  # noqa: E402
from uhd_bin_gltf import read_bin  # noqa: E402

evalhost.build = lambda variant='plain': HOST_DLL  # built already (make, MSYS2 g++, winshim)


@dataclass
class Part:
    parent: int
    pos: tuple


@dataclass
class UhdModel:
    n_parts: int
    parts: list
    blend_table: list


def uhd_model(body):
    # parent 0xFF (the model) is -1 for the host: mot_model_create takes parent < 0 as the model
    parts = [Part(-1 if parent == 0xFF else parent, (x, y, z)) for _bone_id, parent, _pad, x, y, z in body['bones']]
    return UhdModel(len(parts), parts, [])


def load_leon(udas_path):
    ent = {i: (tag, data) for i, tag, data in entries(open(udas_path, 'rb').read())}
    body = read_bin(ent[0][1])
    motions = {i: data for i, (tag, data) in ent.items() if tag == 'FCV' and data}
    return body, uhd_model(body), motions


def player(model, motion_bytes, loop=True, ik=True):
    """ik: the game's arm / leg IK (the hands and toes reach the motion's targets; the floor is flat).
    The joints for parts the model lacks are left out: Leon's motions on a smaller skeleton (Ada, Luis) move his
    extra parts too."""
    motion = fcv.parse(motion_bytes, '<')
    keep = [i for i, j in enumerate(motion.joints) if j.target() is None or j.parts_no < model.n_parts]
    if len(keep) < len(motion.joints):
        index = {old: new for new, old in enumerate(keep)}
        motion.joints = [motion.joints[i] for i in keep]
        motion.layout = [index[i] for i in motion.layout if i in index]
    return evalhost.Player(model, motion, loop=loop, ik=ik)


if __name__ == '__main__':
    body, model, motions = load_leon(sys.argv[1])
    entry = int(sys.argv[2])
    m = fcv.parse(motions[entry], '<')
    p = player(model, motions[entry])
    print(f'motion entry {entry}: {len(m.joints)} joints, max frame {p.lib.mot_model_max_frame(p.handle):.1f}')
    for f in (0, 10, 20):
        pose = p.frame(f)
        print(f'frame {f}: root pos {tuple(round(v, 1) for v in pose.root_pos)} root rot {tuple(round(v, 3) for v in pose.root_rot)}'
              f'; part 7 local angles {tuple(round(v, 3) for v in pose.ang[7])}; part 9 world {tuple(round(v) for v in pose.world[9])}')
