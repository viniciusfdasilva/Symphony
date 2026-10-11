func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a = aud.osc {freq = 220.0 : f32, shape = "saw"} : !aud.sig<f32>
  %d = aud.sub %a, %a : !aud.sig<f32>
  return %d : !aud.sig<f32>
}
