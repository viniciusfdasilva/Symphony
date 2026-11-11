#ifndef AUD_TYPES
#define AUD_TYPES

include "mlir/IR/AttrTypeBase.td"
include "Aud/IR/AudDialect.td"

class Aud_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<Aud_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def Aud_SigType : TypeDef<Aud_Dialect, "Sig"> {
  let mnemonic = "sig";
  let parameters = (ins "Type":$elementType, "Type":$lengthType);
  let assemblyFormat = "`<` $elementType `,` $lengthType `>`";
}

#endif // AUD_TYPES