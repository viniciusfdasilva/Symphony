func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %censor_freq = aud.osc {freq = 1000.0 : f32} : !aud.sig<f32>
  return %censor_freq : !aud.sig<f32>
}