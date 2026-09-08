#!/usr/bin/env bash
# Builds the LZ77 C core (+ wasm shim) into dist/lz77.js + dist/lz77.wasm
# using Emscripten. Requires the emsdk to be installed and activated first
# (emsdk_env.sh sourced so `emcc` is on PATH).
#
# Usage: ./build-wasm.sh

set -euo pipefail

mkdir -p dist

emcc \
  src/bitstream.c \
  src/hashchain.c \
  src/lz77_compress.c \
  src/lz77_decompress.c \
  src/wasm_bindings.c \
  -Iinclude \
  -O3 \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="createLZ77Module" \
  -s EXPORTED_FUNCTIONS='["_wasm_compress","_wasm_decompress","_wasm_free","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["HEAPU8","getValue","setValue"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s ENVIRONMENT=node,web \
  -s SINGLE_FILE=0 \
  -o dist/lz77.js

echo "Built dist/lz77.js + dist/lz77.wasm"