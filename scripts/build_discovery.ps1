<#
.SYNOPSIS
Build GalaxyBall with library discovery.
.EXAMPLE
.\scripts\build_discovery.ps1
#>
[CmdletBinding()]
param(
    [string]$BuildRoot = 'C:\arduinobuild'
)
$ErrorActionPreference = 'Stop'

# Configuration
$SKETCH = Split-Path -Path $PSScriptRoot -Parent
$BOARD = "adafruit:samd:adafruit_qtpy_m0"
$BUILD_ROOT = [IO.Path]::GetFullPath($BuildRoot)
$LIB_PATH = Join-Path (Split-Path -Path $SKETCH -Parent) 'libraries'

# Resolve the sketch from the parent of this scripts folder, independent of the working directory.
$sketchItem = Get-Item -LiteralPath (Resolve-Path -LiteralPath $SKETCH -ErrorAction Stop).ProviderPath -ErrorAction Stop
if ($sketchItem.PSIsContainer) {
    $SKETCH_PATH = $sketchItem.FullName
} elseif ($sketchItem.Extension -eq '.ino') {
    $SKETCH_PATH = $sketchItem.Directory.FullName
} else {
    throw "SKETCH must be a sketch directory or an .ino file."
}
$SKETCH_NAME = Split-Path -Path $SKETCH_PATH -Leaf
if (-not (Test-Path -LiteralPath (Join-Path $SKETCH_PATH "$SKETCH_NAME.ino") -PathType Leaf)) {
    throw "No $SKETCH_NAME.ino found in $SKETCH_PATH. Check the sketch directory configured by $SKETCH."
}

# Use the sketch folder name for all build and upload scripts.
$BUILD_DIR = Join-Path $BUILD_ROOT $SKETCH_NAME
New-Item -ItemType Directory -Path $BUILD_DIR -Force -ErrorAction Stop | Out-Null
Write-Host "Sketch: $SKETCH_PATH"
Write-Host "Build:  $BUILD_DIR"

Write-Host "--- Starting Windows Compile with library discovery ---" -ForegroundColor Cyan
# --jobs 0 uses all CPU cores. Retain build files for incremental compilation.
arduino-cli compile --fqbn $BOARD `
    --jobs 0 `
    --libraries $LIB_PATH `
    --build-path $BUILD_DIR `
    --build-property "compiler.c.elf.extra_flags=-Wl,-Map,$BUILD_DIR/output.map" `
    -v `
    $SKETCH_PATH

if ($LASTEXITCODE -ne 0) {
    throw "Compile failed (exit code $LASTEXITCODE). Check library path, FQBN, and compile output above."
}

Write-Host "--- Compile Success! ---" -ForegroundColor Green
