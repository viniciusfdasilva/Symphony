<img width="1983" height="793" alt="28f87e8f-071f-472a-adb7-ce29ee4f1561" src="https://github.com/user-attachments/assets/f117df88-18f5-425d-98a4-fb1963289dd2" />


# Symphony
An MLIR dialect for audio effects and synthesizers.

# aud-mlir

An out-of-tree [MLIR](https://mlir.llvm.org) dialect for audio effects and
synthesizers.

`aud` lets you describe an audio processing graph (oscillators, filters,
delays, envelopes, mixers).

> **Status:** early stage / experimental. The dialect is under design;
> syntax and operation names may change.

## Operations:
Actually `Symphony` provides two operations: 
* `aud.osc` :  Define a signal by its frequency (e.g., 440 Hz, note A).
* `aud.gain`:  Control the signal amplitude. Its formula is: \[y(t) = G \cdot x(t)\].
* `aud.add` :  Sums two audio signals sample by sample, mixing them into one.
## Example (illustrative syntax)

```mlir
// Defining a signal as a frequency and modifying its gain
func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a = aud.osc {freq = 440.0 : f32} : !aud.sig<f32>  
  %o = aud.gain %a {db = -6.0 : f32} : !aud.sig<f32>
  return %o : !aud.sig<f32>
}
```

```mlir
// Creating an A major chord by first summing the frequencies of the cluster `A + C#` (a major third), then adding `E`, the fifth of the A major scale.
func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a4       = aud.osc {freq = 220.0  : f32, shape = "sine"} : !aud.sig<f32>
  %c5_sharp = aud.osc {freq = 554.37 : f32, shape = "sine"} : !aud.sig<f32>
  %e5       = aud.osc {freq = 659.25 : f32, shape = "sine"} : !aud.sig<f32>
  %cluster  = aud.add %a4, %c5_sharp : !aud.sig<f32>
  %a_chord    = aud.add %c5_sharp, %e5 : !aud.sig<f32>
  return %a_chord : !aud.sig<f32>
}
```

## Get Started:

Requires Linux (or WSL2), `cmake` (≥ 3.20), `ninja`, `clang`, `lld`, `ccache`, `git` and `python3`.

```bash
git clone --recurse-submodules <repo-url> Symphony
cd Symphony

./scripts/build-llvm.sh
./scripts/build.sh
```

## Run:
> WANT TO HEAR WHAT YOU PROGRAMMED?

##### Symphony provides a script to play what you've programmed.

```bash
./build/bin/aud-opt examples/chord.mlir
./scripts/play.sh aplay examples/chord.mlir 2
```

`play.sh` usage: `<player> <input.mlir> [seconds] [output.wav]`. Use `none` as the player to only generate the WAV.


## Related work

- [DSP-MLIR](https://github.com/tridhapuku/DSP_MLIR): a general-purpose DSP
  dialect and DSL for MLIR, used here as a design reference
- [Faust](https://faust.grame.fr): functional language for audio DSP
