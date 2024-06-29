#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "llvm/IR/Module.h"
#include <memory>

/**
 * @brief A class to perform optimization on an IR module.
 * Optimization helps remove unnecessary code and make the code more efficient.
 *
 */
class Optimizer {
public:
    /**
     * @brief Optimizes the given IR module.
     * Uses optimization level O2.
     * This step is optional and may be skipped if you want to see the unoptimized IR.
     *
     * @param ir_module The IR module to optimize.
     */
    void optimize(std::unique_ptr<llvm::Module>& ir_module);
};

#endif // OPTIMIZER_H
