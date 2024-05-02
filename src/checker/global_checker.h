#ifndef GLOBAL_CHECKER_H
#define GLOBAL_CHECKER_H

#include "../parser/decl.h"
#include "../parser/stmt.h"
#include "environment.h"
#include <memory>

/**
 * @brief A class for the global type checker.
 * The global type checker is the first part of a two-stage type checker.
 * This type checker specifically checks all the declarations made in global space.
 * Note: global space includes anything declared outside of a function body, so anything declared within a namespace may also be checked.
 *
 */
class GlobalChecker : public Stmt::Visitor, public Decl::Visitor {
    std::shared_ptr<Environment> environment;

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

    std::any visit_var_decl(Decl::Var* decl) override;
    std::any visit_fun_decl(Decl::Fun* decl) override;
    std::any visit_struct_decl(Decl::Struct* decl) override;

public:
    GlobalChecker(std::shared_ptr<Environment> environment) : environment(environment) {}
};

#endif // GLOBAL_CHECKER_H
