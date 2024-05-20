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
    std::shared_ptr<llvm::LLVMContext> context;
    std::shared_ptr<llvm::Module> ir_module;
    std::shared_ptr<llvm::IRBuilder<>> builder;

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

    std::any visit_expression_stmt(Stmt::Expression* stmt) override;
    std::any visit_block_stmt(Stmt::Block* stmt) override;
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;
    std::any visit_return_stmt(Stmt::Return* stmt) override;
    std::any visit_break_stmt(Stmt::Break* stmt) override;
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;
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
    std::any visit_call_expr(Expr::Call* expr) override;
    std::any visit_access_expr(Expr::Access* expr) override;
    std::any visit_grouping_expr(Expr::Grouping* expr) override;
    std::any visit_identifier_expr(Expr::Identifier* expr) override;
    std::any visit_literal_expr(Expr::Literal* expr) override;
    std::any visit_array_expr(Expr::Array* expr) override;
    std::any visit_tuple_expr(Expr::Tuple* expr) override;

public:
    CodeGenerator();
    std::shared_ptr<llvm::Module> generate(std::vector<std::shared_ptr<Stmt>> stmts);
};

#endif // CODE_GENERATOR_H
