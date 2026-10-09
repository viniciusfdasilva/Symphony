#ifndef AUD_OPS
#define AUD_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "Aud/AudDialect.td"

class Aud_Op<string mnemonic, list<Trait> traits = []> :
    Op<Aud_Dialect, mnemonic, traits>;

def Aud_GainOp : Aud_Op<"gain", [Pure, SameOperandsAndResultType]> {
  let summary = "Multiplies the signal by a gain given in dB";
  let arguments = (ins F32:$input, F32Attr:$db);
  let results = (outs F32:$output);
  let assemblyFormat = "$input attr-dict `:` type($output)";
}

#endif // AUD_OPS
