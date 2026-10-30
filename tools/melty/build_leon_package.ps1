# build_leon_package.ps1 - the Leon mod's release package: a folder of its own that plays from the player's Black Ops
# (BO1Z_GAME_DIR, src/win32/win_common.cpp) and builds Leon from their Resident Evil 4 on first start (leon_setup).
# Melty installs it with melty.json (the repo root); without Melty, LeonLauncher.exe in it does the same.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\melty\build_leon_package.ps1 [-Version 0.1.0]
#
# Needs build\Release\BO1Zombies.exe built, lfsunpack.exe built (-LfsUnpack, tools\leon-converter\README.md), the RE4
# decomp's motion tools and host DLL (-Re4Decomp) and a venv with PyInstaller and Pillow (build\melty\venv). The
# converter is tools\leon-converter, the launcher tools\leon-launcher. Writes build\melty\pkg and
# build\melty\LeonZombies-<version>.zip. Nothing of either game goes in: no Black Ops file (the zones, iwds, videos,
# binkw32's install copy, DirectX cabs stay in the player's install) and nothing converted from RE4 (the setup writes
# mods\leon\rtmodels and mods\leon\sound on the player's PC).
param(
    [string]$Version = '0.1.25',
    [string]$LfsUnpack = 'C:\dev\refs\leon-tools\lfsunpack.exe',
    [string]$Re4Decomp = 'C:\dev\refs\re4'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$ToolsDir = Join-Path $repo 'tools\leon-converter'
$M = Join-Path $repo 'build\melty'
$pkg = Join-Path $M 'pkg'
$py = Join-Path $M 'venv\Scripts\python.exe'
if (-not (Test-Path $py)) { throw "no ${py}: python -m venv build\melty\venv, then pip install pyinstaller pillow" }
$exe = Join-Path $repo 'build\Release\BO1Zombies.exe'
if (-not (Test-Path $exe)) { throw "no ${exe}: build first" }
if (-not (Test-Path $LfsUnpack)) { throw "no ${LfsUnpack}: build it (tools\leon-converter\README.md)" }

# the setup program (leon_setup.py, frozen with the converter, lfsunpack and the decomp's motion host)
$stage = Join-Path $M 'setup_build'
Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item (Join-Path $Re4Decomp 'tools\motion\host\build\libmotion_host.so') (Join-Path $stage 'libmotion_host.dll')
$hidden = 'leon_rlm', 'leon_sounds', 're4_hud', 're4_motion', 'motion.evalhost', 'motion.fcv', 'udas_reader', 'uhd_bin_gltf',
    'uhd_pack', 'leon_gltf', 'leon_full' | ForEach-Object { '--hidden-import', $_ }
$ErrorActionPreference = 'Continue' # (PowerShell 5.1 turns its stderr log lines into errors)
& (Join-Path $M 'venv\Scripts\pyinstaller.exe') --noconfirm --onedir --windowed --name leon_setup --icon (Join-Path $repo 'src\win32\leon.ico') `
    --distpath (Join-Path $stage 'dist') --workpath (Join-Path $stage 'work') --specpath $stage `
    --paths $ToolsDir --paths (Join-Path $Re4Decomp 'tools') @hidden `
    --add-binary "${LfsUnpack};." --add-binary "$(Join-Path $stage 'libmotion_host.dll');motion_host" `
    --exclude-module tkinter --exclude-module unittest --exclude-module pydoc `
    (Join-Path $ToolsDir 'leon_setup.py') 2>&1 | Out-File (Join-Path $stage 'pyinstaller.log')
$ErrorActionPreference = 'Stop'
if ($LASTEXITCODE) { throw "pyinstaller failed ($LASTEXITCODE): $(Join-Path $stage 'pyinstaller.log')" }

# the launcher for playing without Melty (one file next to the game's exe)
$ErrorActionPreference = 'Continue'
& (Join-Path $M 'venv\Scripts\pyinstaller.exe') --noconfirm --onefile --windowed --name LeonLauncher --icon (Join-Path $repo 'src\win32\leon.ico') `
    --distpath (Join-Path $stage 'dist') --workpath (Join-Path $stage 'work_launcher') --specpath $stage `
    --exclude-module unittest --exclude-module pydoc `
    (Join-Path $repo 'tools\leon-launcher\leon_launcher.py') 2>&1 | Out-File (Join-Path $stage 'pyinstaller_launcher.log')
$ErrorActionPreference = 'Stop'
if ($LASTEXITCODE) { throw "pyinstaller failed for the launcher ($LASTEXITCODE): $(Join-Path $stage 'pyinstaller_launcher.log')" }

# the package
Remove-Item -Recurse -Force $pkg -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force (Join-Path $pkg 'main') | Out-Null
Copy-Item $exe $pkg
# the release's exe and its PDB kept (not shipped) under build\melty\symbols\<version>: players' crash offsets
# (Melty's reports: BO1Zombies.exe+0x...) resolve against them
$symbols = Join-Path $M "symbols\$Version"
New-Item -ItemType Directory -Force $symbols | Out-Null
Copy-Item $exe, (Join-Path $repo 'build\Release\BO1Zombies.pdb') $symbols
Copy-Item (Join-Path $repo 'src\steam\redistributable_bin\steam_api.dll'), (Join-Path $repo 'src\binklib\binkw32.dll') $pkg
Copy-Item (Join-Path $repo 'build\Release\msvcp140.dll'), (Join-Path $repo 'build\Release\vcruntime140.dll') $pkg
Copy-Item (Join-Path $repo 'data\main\*') (Join-Path $pkg 'main')
Push-Location $repo
try {
    # the mod's own files only (git-tracked: never mods\leon\rtmodels or its sounds)
    git ls-files mods/leon | ForEach-Object {
        $d = Join-Path $pkg (Split-Path $_)
        New-Item -ItemType Directory -Force $d | Out-Null
        Copy-Item $_ $d
    }
} finally { Pop-Location }
Copy-Item -Recurse (Join-Path $stage 'dist\leon_setup') (Join-Path $pkg 'leon_setup')
Copy-Item (Join-Path $stage 'dist\LeonLauncher.exe') $pkg
Copy-Item (Join-Path $PSScriptRoot 'LEON_README.txt') (Join-Path $pkg 'README.txt') # (the name Melty reads as the readme)
Copy-Item (Join-Path $repo 'LICENSE') (Join-Path $pkg 'LICENSE.txt')

$zip = Join-Path $M "LeonZombies-$Version.zip"
Remove-Item $zip -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $pkg '*') -DestinationPath $zip
$files = Get-ChildItem -Recurse $pkg -File
"{0}: {1} files, {2:N1} MB unpacked, {3:N1} MB zipped" -f $zip, $files.Count, (($files | Measure-Object Length -Sum).Sum / 1MB), ((Get-Item $zip).Length / 1MB)
