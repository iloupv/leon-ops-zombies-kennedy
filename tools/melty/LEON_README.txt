Leon: Zombies Kennedy - Resident Evil 4's Leon in Call of Duty: Black Ops Zombies

You need both games, your own copies:
  - Call of Duty: Black Ops (Steam) - the game itself, read from your install
  - Resident Evil 4 (2014, Steam) - Leon's model, textures, animations and voice are built from it on your PC

Nothing from either game is in this package. On the first start, leon_setup reads your Resident Evil 4 and writes
Leon into mods\leon (rtmodels, sound), and copies the DirectX 9 runtime your Black Ops installs from its own
Redist folder. Neither game's folder is changed. leon_setup.log says what it did.

What is here:
  BO1Zombies.exe       Black Ops' engine rebuilt from a community decompilation, with the Leon mod's engine changes
  steam_api.dll        Steamworks SDK redistributable
  binkw32.dll          RAD Bink video (the decompilation project's copy)
  msvcp140.dll,
  vcruntime140.dll     Microsoft Visual C++ runtime
  main\                the engine's own data (offline playlists)
  mods\leon\           the mod's scripts (the kick, the helmet, blood, sounds)
  leon_setup\          the converter (Python, frozen), lfsunpack (libmspack's LZX) and the motion code of the
                       Resident Evil 4 decompilation, to play RE4's motions on Leon as the game does

Started without Melty: set BO1Z_GAME_DIR to your Black Ops folder and run
  BO1Zombies.exe +set bo1_zombies 1 +set fs_game mods/leon +set bo1_rtmodels leon +set leon_thirdperson 1
    +set player_tankControls 1 +set bo1_frontend_menu levels_zombie
after leon_setup.exe --re4 "<Resident Evil 4 folder>" --bo1 "<Black Ops folder>" --out "<this folder>".

Resident Evil 4 and Leon S. Kennedy belong to Capcom; Call of Duty: Black Ops to Activision. This is a fan mod,
not affiliated with either.
