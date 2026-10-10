func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a        = aud.osc {freq = 480.0  : f32, shape = "sine"} : !aud.sig<f32>
  %b        = aud.osc {freq = 620.0 : f32, shape = "sine"} : !aud.sig<f32>
  %cluster  = aud.add %a, %b : !aud.sig<f32>
  return %cluster : !aud.sig<f32>
}
