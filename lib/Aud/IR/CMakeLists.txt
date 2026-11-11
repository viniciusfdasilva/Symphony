add_mlir_dialect_library(MLIRAud
  AudDialect.cpp
  AudOps.cpp
  AudEnum.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Aud

  DEPENDS
  MLIRAudOpsIncGen
  MLIRAudTypesIncGen
  AudEnumIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRBytecodeOpInterface
  MLIRInferTypeOpInterface
  MLIRSideEffectInterfaces
)