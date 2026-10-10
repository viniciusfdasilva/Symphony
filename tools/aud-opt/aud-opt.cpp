#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Aud/IR/AudDialect.h"
#include "Aud/Conversion/AudPasses.h"

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  mlir::aud::registerAudPasses();

  mlir::DialectRegistry registry;
  mlir::registerAllDialects(registry);
  registry.insert<mlir::aud::AudDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Aud optimizer driver\n", registry));
}
