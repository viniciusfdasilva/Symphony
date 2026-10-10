func.func private @printMemrefF32(memref<*xf32>)

func.func @fx() -> !aud.sig<f32> {
  %a = aud.osc {freq = 3000.0 : f32} : !aud.sig<f32>
  %o = aud.gain %a {db = -6.0 : f32} : !aud.sig<f32>
  return %o : !aud.sig<f32>
}

func.func @main() {
  %s = func.call @fx() : () -> !aud.sig<f32>
  %m = builtin.unrealized_conversion_cast %s : !aud.sig<f32> to memref<16xf32>
  %u = memref.cast %m : memref<16xf32> to memref<*xf32>
  func.call @printMemrefF32(%u) : (memref<*xf32>) -> ()
  return
}