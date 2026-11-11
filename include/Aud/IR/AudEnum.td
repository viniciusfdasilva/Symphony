include "mlir/IR/EnumAttr.td"
include "mlir/IR/AttrTypeBase.td"
include "Aud/IR/AudDialect.td"

def Aud_WaveSine     : I32EnumAttrCase<"Sine", 0, "sine">;
def Aud_WaveSquare   : I32EnumAttrCase<"Square", 1, "square">;
def Aud_WavePulse    : I32EnumAttrCase<"Pulse", 2, "pulse">;
def Aud_WaveSaw      : I32EnumAttrCase<"Saw", 3, "saw">;
def Aud_WaveTriangle : I32EnumAttrCase<"Triangle", 4, "triangle">;

def Aud_WaveShape : I32EnumAttr<"WaveShape", "waveform shape",
    [Aud_WaveSine, Aud_WaveSquare, Aud_WavePulse, Aud_WaveSaw,
     Aud_WaveTriangle]> {
  let cppNamespace = "::mlir::aud";
  let genSpecializedAttr = 0;   // vamos definir o attr manualmente
}

def Aud_WaveShapeAttr : EnumAttr<Aud_Dialect, Aud_WaveShape, "wave"> {
  let assemblyFormat = "`<` $value `>`";
}