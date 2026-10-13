#include "Aud/Conversion/AudPasses.h"
#include "Aud/IR/AudDialect.h"
#include "Aud/IR/AudOps.h"
#include "Aud/IR/AudTypes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

#include "llvm/Support/Debug.h"
#include <cmath>

#define DEBUG_TYPE "transpose-debug"

using namespace mlir;
using namespace mlir::aud;

namespace {

// Constrói: for i in [0, n) { body(i) }
template <typename BodyFn>
void buildLoop(OpBuilder &b, Location loc, int64_t n, BodyFn body) {
  Value c0 = arith::ConstantIndexOp::create(b, loc, 0);
  Value c1 = arith::ConstantIndexOp::create(b, loc, 1);
  Value cN = arith::ConstantIndexOp::create(b, loc, n);
  scf::ForOp::create(b, loc, c0, cN, c1, ValueRange{},
                     [&](OpBuilder &nb, Location l, Value i, ValueRange) {
                       body(nb, l, i);
                       scf::YieldOp::create(nb, l);
                     });
}

static Value generate_sound(Value k, OpBuilder b, Location l, Value i)
  {
    Value iI = arith::IndexCastOp::create(b, l, b.getI64Type(), i);
    Value iF = arith::SIToFPOp::create(b, l, b.getF32Type(), iI);
    Value ph = arith::MulFOp::create(b, l, iF, k);
    return math::SinOp::create(b, l, ph);
}

struct GainLowering : OpConversionPattern<GainOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(GainOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);

    double db = op.getDbAttr().getValueAsDouble();
    double lin = std::pow(10.0, db / 20.0);
    Value g = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(lin));
    Value in = adaptor.getInput();

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value x = memref::LoadOp::create(b, l, in, ValueRange{i});
      Value y = arith::MulFOp::create(b, l, x, g);
      memref::StoreOp::create(b, l, y, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }
};

struct AddLowering : OpConversionPattern<AddOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(AddOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);
    Value lhs = adaptor.getLhs(), rhs = adaptor.getRhs();

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value a = memref::LoadOp::create(b, l, lhs, ValueRange{i});
      Value c = memref::LoadOp::create(b, l, rhs, ValueRange{i});
      Value s = arith::AddFOp::create(b, l, a, c);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }
};


struct SubLowering : OpConversionPattern<SubOp> {
  using OpConversionPattern::OpConversionPattern;
  
  LogicalResult matchAndRewrite(SubOp op, OpAdaptor adaptor, ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);
    Value lhs = adaptor.getLhs(), rhs = adaptor.getRhs();

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value a = memref::LoadOp::create(b, l, lhs, ValueRange{i});
      Value c = memref::LoadOp::create(b, l, rhs, ValueRange{i});
      Value s = arith::SubFOp::create(b, l, a, c);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }
};

struct MulLowering : OpConversionPattern<MulOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(MulOp op, OpAdaptor adaptor, ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out      = memref::AllocOp::create(rw, loc, ty);
    Value freq     = adaptor.getFreq();
    float factor = static_cast<float>(adaptor.getFactor().convertToDouble());
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(factor));
    
    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i){
      Value x = memref::LoadOp::create(b,l, freq, ValueRange{i});
      Value y = arith::MulFOp::create(b, l, x, k);
      memref::StoreOp::create(b, l, y, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }
};


struct DivLowering : OpConversionPattern<DivOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(DivOp op, OpAdaptor adaptor, ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();

    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out  = memref::AllocOp::create(rw, loc, ty);
    Value freq = adaptor.getFreq();
    float factor = static_cast<float>(adaptor.getFactor().convertToDouble());
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(factor));

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i){
      Value x = memref::LoadOp::create(b, l, freq, ValueRange{i});
      Value y = arith::DivFOp::create(b, l, x, k);
      memref::StoreOp::create(b,l,y,out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }
};

struct TransposeLowering : OpConversionPattern<TransposeOp> {
  TransposeLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<TransposeOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(TransposeOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();

    auto osc = op.getFreq().getDefiningOp<OscOp>();
    if (!osc)
      return rw.notifyMatchFailure(op, "input must come directly from aud.osc");

    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);

