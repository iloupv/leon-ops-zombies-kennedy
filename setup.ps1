# setup.ps1 - copy your Call of Duty: Black Ops install next to the built game (one time, about 11 GB).
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1 [-GameDir "D:\Games\Call of Duty Black Ops"]
#
# The game runs from build\Release and reads the original game's files (zone\, main\*.iwd, videos, DLLs) from
# there. Your Steam install is only read, never changed. BlackOps.exe and BlackOpsMP.exe are not copied.
param(
    [string]$GameDir = 'C:\Program Files (x86)\Steam\steamapps\common\Call of Duty Black Ops'
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path (Join-Path $GameDir 'zone'))) {
    throw "No Black Ops install at '$GameDir'. Pass -GameDir with the folder that holds BlackOps.exe."
}
$dst = Join-Path $PSScriptRoot 'build\Release'
New-Item -ItemType Directory -Force $dst | Out-Null
# players\ (your own profile and settings) is left out: the game makes a fresh one
robocopy $GameDir $dst *.* /S /E /XF BlackOps.exe BlackOpsMP.exe /XD (Join-Path $GameDir 'players') /NDL /NFL /NP /R:1 /W:1
if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE)" }
# the engine's own DLLs replace the install's older ones (the exe is built against this Steam SDK and Bink)
Copy-Item (Join-Path $PSScriptRoot 'src\binklib\binkw32.dll') $dst -Force
Copy-Item (Join-Path $PSScriptRoot 'src\steam\redistributable_bin\*.dll') $dst -Force
# runtimes the exe needs that a fresh Windows lacks, put next to the exe so nothing has to be installed:
# DirectX 9 (d3dx9_43, xinput1_3) out of the game's own Redist\DirectX, Visual C++ 2015-2022 (x86) out of Visual Studio
$dx = Join-Path $dst 'Redist\DirectX'
expand.exe (Join-Path $dx 'Jun2010_d3dx9_43_x86.cab') "-F:d3dx9_43.dll" $dst | Out-Null
expand.exe (Join-Path $dx 'APR2007_xinput_x86.cab') "-F:xinput1_3.dll" $dst | Out-Null
foreach ($f in 'd3dx9_43.dll', 'xinput1_3.dll') { if (-not (Test-Path (Join-Path $dst $f))) { throw "could not extract $f from $dx" } }
$crt = Get-ChildItem 'C:\Program Files*\Microsoft Visual Studio\*\*\VC\Redist\MSVC\*\x86\Microsoft.VC14*.CRT' -Directory -ErrorAction SilentlyContinue |
    Sort-Object FullName | Select-Object -Last 1
if ($crt) { Copy-Item (Join-Path $crt.FullName 'msvcp140.dll'), (Join-Path $crt.FullName 'vcruntime140.dll') $dst -Force }
else { 'Visual C++ runtime not found next to Visual Studio: whoever plays needs the "Visual C++ Redistributable 2015-2022 (x86)"' }
"game files ready in $dst"
