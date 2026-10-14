# Leon converter

Builds Leon from the player's own Resident Evil 4 (2005, Ultimate HD Edition, Steam) for the mod: his model, textures,
RE4's motions (played with the game's own motion code, IK included) and his voice. Nothing of the game is stored here.

- `leon_setup.py` - what a player runs (frozen as `leon_setup.exe` in a release; Melty runs it as the install's setup
  step, `LeonLauncher.exe` the first time): unpacks the archives it needs, converts, writes
  `mods/leon/rtmodels/leon.rlm`, `mods/leon/sound/rt/*.wav` and `leon_ready.txt`.
  `leon_setup.py --re4 "<Resident Evil 4 folder>" --bo1 "<Black Ops folder>" --out "<mod install folder>"`
- `leon_rlm.py` - the model and the baked motions (`.rlm`, the format `src/gfx_d3d/r_runtime_xmodel.cpp` reads)
- `re4_motion.py` - evaluates RE4's motions with the RE4 decomp's motion host (`RE4_DECOMP`: its checkout)
- `leon_sounds.py` - Leon's voice and the pickup sound from RE4's XACT wave banks
- `uhd_pack.py`, `uhd_bin_gltf.py`, `udas_reader.py`, `leon_gltf.py`, `leon_full.py`, `le_formats.py` - RE4 UHD formats
- `preview.py`, `gltf_preview.py`, `rlm_skin_preview.py`, `udas_probe.py` - development previews and probes

## The two native pieces

**lfsunpack.exe** (`lfsunpack.c`): RE4 UHD's `.lfs` archives are XMemCompress (LZX) chunks. Build it with
[libmspack](https://github.com/kyz/libmspack)'s `lzxd.c` (LGPL 2.1), e.g. with MSYS2 UCRT64:
`M=libmspack/libmspack/mspack; gcc -O2 -I$M -o lfsunpack.exe lfsunpack.c $M/lzxd.c $M/system.c` (a checkout of the
repo at `libmspack/`).

**The motion host** (`libmotion_host.so`): [adonis-singh/re4](https://github.com/adonis-singh/re4)'s
`tools/motion/host`, built on Windows with MSYS2 g++ (`make`), with `winshim/sys/mman.h` here on its include path for
the two POSIX calls it uses. It is compiled from the decomp's reconstruction of the game's code, which its license says
stays Capcom's: it is not committed here; releases carry the built DLL inside `leon_setup`.

## Credits

The formats follow JADERLINK's [RE4-UHD-PACK-TOOL](https://github.com/JADERLINK/RE4-UHD-PACK-TOOL) and
[RE4-UHD-BIN-TPL-TOOLS](https://github.com/JADERLINK/RE4-UHD-BIN-TPL-TOOLS) (MIT) and emoose's
[re4-research](https://github.com/emoose/re4-research); the motions are played with
[adonis-singh/re4](https://github.com/adonis-singh/re4)'s motion code.