    int32_t semitone = op.getSemitone();
    double newHz = osc.getFreq().convertToDouble() * std::pow(2.0, semitone / 12.0);
    double step = 2.0 * M_PI * newHz / sampleRate;
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(step));

    buildLoop(rw, loc, ty.getDimSize(0),
              [&](OpBuilder &b, Location l, Value i) {
                Value s = generate_sound(k, b, l, i);
                memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

  private:
    double sampleRate;
};

struct OscLowering : OpConversionPattern<OscOp> {
  OscLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<OscOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(OscOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);

    double freq = op.getFreqAttr().getValueAsDouble();

    double step = 2.0 * M_PI * freq / sampleRate;
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(step));

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      
      Value s = generate_sound(k, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};

struct AvgLowering : OpConversionPattern<AvgOp> {
  AvgLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<AvgOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(AvgOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);

    auto lhs = op.getLhs().getDefiningOp<OscOp>();
    auto rhs = op.getRhs().getDefiningOp<OscOp>();

    double freq_lhs_in_hz = lhs.getFreqAttr().getValueAsDouble();
    double freq_rhs_in_hz = rhs.getFreqAttr().getValueAsDouble();

    double avg_freq_in_hz  = static_cast<double>((freq_lhs_in_hz+freq_rhs_in_hz)/2.0);
    double step = 2.0 * M_PI * avg_freq_in_hz / sampleRate;
    
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(step));

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(k, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};

struct CastLowering : OpConversionPattern<UnrealizedConversionCastOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(UnrealizedConversionCastOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    if (adaptor.getOperands().size() != 1 || op->getNumResults() != 1)
      return failure();
    Value in = adaptor.getOperands()[0];
    if (in.getType() != op.getResult(0).getType())
      return failure();
    rw.replaceOp(op, in);
    return success();
  }
};

struct ConvertAudToStdPass
    : PassWrapper<ConvertAudToStdPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConvertAudToStdPass)

  ConvertAudToStdPass() = default;
  ConvertAudToStdPass(const ConvertAudToStdPass &other) : PassWrapper(other) {}

  Option<int64_t> blockSize{*this, "block-size",
                            llvm::cl::desc("Samples per block"),
                            llvm::cl::init(128)};
  Option<double> sampleRate{*this, "sample-rate",
                            llvm::cl::desc("Sample rate in Hz"),
                            llvm::cl::init(48000.0)};

  StringRef getArgument() const final { return "convert-aud-to-std"; }
  StringRef getDescription() const final {
    return "Lower the aud dialect to scf/arith/math/memref";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect, func::FuncDialect, math::MathDialect,
                    memref::MemRefDialect, scf::SCFDialect>();
  }

  void runOnOperation() override {
    MLIRContext *ctx = &getContext();
    int64_t n = blockSize;
    double sr = sampleRate;

    TypeConverter tc;
    tc.addConversion([](Type t) { return t; });
    tc.addConversion([n](SigType t) -> Type {
      return MemRefType::get({n}, t.getElementType());
    });

    ConversionTarget target(*ctx);
    target.addLegalDialect<arith::ArithDialect, math::MathDialect,
                           memref::MemRefDialect, scf::SCFDialect>();
    target.addLegalOp<ModuleOp>();
    target.addDynamicallyLegalOp<UnrealizedConversionCastOp>(
        [&](UnrealizedConversionCastOp op) {
        return tc.isLegal(op.getOperandTypes());
    });
    target.addIllegalDialect<AudDialect>();
    target.addDynamicallyLegalOp<func::FuncOp>([&](func::FuncOp op) {
      return tc.isSignatureLegal(op.getFunctionType()) &&
             tc.isLegal(&op.getBody());
    });
    target.addDynamicallyLegalOp<func::ReturnOp>(
        [&](func::ReturnOp op) { return tc.isLegal(op.getOperandTypes()); });
    target.addDynamicallyLegalOp<func::CallOp>([&](func::CallOp op) {
      return tc.isLegal(op.getOperandTypes()) &&
             tc.isLegal(op.getResultTypes());
    });

    RewritePatternSet patterns(ctx);
    patterns.add<GainLowering, AddLowering, SubLowering, CastLowering, MulLowering, DivLowering>(tc, ctx);
    patterns.add<OscLowering, TransposeLowering, AvgLowering>(tc, ctx, sr);
    populateFunctionOpInterfaceTypeConversionPattern<func::FuncOp>(patterns, tc);
    populateReturnOpTypeConversionPattern(patterns, tc);
    populateCallOpTypeConversionPattern(patterns, tc);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

void mlir::aud::registerAudPasses() { PassRegistration<ConvertAudToStdPass>(); }