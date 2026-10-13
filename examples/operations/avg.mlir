func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a  = aud.osc {freq = 430.0 : f32, shape = "sine"} : !aud.sig<f32>
  %b  = aud.osc {freq = 450.0 : f32, shape = "sine"} : !aud.sig<f32>
  %o =  aud.avg %a, %b : !aud.sig<f32>
  return %o : !aud.sig<f32>
}