func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a  = aud.osc {freq = 440.0 : f32, shape = "sine"} : !aud.sig<f32>
  %b  = aud.osc {freq = 880.0 : f32, shape = "sine"} : !aud.sig<f32>
  %o =  aud.max %a, %b : !aud.sig<f32>
  return %o : !aud.sig<f32>
}