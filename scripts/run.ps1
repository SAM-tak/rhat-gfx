# Builds one R^ program against the built native layers and runs it.
#   scripts/run.ps1 test/triangle.rh [-Build] [-Arguments a, b]
# -Build builds the layers first; -Arguments go to the program.
# The layers are built with the generator and clang of the current shell; run
# it from a shell set up as rhat's scripts/devshell.ps1 does.
param(
    [Parameter(Mandatory)][string]$Source,
    [switch]$Build,
    [string[]]$Arguments = @(),
    [string]$Rhat = (Join-Path $PSScriptRoot '..\..\rhat\build\Release\rhat.exe')
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$out = Join-Path $root 'build'
if ($Build -or -not (Test-Path (Join-Path $out 'rhat_gfx_shim.lib'))) {
    Push-Location $root
    try {
        cmake --preset native
        if ($LASTEXITCODE -ne 0) { throw 'native configure failed' }
        cmake --build --preset native
        if ($LASTEXITCODE -ne 0) { throw 'native build failed' }
    } finally {
        Pop-Location
    }
}
$libs = 'rhat_gfx_shim.lib', 'rhat_gfx.lib', 'rhat_audio.lib', 'rhat_shaderc.lib', 'SDL\SDL3-static.lib' |
    ForEach-Object { Join-Path $out $_ }
$system = 'user32', 'gdi32', 'winmm', 'imm32', 'ole32', 'oleaut32', 'version', 'setupapi',
    'advapi32', 'shell32', 'cfgmgr32', 'dxgi', 'd3d12'
$link = foreach ($l in $libs + $system) { '--link-lib'; $l }
$exe = Join-Path $out ([IO.Path]::GetFileNameWithoutExtension($Source) + '.exe')
& $Rhat build $Source -o $exe --module-path $root @link
if ($LASTEXITCODE -ne 0) { throw 'rhat build failed' }
# From the caller's directory, so that relative paths in -Arguments hold.
& $exe @Arguments
