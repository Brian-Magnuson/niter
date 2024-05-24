#include "emitter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

void Emitter::emit(const std::shared_ptr<llvm::Module>& ir_module, const std::string& target_destination) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    std::string error;
    auto TargetMachine = llvm::EngineBuilder().selectTarget();
    if (!TargetMachine) {
        llvm::errs() << "Unable to create target machine: " << error;
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    std::error_code EC;
    llvm::raw_fd_ostream dest(target_destination, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open output file: " << EC.message();
        return;
    }

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*ir_module);
}
