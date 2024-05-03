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

    /**
     * @brief Checks a declaration statement in global space.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with expression statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_expression_stmt(Stmt::Expression* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with block statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_block_stmt(Stmt::Block* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with conditional statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with loop statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_loop_stmt(Stmt::Loop* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with return statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_return_stmt(Stmt::Return* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with break statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_break_stmt(Stmt::Break* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with continue statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_continue_stmt(Stmt::Continue* /*stmt*/) override { return std::any(); }

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with print statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_print_stmt(Stmt::Print* /*stmt*/) override { return std::any(); }

    /**
     * @brief Checks the end of file statement. May alter the environment accordingly.
     *
     * @param stmt The statement to check (ignored)
     * @return std::any The empty any object always.
     */
    std::any visit_eof_stmt(Stmt::EndOfFile* stmt) override;

    /**
     * @brief Checks a variable declaration in global space. This function is never called on local variable declarations (declarations inside a function body).
     *
     * @param decl The variable declaration to check
     * @return std::any The empty any object always.
     */
    std::any visit_var_decl(Decl::Var* decl) override;

    /**
     * @brief Checks a function declaration in global space.
     * If the function is named `main`, this function will also check if the function has the correct signature.
     * This function should not call visit on the function body, as the function body is checked by the LocalChecker.
     *
     * @param decl The function declaration to check
     * @return std::any The empty any object always.
     */
    std::any visit_fun_decl(Decl::Fun* decl) override;

    /**
     * @brief Checks a struct declaration in global space.
     *
     * @param decl The struct declaration to check
     * @return std::any The empty any object always.
     */
    std::any visit_struct_decl(Decl::Struct* decl) override;

public:
    GlobalChecker(std::shared_ptr<Environment> environment) : environment(environment) {}
};

#endif // GLOBAL_CHECKER_H
