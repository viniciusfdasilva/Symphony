#ifndef AUD_DIALECT
#define AUD_DIALECT

include "mlir/IR/OpBase.td"

def Aud_Dialect : Dialect {
  let name = "aud";
  let cppNamespace = "::mlir::aud";
  let useDefaultTypePrinterParser = 1;
  let useDefaultAttributePrinterParser = 1;
}

#endif // AUD_DIALECT
