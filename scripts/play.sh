#!/usr/bin/env bash
# Usage: scripts/play.sh <player> <input.mlir> [seconds] [output.wav]
#
# Examples:
#   scripts/play.sh aplay examples/tone.mlir
#   scripts/play.sh mpv examples/tone.mlir 3 /tmp/test.wav
#   scripts/play.sh "ffplay -nodisp -autoexit" examples/tone.mlir
#   scripts/play.sh none examples/tone.mlir        # only generate the WAV
set -euo pipefail

if [ $# -lt 2 ]; then
  sed -n '2,9p' "$0" | sed 's/^# \{0,1\}//'
  exit 1
fi

PLAYER="$1"
INPUT="$2"
DURATION="${3:-1}"
SAMPLE_RATE=48000   # must match host/main.cpp

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
AUD_OPT="$ROOT/build/bin/aud-opt"
LLVM_BIN="$ROOT/.local/llvm/bin"
WORK="$ROOT/build/play"
CXX="${CXX:-clang++}"

[ -f "$INPUT" ]   || { echo "File not found: $INPUT" >&2; exit 1; }
[ -x "$AUD_OPT" ] || { echo "Missing $AUD_OPT. Run scripts/build.sh" >&2; exit 1; }
for t in mlir-translate llc; do
  [ -x "$LLVM_BIN/$t" ] || { echo "Missing $LLVM_BIN/$t" >&2; exit 1; }
done

if ! grep -q "llvm.emit_c_interface" "$INPUT"; then
  echo "Warning: $INPUT has no 'llvm.emit_c_interface' on @fx; linking will likely fail." >&2
fi

# WAV path: 4th argument, or <mlir-name>.wav inside build/play
mkdir -p "$WORK"
NAME="$(basename "${INPUT%.mlir}")"
WAV="${4:-$WORK/$NAME.wav}"

# Block size = number of samples for the requested duration
BLOCK=$(awk -v s="$SAMPLE_RATE" -v d="$DURATION" 'BEGIN{printf "%d", s*d}')
echo "==> $NAME: ${DURATION}s, block of $BLOCK samples at ${SAMPLE_RATE} Hz"

echo "==> Lowering (aud -> LLVM dialect)"
"$AUD_OPT" "$INPUT" \
  --convert-aud-to-std="block-size=$BLOCK sample-rate=$SAMPLE_RATE" \
  --convert-scf-to-cf --convert-cf-to-llvm \
  --convert-arith-to-llvm --convert-math-to-llvm \
  --finalize-memref-to-llvm --convert-func-to-llvm \
  --reconcile-unrealized-casts \
  -o "$WORK/$NAME.llvm.mlir"

echo "==> LLVM IR and object file"
"$LLVM_BIN/mlir-translate" --mlir-to-llvmir "$WORK/$NAME.llvm.mlir" -o "$WORK/$NAME.ll"
"$LLVM_BIN/llc" -O2 -filetype=obj -relocation-model=pic "$WORK/$NAME.ll" -o "$WORK/$NAME.o"

echo "==> Linking with the host program"
"$CXX" -O2 "$ROOT/host/main.cpp" "$WORK/$NAME.o" -o "$WORK/$NAME.bin"

echo "==> Generating $WAV"
"$WORK/$NAME.bin" "$WAV"

if [ "$PLAYER" = "none" ]; then
  echo "==> Done (no playback): $WAV"
  exit 0
fi

echo "==> Playing with: $PLAYER"
# intentionally unquoted so a player with arguments works, e.g. "ffplay -nodisp -autoexit"
$PLAYER "$WAV"
