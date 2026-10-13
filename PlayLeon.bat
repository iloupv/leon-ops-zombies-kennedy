@echo off
rem Black Ops 1 Zombies with the Leon mod (mods\leon): Kino der Toten, the player as Resident Evil 4's Leon, in
rem third person over his right shoulder, with Resident Evil 4's controls, aim and laser sight. Needs mods\leon\rtmodels\leon.rlm,
rem converted from your own Resident Evil 4 (see mods\leon).
set "GAME=%~dp0build\Release"
if not exist "%GAME%\BO1Zombies.exe" (echo Game not found in %GAME% & pause & exit /b 1)
if not exist "%GAME%\mods\leon\rtmodels\leon.rlm" (echo Leon not converted: %GAME%\mods\leon\rtmodels\leon.rlm & pause & exit /b 1)
cd /d "%GAME%"
start "" "%GAME%\BO1Zombies.exe" +set bo1_zombies 1 +set fs_b "%GAME%" +set fs_h "%GAME%" +set logfile 2 +set net_ip 127.0.0.1 +set r_fullscreen 0 +set r_borderless 1 +set vid_xpos 0 +set vid_ypos 0 +set fs_game mods/leon +set bo1_rtmodels leon +set leon_thirdperson 1 +set player_tankControls 1 +set cg_thirdPersonShoulder 18 +set cg_thirdPersonShoulderUp 6 +set cg_thirdPersonRange 42 +devmap zombie_theater
