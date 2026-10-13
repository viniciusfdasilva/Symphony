func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a = aud.osc {freq = 440.0 : f32} : !aud.sig<f32>
  return %a : !aud.sig<f32>
}