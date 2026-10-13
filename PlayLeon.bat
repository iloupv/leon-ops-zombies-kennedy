@echo off
rem Black Ops 1 Zombies with the Leon mod (mods\leon): the player as Resident Evil 4's Leon, in third person over his
rem right shoulder, with Resident Evil 4's controls, aim and laser sight, on any zombie map. Needs
rem mods\leon\rtmodels\leon.rlm, converted from your own Resident Evil 4 (see mods\leon).
rem Usage: PlayLeon.bat [map]   (no map: a menu; e.g. PlayLeon.bat zombie_pentagon)
setlocal
set "GAME=%~dp0build\Release"
if not exist "%GAME%\BO1Zombies.exe" (echo Game not found in %GAME% & pause & exit /b 1)
if not exist "%GAME%\mods\leon\rtmodels\leon.rlm" (echo Leon not converted: %GAME%\mods\leon\rtmodels\leon.rlm & pause & exit /b 1)
set "MAP=%~1"
if not "%MAP%"=="" goto play
echo Leon mod - pick a map:
echo   1  Kino der Toten       (zombie_theater)
echo   2  Five                 (zombie_pentagon)
echo   3  Ascension            (zombie_cosmodrome)
echo   4  Call of the Dead     (zombie_coast)
echo   5  Shangri-La           (zombie_temple)
echo   6  Moon                 (zombie_moon)
echo   7  Nacht der Untoten    (zombie_cod5_prototype)
echo   8  Verruckt             (zombie_cod5_asylum)
echo   9  Shi No Numa          (zombie_cod5_sumpf)
echo   0  Der Riese            (zombie_cod5_factory)
choice /c 1234567890 /n /m "Map [1-0]: "
set "PICK=%ERRORLEVEL%"
if "%PICK%"=="1" set "MAP=zombie_theater"
if "%PICK%"=="2" set "MAP=zombie_pentagon"
if "%PICK%"=="3" set "MAP=zombie_cosmodrome"
if "%PICK%"=="4" set "MAP=zombie_coast"
if "%PICK%"=="5" set "MAP=zombie_temple"
if "%PICK%"=="6" set "MAP=zombie_moon"
if "%PICK%"=="7" set "MAP=zombie_cod5_prototype"
if "%PICK%"=="8" set "MAP=zombie_cod5_asylum"
if "%PICK%"=="9" set "MAP=zombie_cod5_sumpf"
if "%PICK%"=="10" set "MAP=zombie_cod5_factory"
:play
cd /d "%GAME%"
start "" "%GAME%\BO1Zombies.exe" +set bo1_zombies 1 +set fs_b "%GAME%" +set fs_h "%GAME%" +set logfile 2 +set net_ip 127.0.0.1 +set r_fullscreen 0 +set r_borderless 1 +set vid_xpos 0 +set vid_ypos 0 +set fs_game mods/leon +set bo1_rtmodels leon +set leon_thirdperson 1 +set player_tankControls 1 +set cg_thirdPersonShoulder 18 +set cg_thirdPersonShoulderUp 6 +set cg_thirdPersonRange 42 +devmap %MAP%
