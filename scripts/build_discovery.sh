#!/usr/bin/env bash
# Build from <sketch>/scripts, independent of the caller's working directory.
# Usage: ./scripts/build_discovery.sh
#        [--build-root /path/to/builds] [--libraries /path/to/libraries]
set -euo pipefail
export LC_ALL=C
usage() {
    printf '%s\n' 'Usage: build_discovery.sh [--build-root DIR] [--libraries DIR]'
}
die() { printf 'Error: %s\n' "$*" >&2; exit 1; }
BUILD_ROOT="${BUILD_ROOT:-${HOME:?HOME must be set}/arduinobuild}"
while (($#)); do
    case "$1" in
        --build-root|--libraries)
            (($# >= 2)) || die "Missing value for $1"
            case "$1" in
                --build-root) BUILD_ROOT=$2 ;;
                --libraries) LIB_PATH=$2 ;;
            esac
            shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; die "Unknown argument: $1" ;;
    esac
done
command -v arduino-cli >/dev/null || die 'arduino-cli is required.'
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
SKETCH_PATH=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
SKETCH_NAME=${SKETCH_PATH##*/}
LIB_PATH="${LIB_PATH:-${SKETCH_PATH%/*}/libraries}"
[[ -f "$SKETCH_PATH/$SKETCH_NAME.ino" ]] || die "No $SKETCH_NAME.ino found in $SKETCH_PATH"
[[ -d "$LIB_PATH" ]] || die "Library directory not found: $LIB_PATH (mount the drive or use --libraries)."
[[ -n "$BUILD_ROOT" ]] || die 'Build root cannot be empty.'
mkdir -p -- "$BUILD_ROOT"
BUILD_ROOT=$(cd -- "$BUILD_ROOT" && pwd -P)
BUILD_DIR="$BUILD_ROOT/$SKETCH_NAME"
mkdir -p -- "$BUILD_DIR"
BOARD='adafruit:samd:adafruit_qtpy_m0'
printf 'Sketch: %s\nBuild:  %s\n' "$SKETCH_PATH" "$BUILD_DIR"
printf '%s\n' '--- Starting Linux Compile with library discovery ---'
if arduino-cli compile --fqbn "$BOARD" \
    --jobs 0 \
    --libraries "$LIB_PATH" \
    --build-path "$BUILD_DIR" \
    --build-property "compiler.c.elf.extra_flags=-Wl,-Map,$BUILD_DIR/output.map" \
    -v "$SKETCH_PATH"; then
    printf '%s\n' '--- Compile Success! ---'
else
    status=$?
    printf 'Compile failed (exit code %s). Check the library path, FQBN and output above.\n' "$status" >&2
    exit "$status"
fi
# Compile only. For a separate upload, select the actual Linux device using
# `arduino-cli board list` (e.g. /dev/serial/by-id/..., or /dev/ttyACM0).
