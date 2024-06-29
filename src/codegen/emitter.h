#ifndef EMITTER_H
#define EMITTER_H

#include "llvm/IR/Module.h"
#include <memory>
#include <string>

/**
 * @brief A class to emit the IR module to an object file.
 * The object file can be linked to create an executable.
 *
 */
class Emitter {
public:
    /**
     * @brief Emit the IR module to an object file.
     *
     * @param ir_module The IR module containing the IR to emit.
     * @param target_destination A string specifying the target destination for the object file. E.g. "./bin/output.o".
     * Paths are relative to CWD.
     */
    void emit(const std::unique_ptr<llvm::Module>& ir_module, const std::string& target_destination = "output.o");
};

#endif // EMITTER_H
