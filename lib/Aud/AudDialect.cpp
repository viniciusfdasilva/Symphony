#include "Aud/AudDialect.h"
#include "Aud/AudOps.h"
#include "Aud/AudTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::aud;

#include "Aud/AudOpsDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "Aud/AudTypes.cpp.inc"

void AudDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Aud/AudTypes.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "Aud/AudOps.cpp.inc"
      >();
}