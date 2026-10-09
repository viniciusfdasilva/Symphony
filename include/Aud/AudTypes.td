#ifndef AUD_TYPES
#define AUD_TYPES

include "mlir/IR/AttrTypeBase.td"
include "Aud/AudDialect.td"

class Aud_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<Aud_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def Aud_SigType : Aud_Type<"Sig", "sig"> {
  let summary = "Audio-rate signal";
  let parameters = (ins "Type":$elementType);
  let assemblyFormat = "`<` $elementType `>`";
}

#endif // AUD_TYPES