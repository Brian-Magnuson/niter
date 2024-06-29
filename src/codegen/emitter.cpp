#include "emitter.h"
#include "../logger/logger.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

void Emitter::emit(const std::unique_ptr<llvm::Module>& ir_module, const std::string& target_destination) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto TargetMachine = llvm::EngineBuilder()
                             .setRelocationModel(llvm::Reloc::Model::PIC_)
                             .selectTarget();

    if (!TargetMachine) {
        ErrorLogger::inst().log_error(E_NO_TARGET_MACHINE, "Could not select target machine");
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::ObjectFile;

    std::error_code EC;
    llvm::raw_fd_ostream dest(target_destination, EC, llvm::sys::fs::OF_None);

    if (EC) {
        ErrorLogger::inst().log_error(E_INVALID_OUTPUT, "Could not open file `" + target_destination + "` due to error: " + EC.message());
        return;
    }

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        ErrorLogger::inst().log_error(E_INVALID_OUTPUT_TYPE, "Could not emit a file of the specified type");
        return;
    }

    pass.run(*ir_module);
}
