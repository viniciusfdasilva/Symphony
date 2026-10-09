#include "Aud/AudDialect.h"
#include "Aud/AudOps.h"

using namespace mlir;
using namespace mlir::aud;

#include "Aud/AudOpsDialect.cpp.inc"

void AudDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Aud/AudOps.cpp.inc"
      >();
}
