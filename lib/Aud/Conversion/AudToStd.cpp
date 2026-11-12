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

#define DEBUG_TYPE "symphony-debug"

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

static Value generate_sound(WaveShape shape, Value k, OpBuilder b, Location l, Value i)
{

  auto cst = [&](double v) -> Value{
    return arith::ConstantOp::create(b, l, b.getF32FloatAttr(v));
  };

  auto frac = [&](Value x) -> Value{
    return arith::SubFOp::create(b, l, x, math::FloorOp::create(b, l, x));
  };

  Value iI = arith::IndexCastOp::create(b, l, b.getI64Type(), i);
  Value iF = arith::SIToFPOp::create(b, l, b.getF32Type(), iI);
  Value p  = frac(arith::MulFOp::create(b, l, iF, k));

  switch (shape)
  {
    case WaveShape::Sine: {
      Value x = arith::MulFOp::create(b, l, p, cst(2.0 * M_PI));
      return math::SinOp::create(b, l, x);
    }

    case WaveShape::Square: {
      Value lt = arith::CmpFOp::create(b, l, arith::CmpFPredicate::OLT, p, cst(0.5));
      return arith::SelectOp::create(b, l, lt, cst(1.0), cst(-1.0));
    }

    case WaveShape::Pulse: {
      Value d  = cst(0.5);
      Value lo = arith::MulFOp::create(b, l, arith::SubFOp::create(b, l, cst(1.0), d), cst(0.5));
      Value hi = arith::MulFOp::create(b, l, arith::AddFOp::create(b, l, cst(1.0), d), cst(0.5));
      Value geLo = arith::CmpFOp::create(b, l, arith::CmpFPredicate::OGE, p, lo);
      Value ltHi = arith::CmpFOp::create(b, l, arith::CmpFPredicate::OLT, p, hi);
      Value in   = arith::AndIOp::create(b, l, geLo, ltHi);
      return arith::SelectOp::create(b, l, in, cst(1.0), cst(0.0));
    }

    case WaveShape::Saw: {
      Value x = arith::MulFOp::create(b, l, p, cst(2.0));
      return arith::SubFOp::create(b, l, x, cst(1.0));
    }

    case WaveShape::Triangle: {
      Value d = math::AbsFOp::create(b, l, arith::SubFOp::create(b, l, p, cst(0.5)));
      return arith::SubFOp::create(b, l, cst(1.0), arith::MulFOp::create(b, l, d, cst(4.0)));
    }
  }
  llvm_unreachable("unhandled WaveShape");
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
    Value lhs = adaptor.getLhs(), rhs = adaptor.getRhs();

    // tamanho lido do operando (index)
    Value n = memref::DimOp::create(rw, loc, lhs, 0);
    Value out = memref::AllocOp::create(rw, loc, ty, ValueRange{n});

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

    WaveShape shape = osc.getShape();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    Value out = memref::AllocOp::create(rw, loc, ty);

    int32_t semitone = op.getSemitone();
    double newHz = osc.getFreq().convertToDouble() * std::pow(2.0, semitone / 12.0);
    double step = 2.0 * M_PI * newHz / sampleRate;
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(step));

    buildLoop(rw, loc, ty.getDimSize(0),
              [&](OpBuilder &b, Location l, Value i) {
                Value s = generate_sound(shape, k, b, l, i);
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

    double freq = op.getFreqAttr().getValueAsDouble();
    WaveShape shape = op.getShape();

    int64_t samples = op.getSamplesAttr().getInt();

    Value out;
    if (ty.isDynamicDim(0)) {
      Value n = arith::ConstantIndexOp::create(rw, loc, samples);
      out = memref::AllocOp::create(rw, loc, ty, ValueRange{n});
    } else {
      if (ty.getDimSize(0) != samples)
        return rw.notifyMatchFailure(op, "result size doesn't match samples");
      out = memref::AllocOp::create(rw, loc, ty);
    }

    double k = freq / sampleRate;
    Value kV = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(k));

    buildLoop(rw, loc, samples, [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(shape, kV, b, l, i);
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

    if (!lhs || !rhs)
      return rw.notifyMatchFailure(op, "avg operands must be produced by aud.osc");

    double freq_lhs_in_hz = lhs.getFreqAttr().getValueAsDouble();
    double freq_rhs_in_hz = rhs.getFreqAttr().getValueAsDouble();

    OscOp winner = (freq_lhs_in_hz >= freq_rhs_in_hz) ? lhs : rhs;
    WaveShape shape = winner.getShape();

    double avg_freq_in_hz  = static_cast<double>((freq_lhs_in_hz+freq_rhs_in_hz)/2.0);
    double step = 2.0 * M_PI * avg_freq_in_hz / sampleRate;
    
    Value k = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(step));

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(shape, k, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};


struct MinLowering : OpConversionPattern<MinOp> {
  MinLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<MinOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(MinOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
    auto lhs = op.getLhs().getDefiningOp<OscOp>();
    auto rhs = op.getRhs().getDefiningOp<OscOp>();

    if (!lhs || !rhs)
      return rw.notifyMatchFailure(op, "min operands must be produced by aud.osc");

    double freq_lhs_in_hz = lhs.getFreqAttr().getValueAsDouble();
    double freq_rhs_in_hz = rhs.getFreqAttr().getValueAsDouble();

    OscOp winner = (freq_lhs_in_hz >= freq_rhs_in_hz) ? lhs : rhs;
    WaveShape shape = winner.getShape();
    double min_freq_in_hz  = static_cast<double>(std::min(freq_lhs_in_hz, freq_rhs_in_hz));
    
    double k = min_freq_in_hz / sampleRate;
    Value out = memref::AllocOp::create(rw, loc, ty);
    Value kV  = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(k));

    buildLoop(rw, loc, ty.getDimSize(0), [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(shape, kV, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};


struct MaxLowering : OpConversionPattern<MaxOp> {
  MaxLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<MaxOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(MaxOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));

    auto lhs = op.getLhs().getDefiningOp<OscOp>();
    auto rhs = op.getRhs().getDefiningOp<OscOp>();
    if (!lhs || !rhs)
      return rw.notifyMatchFailure(op, "max operands must be produced by aud.osc");

    double freqLhs = lhs.getFreqAttr().getValueAsDouble();
    double freqRhs = rhs.getFreqAttr().getValueAsDouble();

    OscOp winner = (freqLhs >= freqRhs) ? lhs : rhs;
    WaveShape shape = winner.getShape();
    double maxFreqHz = std::max(freqLhs, freqRhs);

    double k = maxFreqHz / sampleRate;                  // ciclos/amostra, sem 2π

    Value out = memref::AllocOp::create(rw, loc, ty);
    Value kV  = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(k));

    buildLoop(rw, loc, ty.getDimSize(0),
              [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(shape, kV, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};


struct ClampLowering : OpConversionPattern<ClampOp> {
  ClampLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
      : OpConversionPattern<ClampOp>(tc, ctx), sampleRate(sampleRate) {}

  LogicalResult
  matchAndRewrite(ClampOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rw) const override {
    Location loc = op.getLoc();
    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));

    // o sinal de entrada precisa vir direto de uma aud.osc
    auto osc = op.getFreq().getDefiningOp<OscOp>();
    if (!osc)
      return rw.notifyMatchFailure(op, "clamp operand must be produced by aud.osc");

    WaveShape shape = osc.getShape();                         // shape vem da osc
    double freqHz   = osc.getFreqAttr().getValueAsDouble();

    double lo = adaptor.getMin().convertToDouble();
    double hi = adaptor.getMax().convertToDouble();
    if (lo > hi)
      return rw.notifyMatchFailure(op, "min greater than max");

    double clampedHz = std::clamp(freqHz, lo, hi);
    double k = clampedHz / sampleRate;                        // ciclos/amostra, sem 2π

    Value out = memref::AllocOp::create(rw, loc, ty);
    Value kV  = arith::ConstantOp::create(rw, loc, rw.getF32FloatAttr(k));

    buildLoop(rw, loc, ty.getDimSize(0),
              [&](OpBuilder &b, Location l, Value i) {
      Value s = generate_sound(shape, kV, b, l, i);
      memref::StoreOp::create(b, l, s, out, ValueRange{i});
    });

    rw.replaceOp(op, out);
    return success();
  }

private:
  double sampleRate;
};

//struct ReverbLowering : OpConversionPattern<ReverbOp> {
//  ReverbLowering(const TypeConverter &tc, MLIRContext *ctx, double sampleRate)
//      : OpConversionPattern<ReverbOp>(tc, ctx), sampleRate(sampleRate) {}
//
//  LogicalResult
//  matchAndRewrite(ReverbOp op, OpAdaptor adaptor,
//                  ConversionPatternRewriter &rw) const override {
//    Location loc = op.getLoc();
//    auto ty = cast<MemRefType>(getTypeConverter()->convertType(op.getType()));
//    Value out = memref::AllocOp::create(rw, loc, ty);
//   
//    float delay = op.getDecayAttr().getValueAsDouble();
//    float decay = op.getDelayAttr().getValueAsDouble();
//    auto freq_oscop     = op.getFreq().getDefiningOp<OscOp>();
//    auto sample_oscop   = op.getSamples().getDefiningOp<OscOp>();
//    
//    float freq_in_hz = freq_oscop.getFreqAttr().getValueAsDouble();
//    uint64_t sample = sample_oscop.getSamplesAttr().getValueAsInteger();
//    double step = 2.0 * M_PI * freq_in_hz / sampleRate;
//
//    float g = std::pow(10, (-3*delay/(decay*sample)));
//
//    rw.replaceOp(op, out);
//    return success();
//  }
//
//private:
//  double sampleRate;
//};

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

    tc.addConversion([](SigType t) -> Type {
      return MemRefType::get({ShapedType::kDynamic}, t.getElementType());
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
    patterns.add<OscLowering, TransposeLowering, AvgLowering, MinLowering, MaxLowering, ClampLowering>(tc, ctx, sr);
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