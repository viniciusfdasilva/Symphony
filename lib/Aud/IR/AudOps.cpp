#include "Aud/IR/AudOps.h"
#include "Aud/IR/AudDialect.h"

#define GET_OP_CLASSES
#include "Aud/IR/AudOps.cpp.inc"
#include "Aud/IR/AudEnum.cpp.inc"


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

LogicalResult ReverbOp::verify() {
  if (getDelayAttr().getValueAsDouble() <= 0.0)
    return emitOpError("delay must be positive");
  double d = getDecayAttr().getValueAsDouble();
  if (d < 0.0 || d >= 1.0)
    return emitOpError("decay must be in [0, 1) for the feedback to be stable");
  return success();
}