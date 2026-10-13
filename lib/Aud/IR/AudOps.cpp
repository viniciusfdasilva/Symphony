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

LogicalResult ClampOp::verify()
{
    if(this->getMax().convertToDouble() < this->getMin().convertToDouble()) return emitOpError("Max value must be greater than min value.");
    return success();
}