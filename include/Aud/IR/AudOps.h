#pragma once

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "Aud/IR/AudTypes.h"
#include "Aud/IR/AudDialect.h"

// 1) enum primeiro (WaveShape)
#include "Aud/IR/AudEnum.h.inc"

// 2) attr que usa o enum (WaveShapeAttr)
#define GET_ATTRDEF_CLASSES
#include "Aud/IR/AudAttrDefs.h.inc"

// 3) ops, que usam o attr
#define GET_OP_CLASSES
#include "Aud/IR/AudOps.h.inc"