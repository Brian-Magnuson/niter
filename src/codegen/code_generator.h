#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "../parser/decl.h"
#include "../parser/expr.h"
#include "../parser/stmt.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>
#include <vector>

/**
 * @brief A class to perform LLVM code generation.
 * Note: This class assumes that the AST has been type-checked.
 * It does not perform type-checking, it does not check for memory safety, and it does not check for undefined behavior.
 * That is the responsibility of the type checker.
 *
 */
class CodeGenerator : public Stmt::Visitor, public Decl::Visitor, public Expr::Visitor {
    std::shared_ptr<llvm::LLVMContext> context;
    std::shared_ptr<llvm::Module> ir_module;
    std::shared_ptr<llvm::IRBuilder<>> builder;

public:
    CodeGenerator() = default;
    std::shared_ptr<llvm::Module> generate(std::vector<std::shared_ptr<Stmt>> stmts);
};

#endif // CODE_GENERATOR_H
