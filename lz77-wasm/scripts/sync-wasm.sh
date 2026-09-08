#!/usr/bin/env bash
# Copies the emcc build output (dist/lz77.js + dist/lz77.wasm) from the
# sibling lz77c repo into this package's vendor/ folder.
#
# Assumes lz77-wasm and lz77c are sibling directories, e.g.:
#   ~/projetos-dv/lz77c
#   ~/projetos-dv/lz77-wasm
#
# Run this after every `bash build-wasm.sh` in lz77c.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_ROOT="$(dirname "$SCRIPT_DIR")"
LZ77C_DIST="$PACKAGE_ROOT/../lz77c/dist"
VENDOR_DIR="$PACKAGE_ROOT/vendor"

if [ ! -f "$LZ77C_DIST/lz77.js" ] || [ ! -f "$LZ77C_DIST/lz77.wasm" ]; then
  echo "error: could not find dist/lz77.js and dist/lz77.wasm in $LZ77C_DIST"
  echo "  did you run 'bash build-wasm.sh' in the lz77c repo first?"
  exit 1
fi

mkdir -p "$VENDOR_DIR"
cp "$LZ77C_DIST/lz77.js" "$VENDOR_DIR/lz77.js"
cp "$LZ77C_DIST/lz77.wasm" "$VENDOR_DIR/lz77.wasm"

echo "Synced $LZ77C_DIST/lz77.{js,wasm} -> $VENDOR_DIR/"