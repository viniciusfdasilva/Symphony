module {
  func.func @fx() -> memref<128xf32> attributes {llvm.emit_c_interface} {
    %alloc = memref.alloc() : memref<128xf32>
    %cst = arith.constant 0.028797932 : f32
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
    %alloc_0 = memref.alloc() : memref<128xf32>
    %cst_1 = arith.constant 0.0575958639 : f32
    %c0_2 = arith.constant 0 : index
    %c1_3 = arith.constant 1 : index
    %c128_4 = arith.constant 128 : index
    scf.for %arg0 = %c0_2 to %c128_4 step %c1_3 {
      %0 = arith.index_cast %arg0 : index to i64
      %1 = arith.sitofp %0 : i64 to f32
      %2 = arith.mulf %1, %cst_1 : f32
      %3 = math.sin %2 : f32
      memref.store %3, %alloc_0[%arg0] : memref<128xf32>
    }
    %alloc_5 = memref.alloc() : memref<128xf32>
    %c0_6 = arith.constant 0 : index
    %c1_7 = arith.constant 1 : index
    %c128_8 = arith.constant 128 : index
    scf.for %arg0 = %c0_6 to %c128_8 step %c1_7 {
      %0 = memref.load %alloc_0[%arg0] : memref<128xf32>
      %1 = memref.load %alloc[%arg0] : memref<128xf32>
      %2 = arith.subf %0, %1 : f32
      memref.store %2, %alloc_5[%arg0] : memref<128xf32>
    }
    return %alloc_5 : memref<128xf32>
  }
}

