@echo off
rem Black Ops 1 Zombies with the Leon mod (mods\leon): the player as Resident Evil 4's Leon, in third person over his
rem right shoulder, with Resident Evil 4's controls, aim and laser sight, on any zombie map. Needs
rem mods\leon\rtmodels\leon.rlm, converted from your own Resident Evil 4 (see mods\leon).
rem Usage: PlayLeon.bat [map]   (no map: the game opens at the zombies map list, with Settings over Back;
rem                              a map, e.g. PlayLeon.bat zombie_pentagon: straight into it)
setlocal
set "GAME=%~dp0build\Release"
if not exist "%GAME%\BO1Zombies.exe" (echo Game not found in %GAME% & pause & exit /b 1)
if not exist "%GAME%\mods\leon\rtmodels\leon.rlm" (echo Leon not converted: %GAME%\mods\leon\rtmodels\leon.rlm & pause & exit /b 1)
set "LEON=+set fs_game mods/leon +set bo1_rtmodels leon +set leon_thirdperson 1 +set player_tankControls 1 +set cg_thirdPersonShoulder 18 +set cg_thirdPersonShoulderUp 6 +set cg_thirdPersonRange 42 +set cg_drawVersion 0 +set bo1_noloadhints 1"
set "START=+set bo1_frontend_menu levels_zombie"
if not "%~1"=="" set "START=+devmap %~1"
rem The window (borderless over the screen) only on the first run: after it, the game's own display settings, saved
rem in players\mods\leon\config.cfg
set "WINDOW="
if not exist "%GAME%\players\mods\leon\config.cfg" set "WINDOW=+set r_fullscreen 0 +set r_borderless 1 +set vid_xpos 0 +set vid_ypos 0"
cd /d "%GAME%"
start "" "%GAME%\BO1Zombies.exe" +set bo1_zombies 1 +set fs_b "%GAME%" +set fs_h "%GAME%" +set logfile 2 +set net_ip 127.0.0.1 %WINDOW% %LEON% %START%
