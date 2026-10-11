#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cmake -G Ninja -S "$ROOT" -B "$ROOT/build" \
  -DMLIR_DIR="$ROOT/.local/llvm/lib/cmake/mlir" \
  -DLLVM_DIR="$ROOT/.local/llvm/lib/cmake/llvm" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build "$ROOT/build"
