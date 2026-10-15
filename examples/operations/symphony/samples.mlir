func.func @fx() -> !aud.sig<f32, i32> attributes {llvm.emit_c_interface} {
  %a = aud.osc {freq = 440.0 : f32, samples = 33 : i32} : !aud.sig<f32, i32>
  return %a : !aud.sig<f32, i32>
}