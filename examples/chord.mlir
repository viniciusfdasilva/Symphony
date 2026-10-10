func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a4       = aud.osc {freq = 440.0  : f32, shape = "sine"} : !aud.sig<f32>
  %c5_sharp = aud.osc {freq = 554.37 : f32, shape = "sine"} : !aud.sig<f32>
  %e5       = aud.osc {freq = 659.25 : f32, shape = "sine"} : !aud.sig<f32>
  %cluster  = aud.add %a4, %c5_sharp : !aud.sig<f32>
  %chord    = aud.add %c5_sharp, %e5 : !aud.sig<f32>
  return %chord : !aud.sig<f32>
}
