#include "Aud/IR/AudDialect.h"
#include "Aud/IR/AudOps.h"
#include "Aud/IR/AudTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::aud;

#include "Aud/IR/AudOpsDialect.cpp.inc"

// Types: !aud.sig
#define GET_TYPEDEF_CLASSES
#include "Aud/IR/AudTypes.cpp.inc"

// Attributes: #aud.wave<...>
// (definições do attr + parseAttribute/printAttribute do dialeto)
#define GET_ATTRDEF_CLASSES
#include "Aud/IR/AudAttrDefs.cpp.inc"

void AudDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Aud/IR/AudTypes.cpp.inc"
      >();
  addAttributes<
#define GET_ATTRDEF_LIST
#include "Aud/IR/AudAttrDefs.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "Aud/IR/AudOps.cpp.inc"
      >();
}