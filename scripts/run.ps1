# Builds one R^ program against the built native layers and runs it.
#   scripts/run.ps1 test/triangle.rh [-Build]    (-Build builds the layers first)
# The layers are built with the generator and clang of the current shell; run
# it from a shell set up as rhat's scripts/devshell.ps1 does.
param(
    [Parameter(Mandatory)][string]$Source,
    [switch]$Build,
    [string]$Rhat = (Join-Path $PSScriptRoot '..\..\rhat\build\Release\rhat.exe')
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$out = Join-Path $root 'build'
if ($Build -or -not (Test-Path (Join-Path $out 'rhat_gfx_shim.lib'))) {
    cmake -S $root -B $out -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang
    cmake --build $out --target rhat_gfx_shim
    if ($LASTEXITCODE -ne 0) { throw 'native build failed' }
}
$libs = 'rhat_gfx_shim.lib', 'rhat_gfx.lib', 'rhat_audio.lib', 'rhat_shaderc.lib', 'SDL\SDL3-static.lib' |
    ForEach-Object { Join-Path $out $_ }
$system = 'user32', 'gdi32', 'winmm', 'imm32', 'ole32', 'oleaut32', 'version', 'setupapi',
    'advapi32', 'shell32', 'cfgmgr32', 'dxgi', 'd3d12'
$link = foreach ($l in $libs + $system) { '--link-lib'; $l }
$exe = Join-Path $out ([IO.Path]::GetFileNameWithoutExtension($Source) + '.exe')
& $Rhat build $Source -o $exe --module-path $root @link
if ($LASTEXITCODE -ne 0) { throw 'rhat build failed' }
Push-Location $out
try { & $exe } finally { Pop-Location }
