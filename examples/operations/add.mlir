func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a4       = aud.osc {freq = 440.0  : f32, shape = "sine"} : !aud.sig<f32>
  %b        = aud.osc {freq = 1.0 : f32, shape = "sine"} : !aud.sig<f32>
  %result   = aud.add %a4, %b : !aud.sig<f32>
  return %result : !aud.sig<f32>
}