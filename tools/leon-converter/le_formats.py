"""Little-endian copies of the RE4 decomp's format readers (tools/motion/meshbin.py, modelbin.py,
gxtex.py), for the PC (UHD) data: the same layouts as the GameCube's with every field byte-swapped.
The decomp's files are read, never edited; each module's struct formats are switched from '>' to '<'."""
import os
import sys
import types


def load(decomp_root, name):
    path = os.path.join(decomp_root, 'tools', 'motion', name + '.py')
    src = open(path, encoding='utf-8').read().replace("'>", "'<").replace('">', '"<')
    # the PC build places the weight / blend / flip / shape tables before the vertices; the GameCube
    # order is only needed for byte-exact re-serialising, which the PC path does not do
    src = src.replace("raise ValueError(f'section order {order}, expected {expect}')", "pass")
    mod = types.ModuleType(name + '_le')
    mod.__file__ = path
    sys.modules[name + '_le'] = mod
    exec(compile(src, path + ' (little-endian)', 'exec'), mod.__dict__)
    if '_zero_pad' in mod.__dict__:
        # padding checks only guard byte-exact re-serialising; the PC headers carry extra words there
        mod._zero_pad = lambda *args, **kwargs: None
    return mod
