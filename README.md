# Leon Ops: Zombies Kennedy

Resident Evil 4's Leon S. Kennedy in Call of Duty: Black Ops Zombies, played the RE4 way. A fan mod: you bring your
own copies of both games, and Leon is built from your Resident Evil 4 on your own PC. Nothing from either game is in
this repository or its releases.

- Over-the-shoulder camera, RE4 tank controls (turning on the spot with RE4's own step-turn), RE4's quick 180 turn,
  and a sidestep while aiming
- Laser sight with RE4's own aim sway (from RE4's aiming code); standing still, the mouse swings the camera around
  Leon, and aiming turns him to where it looks
- RE4's animations for every kind of gun (handgun, Broken Butterfly, shotgun / Striker reloaded shell by shell,
  rifle, Chicago Typewriter, TMP, rocket launcher, the mine thrower for the wonder weapons), played with RE4's own
  motion code; a new gun from the wall or the box is loaded on the spot
- Knife slash and grenades the RE4 way; stagger a zombie with a head or leg shot (it glows faintly yellow), then
  roundhouse kick it - every zombie the kick catches is a knife kill
- RE4's Merchant at the Mystery Box: "What're ya buyin'?", and a line for the weapon it gives you ("Stranger,
  stranger, now that's a weapon!" for the wonder weapons)
- Leon's hit reactions, death, pain grunts and voice; his head tracks nearby zombies; RE4's pickup sound; more blood
- Gamepad support with RE4's layout
- The astronaut helmet on Moon; boots straight to the zombies map list
- Single player

**Controls**

| | Keyboard / mouse | Gamepad |
| --- | --- | --- |
| Walk / turn | W S / A D | Left stick |
| Run | Shift (press once) | Hold A, or press L3 once |
| Quick 180 turn | S + Shift | Back + A |
| Aim / fire | Right / left mouse | LT / RT |
| Sidestep (aimed) | A D | Left stick |
| Knife | Space | B or R3 |
| Kick (staggered zombie), use, reload | F / R | X |

## Play it

You need, on Windows, **Call of Duty: Black Ops** (Steam app 42700) and **Resident Evil 4 (2005), Ultimate HD
Edition** (Steam app 254700). The base game has Kino der Toten and "Five"; the other zombie maps come with the Black
Ops map packs.

**With Melty:** get it from https://melty.gg/m/leon-zombies-kennedy and press Play. Melty installs it, builds Leon
the first time and starts it.

**Without Melty:** download the latest `LeonZombies-<version>.zip` from
[Releases](https://github.com/iloupv/leon-ops-zombies-kennedy/releases), unzip it anywhere and run
`LeonLauncher.exe`. It finds both games in your Steam libraries (or asks for their folders once), builds Leon from
your Resident Evil 4 the first time (a few seconds) and starts the game. Neither game's folder is changed.

## How it works

`BO1Zombies.exe` is the Black Ops engine rebuilt from a decompilation, with this mod's changes. It runs from its own
folder and reads your Black Ops install (`BO1Z_GAME_DIR`). The mod lives in `mods/leon` (scripts) and the engine
(`src/gfx_d3d/r_runtime_xmodel.cpp` builds Leon's model at level load; `src/cgame_mp`, `src/client_mp` and
`src/bgame` hold the camera, controls, aiming and animation code). `tools/leon-converter` turns Leon's model,
textures, motions and voice from your Resident Evil 4 into `mods/leon/rtmodels/leon.rlm` and `mods/leon/sound`.
`tools/leon-launcher` is the launcher, `tools/melty` and `melty.json` the release package and Melty's install recipe.

## Build from source

1. Build the engine (Visual Studio 2022, 32-bit): `cmake -B build -A Win32`, then
   `cmake --build build --config Release --target BO1Zombies copy_mods`.
2. Build the converter's two native pieces (`tools/leon-converter/README.md`): `lfsunpack.exe` (with libmspack) and
   the RE4 decomp's motion host.
3. `python -m venv build\melty\venv`, `build\melty\venv\Scripts\pip install pyinstaller pillow`, then
   `powershell -File tools\melty\build_leon_package.ps1 -Version <version>` writes `build\melty\LeonZombies-<version>.zip`.

Not included here, though the decompilation project ships them: Valve's Steam client DLLs (the exe needs only the SDK's
`steam_api.dll`), and the retail-derived files of its `mods/_stack` and `mods/zinfo` examples (copies of Treyarch's
`maps/_zombiemode_ffotd.gsc` and entity placements from the retail maps). The Leon mod needs none of them; those two
example mods do (`tools/leon-publish/publish_public.sh` filters them out of this repository).

## Credits

- **Black Ops engine:** [MisaDev4/bo1-zombies-decompiled](https://github.com/MisaDev4/bo1-zombies-decompiled), the
  decompiled engine code for Black Ops 1 Zombies this project is built on.
- **Resident Evil 4 GameCube decompilation:** [adonis-singh/re4](https://github.com/adonis-singh/re4), whose motion
  code plays RE4's animations on Leon exactly as the game does.
- **Resident Evil 4 UHD tools:** JADERLINK's [RE4-UHD-PACK-TOOL](https://github.com/JADERLINK/RE4-UHD-PACK-TOOL) and
  [RE4-UHD-BIN-TPL-TOOLS](https://github.com/JADERLINK/RE4-UHD-BIN-TPL-TOOLS) (MIT), the reference for the UHD
  texture packs and model files.
- **RE4 .lfs archives:** emoose's [re4-research](https://github.com/emoose/re4-research) (the format) and
  [libmspack](https://github.com/kyz/libmspack) (LGPL 2.1, its LZX decoder).
- Resident Evil 4 and Leon S. Kennedy © Capcom. Call of Duty: Black Ops © Activision. A fan project, not
  affiliated with or endorsed by either.

## License

The project's own work is MIT ([LICENSE](LICENSE)); the decompiled engine, the credited projects and both games keep
their own terms.
