func.func @fx() -> !aud.sig<f32, i32> attributes {llvm.emit_c_interface} {
  %a = aud.osc {
                    shape    = #aud.wave<square>, 
                    freq    = 440.0 : f32, 
                    samples = 96000 : i32
               } : !aud.sig<f32, i32>

  return %a : !aud.sig<f32, i32>
}