# Symphony
An MLIR dialect for audio effects and synthesizers, compiling signal graphs to real-time-safe native code and WebAssembly.

# aud-mlir

An out-of-tree [MLIR](https://mlir.llvm.org) dialect for audio effects and
synthesizers.

`aud` lets you describe an audio processing graph (oscillators, filters,
delays, envelopes, mixers) at a high level and compiles it down to a
block-based `process(in, out, n)` function that can run as native code,
in WebAssembly (AudioWorklet), or inside a plugin.

> **Status:** early stage / experimental. The dialect is under design;
> syntax and operation names may change.

## Why a dialect?

General-purpose compilers see audio code as plain loops. Working at the
domain level makes optimizations possible that they cannot express, such as:

- fusing a whole effect chain into a single loop with no intermediate buffers
- algebraic simplification (e.g. merging consecutive gains)
- constant-folding filter coefficients at compile time
- vectorizing voices and channels

## Design goals

- Explicit, reentrant state for filters, delays and oscillators
- Block processing with separate audio-rate and control-rate signals
- Real-time safe output: no allocation or locks on the audio path
- Multiple targets from one description: native, ARM, WebAssembly, plugins

## Example (illustrative syntax)

    func.func @fx(%in: !aud.sig<f32>) -> !aud.sig<f32> {
      %lp   = aud.biquad %in {type = "lowpass", freq = 2000.0 : f32, q = 0.7 : f32}
              : !aud.sig<f32>
      %dist = aud.tanh %lp {drive = 4.0 : f32} : !aud.sig<f32>
      %out  = aud.gain %dist {db = -6.0 : f32} : !aud.sig<f32>
      return %out : !aud.sig<f32>
    }

## Related work

- [DSP-MLIR](https://github.com/tridhapuku/DSP_MLIR): a general-purpose DSP
  dialect and DSL for MLIR, used here as a design reference
- [Faust](https://faust.grame.fr): functional language for audio DSP

## Building

Requires LLVM/MLIR built from source (version pinned in this README), CMake
and Ninja. See the build instructions below.
