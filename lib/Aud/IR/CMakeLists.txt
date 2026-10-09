
add_mlir_dialect_library(MLIRAud
  AudDialect.cpp
  AudOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Aud/IR

  DEPENDS
  MLIRAudOpsIncGen
  MLIRAudTypesIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRBytecodeOpInterface
  MLIRInferTypeOpInterface
  MLIRSideEffectInterfaces
)