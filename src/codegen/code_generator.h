#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "../parser/decl.h"
#include "../parser/expr.h"
#include "../parser/stmt.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <any>
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
    // A shared pointer to the LLVM context, created in Environment.
    std::shared_ptr<llvm::LLVMContext> context;
    // The LLVM module that will be generated.
    std::shared_ptr<llvm::Module> ir_module;
    // The IR builder that will be used to generate the IR. Remember to always set the insertion point before using it.
    std::shared_ptr<llvm::IRBuilder<>> builder;

    // The current function being generated.
    // llvm::Function* current_function;
    // TODO: Determine if this is necessary.

    // A stack of blocks for control flow; break stmts will jump to the last block in this stack; return stmts will jump to the first block in this stack.
    std::vector<llvm::BasicBlock*> block_stack;

    /**
     * @brief Traverses the entire namespace tree and declares all structs.
     * Also assigns the struct type to the struct node in the tree.
     *
     */
    void declare_all_structs();

    /**
     * @brief Visits a declaration statement.
     *
     * @param stmt The declaration statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Visits an expression statement.
     *
     * @param stmt The expression statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    /**
     * @brief Visits a block statement.
     *
     * @param stmt The block statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_block_stmt(Stmt::Block* stmt) override;

    /**
     * @brief Visits a conditional statement.
     *
     * @param stmt The conditional statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;

    /**
     * @brief Visits a loop statement.
     *
     * @param stmt The loop statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;

    /**
     * @brief Visits a return statement.
     * A return statement doesn't actually create a return instruction.
     * It instead assigns "__return_val__" to the return value, then jumps to the exit block where the return instruction is created.
     * It also creates a new block for any statements that may appear after the return statement.
     *
     * @param stmt The return statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_return_stmt(Stmt::Return* stmt) override;

    std::any visit_break_stmt(Stmt::Break* stmt) override;
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;

    /**
     * @brief Visits a print statement.
     * Effectively calls `printf` from the C standard library and passes the single string argument.
     * The first time this function is called, it will declare the `printf` function. Subsequent calls will use the existing declaration.
     *
     * @param stmt The print statement to visit.
     * @return std::any nullptr always.
     */
    std::any visit_print_stmt(Stmt::Print* stmt) override;
    std::any visit_eof_stmt(Stmt::EndOfFile* stmt) override;

    /**
     * @brief Visits a variable declaration.
     *
     * @param decl The variable declaration to visit.
     * @return std::any nullptr always.
     */
    std::any visit_var_decl(Decl::Var* decl) override;

    /**
     * @brief Visits a function declaration.
     * If the function is named `main`, its linkage will be automatically set to external.
     *
     * @param decl The function declaration to visit.
     * @return std::any nullptr always.
     */
    std::any visit_fun_decl(Decl::Fun* decl) override;
    std::any visit_struct_decl(Decl::Struct* decl) override;

    std::any visit_assign_expr(Expr::Assign* expr) override;
    std::any visit_logical_expr(Expr::Logical* expr) override;
    std::any visit_binary_expr(Expr::Binary* expr) override;
    std::any visit_unary_expr(Expr::Unary* expr) override;

    /**
     * @brief Visits a call expression.
     * Generates code for the function call.
     *
     * @param expr The call expression to visit.
     * @return std::any An llvm::Value* representing the result of the call.
     */
    std::any visit_call_expr(Expr::Call* expr) override;
    std::any visit_access_expr(Expr::Access* expr) override;

    /**
     * @brief Visits a grouping expression.
     * Simply visits the expression inside the grouping and returns the Value*.
     *
     * @param expr The grouping expression to visit.
     * @return std::any An llvm::Value* representing the value of the expression inside the grouping.
     */
    std::any visit_grouping_expr(Expr::Grouping* expr) override;

    /**
     * @brief Visits an identifier expression.
     * This function specifically loads the value from the variable's llvm_allocation.
     * This function should not be used when the identifier is used as an lvalue.
     *
     * @param expr The identifier expression to visit.
     * @return std::any The llvm::Value* representing the value stored in the variable.
     */
    std::any visit_identifier_expr(Expr::Identifier* expr) override;

    /**
     * @brief Visits a literal expression.
     *
     * @param expr The literal expression to visit.
     * @return std::any An llvm::Value* representing the literal.
     */
    std::any visit_literal_expr(Expr::Literal* expr) override;
    std::any visit_array_expr(Expr::Array* expr) override;
    std::any visit_tuple_expr(Expr::Tuple* expr) override;

public:
    CodeGenerator();

    /**
     * @brief Runs the code generator on the given statements.
     * This function will generate the LLVM IR and return a pointer to the module.
     * The module will not be optimized. Use the Optimizer class to optimize the module.
     * If an error occurs during code generation or if the module cannot be verified, this function will return nullptr.
     *
     * @param stmts The statements to generate code for.
     * @param dump_ir A boolean specifying whether to dump the IR to a file. Default is false.
     * @return std::shared_ptr<llvm::Module> A pointer to the generated module. nullptr if an error occurred.
     */
    std::shared_ptr<llvm::Module> generate(std::vector<std::shared_ptr<Stmt>> stmts, bool dump_ir = false);

    /**
     * @brief Dumps the IR to a file, allowing the user to inspect the generated IR.
     * Can be called, even if the IR is not valid, allowing the user to debug the code generator.
     *
     * @param filename
     */
    void dump_ir(const std::string& filename = "./debug/bin/output.ll");
};

#endif // CODE_GENERATOR_H