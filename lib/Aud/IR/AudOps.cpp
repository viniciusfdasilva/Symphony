#include "Aud/IR/AudOps.h"
#include "Aud/IR/AudDialect.h"

#define GET_OP_CLASSES
#include "Aud/IR/AudOps.cpp.inc"

using namespace mlir;
using namespace mlir::aud;

LogicalResult DivOp::verify()
{
    if(this->getFactor().convertToDouble() == 0.0) return emitOpError("Invalid operation: Division by zero");
    return success();
}