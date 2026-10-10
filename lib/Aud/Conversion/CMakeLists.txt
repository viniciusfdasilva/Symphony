add_mlir_conversion_library(MLIRAudToStd
  AudToStd.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Aud

  DEPENDS
  MLIRAudOpsIncGen
  MLIRAudTypesIncGen

  LINK_LIBS PUBLIC
  MLIRAud
  MLIRArithDialect
  MLIRFuncDialect
  MLIRFuncTransforms
  MLIRMathDialect
  MLIRMemRefDialect
  MLIRSCFDialect
  MLIRPass
  MLIRTransformUtils
)