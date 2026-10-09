func.func @main(%x: f32) -> f32 {
  %0 = aud.gain %x {db = -6.0 : f32} : f32
  return %0 : f32
}
