#include "optimizer.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"

void Optimizer::optimize(std::unique_ptr<llvm::Module>& ir_module) {
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;

    llvm::PassBuilder pass_builder;

    pass_builder.registerModuleAnalyses(mam);
    pass_builder.registerCGSCCAnalyses(cgam);
    pass_builder.registerFunctionAnalyses(fam);
    pass_builder.registerLoopAnalyses(lam);
    pass_builder.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::ModulePassManager mpm = pass_builder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    mpm.run(*ir_module, mam);
}
