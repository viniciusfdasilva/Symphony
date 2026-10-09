
add_mlir_dialect(AudOps aud)

add_mlir_doc(AudDialect AudDialect Aud/ -gen-dialect-doc)
add_mlir_doc(AudOps AudOps Aud/ -gen-op-doc)

set(LLVM_TARGET_DEFINITIONS AudTypes.td)

mlir_tablegen(AudTypes.h.inc -gen-typedef-decls)
mlir_tablegen(AudTypes.cpp.inc -gen-typedef-defs)

add_public_tablegen_target(MLIRAudTypesIncGen)