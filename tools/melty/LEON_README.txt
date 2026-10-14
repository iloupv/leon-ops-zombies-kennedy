Leon Ops: Zombies Kennedy - Resident Evil 4's Leon in Call of Duty: Black Ops Zombies
https://github.com/iloupv/leon-ops-zombies-kennedy

You need both games, your own copies:
  - Call of Duty: Black Ops (Steam) - the game itself, read from your install
  - Resident Evil 4 (2005, Ultimate HD Edition, Steam) - Leon's model, textures, animations and voice are built
    from it on your PC

Nothing from either game is in this package. On the first start, leon_setup reads your Resident Evil 4 and writes
Leon into mods\leon (rtmodels, sound), and copies the DirectX 9 runtime your Black Ops installs from its own
Redist folder. Neither game's folder is changed. leon_setup.log says what it did.

To play:
  - with Melty (https://melty.gg/m/leon-zombies-kennedy): press Play
  - without Melty: run LeonLauncher.exe. It finds both games in your Steam libraries (or asks for their folders
    once), builds Leon the first time and starts the game.

What is here:
  BO1Zombies.exe       Black Ops' engine rebuilt from a community decompilation, with the Leon mod's engine changes
  LeonLauncher.exe     the launcher for playing without Melty
  steam_api.dll        Steamworks SDK redistributable
  binkw32.dll          RAD Bink video (the decompilation project's copy)
  msvcp140.dll,
  vcruntime140.dll     Microsoft Visual C++ runtime
  main\                the engine's own data (offline playlists)
  mods\leon\           the mod's scripts and settings
  leon_setup\          the converter (Python, frozen), lfsunpack (libmspack's LZX) and the motion code of the
                       Resident Evil 4 decompilation, to play RE4's motions on Leon as the game does

Credits: MisaDev4/bo1-zombies-decompiled (the Black Ops engine), adonis-singh/re4 (the Resident Evil 4 GameCube
decompilation), JADERLINK's RE4-UHD-PACK-TOOL and RE4-UHD-BIN-TPL-TOOLS, emoose's re4-research, libmspack.

Resident Evil 4 and Leon S. Kennedy belong to Capcom; Call of Duty: Black Ops to Activision. This is a fan mod,
not affiliated with either. The project's own code is MIT (LICENSE.txt).
