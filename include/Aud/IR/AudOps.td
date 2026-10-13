#ifndef AUD_OPS
#define AUD_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "Aud/IR/AudDialect.td"
include "Aud/IR/AudTypes.td"

class Aud_Op<string mnemonic, list<Trait> traits = []> :
    Op<Aud_Dialect, mnemonic, traits>;

def Aud_GainOp : Aud_Op<"gain", [Pure, SameOperandsAndResultType]> {
  let summary = "Multiplies the signal by a gain given in dB";
  let arguments = (ins Aud_SigType:$input, F32Attr:$db);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$input attr-dict `:` type($output)";
}

def Aud_AddOp : Aud_Op<"add", [Pure, Commutative, SameOperandsAndResultType]> {
  let summary = "Sample-wise sum of two signals";
  let arguments = (ins Aud_SigType:$lhs, Aud_SigType:$rhs);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($output)";
}

def Aud_SubOp : Aud_Op<"sub", [Pure, SameOperandsAndResultType]>{
  let summary   = "Sample-wise subtract two signals";
  let arguments = (ins Aud_SigType:$lhs, Aud_SigType:$rhs);
  let results   = (outs Aud_SigType:$output);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($output)"; 
}   

def Aud_MulOp : Aud_Op<"mul", [Pure, SameOperandsAndResultType]> {
  let summary = "Multiplies a signal by a constant (linear) factor";
  let arguments = (ins Aud_SigType:$freq, F32Attr:$factor);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$freq `,` $factor attr-dict `:` type($output)";
}

def Aud_DivOp : Aud_Op<"div", [Pure, SameOperandsAndResultType]> {
  let summary = "Division of a signal by a constant (linear) factor";
  let arguments = (ins Aud_SigType:$freq, F32Attr:$factor);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$freq `,` $factor attr-dict `:` type($output)";
  let hasVerifier = 1;
}

def Aud_TransposeOp : Aud_Op<"transpose", [Pure, SameOperandsAndResultType]> {
  let summary   = "Transposition of a signal by a constant factor (in semitone)";
  let arguments = (ins Aud_SigType:$freq, I32Attr:$semitone);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$freq `,` $semitone attr-dict `:` type($output)";
}

def Aud_AvgOp : Aud_Op<"avg", [Pure, SameOperandsAndResultType]> {
  let summary   = "Average between two signals";
  let arguments = (ins Aud_SigType:$lhs, Aud_SigType:$rhs);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($output)";
}

def Aud_MinOp : Aud_Op<"min", [Pure, SameOperandsAndResultType]> {
  let summary   = "Min between two signals";
  let arguments = (ins Aud_SigType:$lhs, Aud_SigType:$rhs);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($output)";
}

def Aud_MaxOp : Aud_Op<"max", [Pure, SameOperandsAndResultType]> {
  let summary   = "Max between two signals";
  let arguments = (ins Aud_SigType:$lhs, Aud_SigType:$rhs);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($output)";
}

def Aud_ClampOp : Aud_Op<"clamp", [Pure, SameOperandsAndResultType]> {
  let summary   = "Clamp between two signals";
  let arguments = (ins Aud_SigType:$freq, F32Attr:$min, F32Attr:$max);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "$freq `,` $min `,` $max attr-dict `:` type($output)";
  let hasVerifier = 1;
}

def Aud_OscOp : Aud_Op<"osc", [Pure]> {
  let summary = "Sine oscillator (MVP: stateless, starts at phase 0)";
  let arguments = (ins F32Attr:$freq);
  let results = (outs Aud_SigType:$output);
  let assemblyFormat = "attr-dict `:` type($output)";
}

#endif // AUD_OPS