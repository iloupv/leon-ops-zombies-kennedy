# Safety adapter only. All game launch/window/timeout enforcement stays in headless.ps1.
param([string]$Commands, [int]$AutoQuitMs, [int]$TimeoutSec, [string]$SaveLog)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$slots = 4
& (Join-Path $repo 'tools/headless.ps1') -Zombies -Client -Commands $Commands -AutoQuitMs $AutoQuitMs -TimeoutSec $TimeoutSec -SaveLog $SaveLog -MaxSlots $slots -TailLines 3
exit $LASTEXITCODE
