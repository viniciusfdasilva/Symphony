module {
  func.func @fx() -> memref<128xf32> attributes {llvm.emit_c_interface} {
    %alloc = memref.alloc() : memref<128xf32>
    %cst = arith.constant 0.0575958639 : f32
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %c128 = arith.constant 128 : index
    scf.for %arg0 = %c0 to %c128 step %c1 {
      %0 = arith.index_cast %arg0 : index to i64
      %1 = arith.sitofp %0 : i64 to f32
      %2 = arith.mulf %1, %cst : f32
      %3 = math.sin %2 : f32
      memref.store %3, %alloc[%arg0] : memref<128xf32>
    }
    return %alloc : memref<128xf32>
  }
}

