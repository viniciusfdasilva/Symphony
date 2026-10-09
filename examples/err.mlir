func.func @erro(%x: f32) -> !aud.sig<f32> {
  %a = aud.osc {freq = 440.0 : f32} : !aud.sig<f32>
  %b = aud.add %a, %x : !aud.sig<f32>
  return %b : !aud.sig<f32>
}