func.func @fx() -> !aud.sig<f32> {
  %a = aud.osc {freq = 440.0 : f32} : !aud.sig<f32>
  %b = aud.osc {freq = 660.0 : f32} : !aud.sig<f32>
  %s = aud.add %a, %b : !aud.sig<f32>
  %o = aud.gain %s {db = -6.0 : f32} : !aud.sig<f32>
  return %o : !aud.sig<f32>
}