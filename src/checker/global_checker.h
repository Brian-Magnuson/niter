#ifndef GLOBAL_CHECKER_H
#define GLOBAL_CHECKER_H

#include "../parser/decl.h"
#include "../parser/stmt.h"
#include "environment.h"
#include <any>
#include <exception>
#include <memory>
#include <vector>

/**
 * @brief An exception for global type errors.
 *
 */
class GlobalTypeException : public std::exception {};

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
     * @throw GlobalTypeException If an error occurs; will be caught by the type_check function.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Throws an exception for global expression statements.
     * No global expression statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    /**
     * @brief Returns the empty any object always.
     * This type checker does not concern itself with block statements.
     *
     * @param stmt The statement to check
     * @return std::any The empty any object always.
     */
    std::any visit_block_stmt(Stmt::Block* /*stmt*/) override { return std::any(); }

    /**
     * @brief Throws an exception for global conditional statements.
     * No global conditional statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;

    /**
     * @brief Throws an exception for global loop statements.
     * No global loop statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;

    /**
     * @brief Throws an exception for global return statements.
     * No global return statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_return_stmt(Stmt::Return* stmt) override;

    /**
     * @brief Throws an exception for global break statements.
     * No global break statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_break_stmt(Stmt::Break* stmt) override;

    /**
     * @brief Throws an exception for global continue statements.
     * No global continue statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;

    /**
     * @brief Throws an exception for global print statements.
     * No global print statements are allowed.
     *
     * @param stmt The statement to check
     * @return This function never returns.
     * @throw GlobalTypeException Always thrown. Will be caught by the type_check function.
     */
    std::any visit_print_stmt(Stmt::Print* stmt) override;

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
     * @brief Checks an external function declaration in global space.
     *
     * @param decl The external function declaration to check
     * @return std::any The empty any object always.
     */
    std::any visit_extern_fun_decl(Decl::ExternFun* decl) override;

    /**
     * @brief Checks a struct declaration in global space.
     *
     * @param decl The struct declaration to check
     * @return std::any The empty any object always.
     */
    std::any visit_struct_decl(Decl::Struct* decl) override;

public:
    GlobalChecker() = default;

    /**
     * @brief Runs the global type checker on a list of statements.
     *
     * @param stmts The list of statements to check.
     */
    void type_check(std::vector<std::shared_ptr<Stmt>> stmts);
};

#endif // GLOBAL_CHECKER_H
