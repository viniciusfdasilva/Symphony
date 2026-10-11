func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a4  = aud.osc {freq = 440.0 : f32, shape = "sine"} : !aud.sig<f32>
  %inv = aud.transpose %a4, -12 : !aud.sig<f32>
  return %inv : !aud.sig<f32>
}