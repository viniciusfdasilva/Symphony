#ifndef AUD_DIALECT
#define AUD_DIALECT

include "mlir/IR/OpBase.td"

def Aud_Dialect : Dialect {
  let name = "aud";
  let summary = "Audio effects and synthesizers dialect";
  let cppNamespace = "::mlir::aud";
  let useDefaultTypePrinterParser = 1;
}

#endif // AUD_DIALECT
