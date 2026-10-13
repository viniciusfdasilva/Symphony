func.func @fx() -> !aud.sig<f32> attributes {llvm.emit_c_interface} {
  %a  = aud.osc {freq = 480.0  : f32, shape = "sine"} : !aud.sig<f32>
  %a1 = aud.transpose %a, 12 : !aud.sig<f32>

  %b  = aud.osc {freq = 620.0 : f32, shape = "sine"} :  !aud.sig<f32>
  %b1 = aud.transpose %b, 12 : !aud.sig<f32>

  %r = aud.add %a1, %b1 : !aud.sig<f32>
  %o = aud.gain %r {db = 6.0 : f32} : !aud.sig<f32>
  return %o : !aud.sig<f32>
}