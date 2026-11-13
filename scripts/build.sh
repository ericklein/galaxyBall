#!/bin/bash

# Configuration
SKETCH=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
BOARD="adafruit:samd:adafruit_qtpy_m0"
BUILD_DIR="./build"
LIB_PATH="${LIB_PATH:-${SKETCH%/*}/libraries}"

# Ensure directories exist for persistent speed gains
mkdir -p "$BUILD_DIR"

echo "--- Starting Ultra-Fast Compile ---"

# --jobs 0: Parallelizes across all CPU cores
# --build-path/--build-cache-path: Prevents recompiling the SAMD core

arduino-cli compile --fqbn "$BOARD" \
  --jobs 0 \
  --libraries "$LIB_PATH" \
  --build-path "$BUILD_DIR" \
  "$SKETCH"

# Check if compile succeeded
if [ $? -eq 0 ]; then
    echo "--- Compile Success! ---"
    # Optional: arduino-cli upload -p /dev/ttyACM0 --fqbn "$BOARD" --input-dir "$BUILD_DIR"
else
    echo "--- Compile Failed ---" >&2
    exit 1
fi