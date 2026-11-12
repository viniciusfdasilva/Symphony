#!/usr/bin/env python3
"""
Usage: scripts/play.py <player> <input.mlir> [seconds] [output.wav]

Examples:
  scripts/play.py aplay examples/tone.mlir
  scripts/play.py mpv examples/tone.mlir 3 /tmp/test.wav
  scripts/play.py "ffplay -nodisp -autoexit" examples/tone.mlir
  scripts/play.py none examples/tone.mlir      # only generate the WAV
"""
import argparse
import ctypes
import ctypes.util
import os
import shlex
import struct
import subprocess
import sys
import wave
from pathlib import Path

SAMPLE_RATE = 48000

# Project root; override with the SYMPHONY_ROOT environment variable if needed.
ROOT = Path(os.environ.get("SYMPHONY_ROOT",
                           "/home/silva/viniciusfdasilva/Symphony"))
AUD_OPT = ROOT / "build/bin/aud-opt"
LLVM_BIN = ROOT / ".local/llvm/bin"
WORK = ROOT / "build/play"
CXX = os.environ.get("CXX", "clang++")


class MemRef1D(ctypes.Structure):
    _fields_ = [
        ("allocated", ctypes.c_void_p),
        ("aligned", ctypes.c_void_p),
        ("offset", ctypes.c_int64),
        ("sizes", ctypes.c_int64 * 1),
        ("strides", ctypes.c_int64 * 1),
    ]


def run(cmd):
    cmd = [str(c) for c in cmd]
    subprocess.run(cmd, check=True)


def die(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


def compile_to_shared_lib(mlir: Path, name: str, block: int) -> Path:
    llvm_mlir = WORK / f"{name}.llvm.mlir"
    ll = WORK / f"{name}.ll"
    obj = WORK / f"{name}.o"
    so = WORK / f"{name}.so"

    print("==> Lowering (aud -> LLVM dialect)")
    run([
        AUD_OPT, mlir,
        f"--convert-aud-to-std=block-size={block} sample-rate={SAMPLE_RATE}",
        "--convert-scf-to-cf", "--convert-cf-to-llvm",
        "--convert-arith-to-llvm", "--convert-math-to-llvm",
        "--finalize-memref-to-llvm", "--convert-func-to-llvm",
        "--reconcile-unrealized-casts",
        "-o", llvm_mlir,
    ])

    print("==> LLVM IR and object file")
    run([LLVM_BIN / "mlir-translate", "--mlir-to-llvmir", llvm_mlir, "-o", ll])
    run([LLVM_BIN / "llc", "-O2", "-filetype=obj",
         "-relocation-model=pic", ll, "-o", obj])

    print("==> Building shared library")
    run([CXX, "-shared", "-O2", obj, "-o", so, "-lm"])
    return so


def render(so: Path) -> list:
    """Call @fx through ctypes and return the samples (floats)."""
    lib = ctypes.CDLL(str(so))
    fx = lib._mlir_ciface_fx
    fx.argtypes = [ctypes.POINTER(MemRef1D)]
    fx.restype = None

    buf = MemRef1D()
    fx(ctypes.byref(buf))

    n = buf.sizes[0]
    # aligned + offset (offset is in elements, not bytes)
    addr = buf.aligned + buf.offset * ctypes.sizeof(ctypes.c_float)
    samples = list((ctypes.c_float * n).from_address(addr))

    # free the buffer allocated by the MLIR code
    libc = ctypes.CDLL(ctypes.util.find_library("c"))
    libc.free.argtypes = [ctypes.c_void_p]
    libc.free(buf.allocated)
    return samples


def write_wav(path: Path, samples: list):
    pcm = struct.pack(
        f"<{len(samples)}h",
        *(int(max(-1.0, min(1.0, x)) * 32767.0) for x in samples),
    )
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SAMPLE_RATE)
        w.writeframes(pcm)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("player", help='"aplay", "mpv", "ffplay -nodisp -autoexit" or "none"')
    ap.add_argument("input", type=Path, help=".mlir file")
    ap.add_argument("duration", nargs="?", type=float, default=1.0,
                    help="duration in seconds (default: 1)")
    ap.add_argument("output", nargs="?", type=Path, help="output WAV file")
    args = ap.parse_args()

    if not args.input.is_file():
        die(f"File not found: {args.input}")
    if not os.access(AUD_OPT, os.X_OK):
        die(f"Missing {AUD_OPT}. Run scripts/build.sh")
    for t in ("mlir-translate", "llc"):
        if not os.access(LLVM_BIN / t, os.X_OK):
            die(f"Missing {LLVM_BIN / t}")

    if "llvm.emit_c_interface" not in args.input.read_text():
        print(f"Warning: {args.input} has no 'llvm.emit_c_interface' on @fx; "
              "linking will likely fail.", file=sys.stderr)

    WORK.mkdir(parents=True, exist_ok=True)
    name = args.input.stem
    wav = args.output or WORK / f"{name}.wav"
    block = int(SAMPLE_RATE * args.duration)
    print(f"==> {name}: {args.duration}s, block of {block} samples at {SAMPLE_RATE} Hz")

    so = compile_to_shared_lib(args.input, name, block)

    print(f"==> Generating {wav}")
    samples = render(so)
    write_wav(wav, samples)
    print(f"Recorded {wav}: {len(samples)} samples")

    if args.player == "none":
        print(f"==> Done (no playback): {wav}")
        return

    print(f"==> Playing with: {args.player}")
    run(shlex.split(args.player) + [wav])


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        die(f"Command failed ({e.returncode}): {' '.join(map(str, e.cmd))}")
