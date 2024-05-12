#ifndef LOCAL_CHECKER_H
#define LOCAL_CHECKER_H

#include "../parser/decl.h"
#include "../parser/expr.h"
#include "../parser/stmt.h"
#include "environment.h"
#include <any>
#include <exception>
#include <memory>

/**
 * @brief An exception for local type errors.
 *
 */
class LocalTypeException : public std::exception {};

/**
 * @brief A class for the local type checker.
 * The local type checker is the second part of a two-stage type checker.
 * This type checker specifically checks all the declarations made in local space.
 * Note: local space includes anything declared within a function body.
 * Theoretically, this type checker should only have to make one pass over the code since all types are made known from the global checker.
 *
 */
class LocalChecker : public Stmt::Visitor, public Decl::Visitor, public Expr::Visitor {

    /**
     * @brief Visits a declaration statement and determines if the declaration is valid.
     *
     * @param stmt The declaration statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Visits an expression statement and determines if the expression is valid.
     *
     * @param stmt The expression statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    /**
     * @brief Visits a block statement and determines if the block is valid.
     *
     * @param stmt The block statement to visit.
     * @return std::any An std::shared_ptr<Annotation> if the block has a return type, the empty std::any value otherwise.
     */
    std::any visit_block_stmt(Stmt::Block* stmt) override;

    /**
     * @brief Visits a conditional statement and determines if the condition is valid.
     *
     * @param stmt The conditional statement to visit.
     * @return std::any An std::shared_ptr<Annotation> if the block has a return type, the empty std::any value otherwise.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;

    /**
     * @brief Visits a loop statement and determines if the loop is valid.
     *
     * @param stmt The loop statement to visit.
     * @return std::any An std::shared_ptr<Annotation> if the block has a return type, the empty std::any value otherwise.
     */
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;

    /**
     * @brief Visits a return statement and determines if the return type is valid.
     *
     * @param stmt The return statement to visit.
     * @return std::any The std::shared_ptr<Annotation> of the return type.
     */
    std::any visit_return_stmt(Stmt::Return* stmt) override;

    /**
     * @brief Visits a break statement and determines if the break is valid.
     *
     * @param stmt The break statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_break_stmt(Stmt::Break* stmt) override;

    /**
     * @brief Visits a continue statement and determines if the continue is valid.
     *
     * @param stmt The continue statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;

    /**
     * @brief Visits a print statement and determines if the print is valid.
     *
     * @param stmt The print statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_print_stmt(Stmt::Print* stmt) override;

    /**
     * @brief Visits an end of file statement and determines if the end of file is valid.
     *
     * @param stmt The end of file statement to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_eof_stmt(Stmt::EndOfFile* stmt) override;

    /**
     * @brief Visits a variable declaration and determines if the initialization is valid.
     *
     * @param decl The variable declaration to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_var_decl(Decl::Var* decl) override;

    /**
     * @brief Visits a function declaration and determines if the return type and body are valid.
     *
     * @param decl The function declaration to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_fun_decl(Decl::Fun* decl) override;

    /**
     * @brief Does nothing for struct declarations. Struct declarations are handled in the global checker.
     *
     * @param decl The struct declaration to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_struct_decl(Decl::Struct* /* decl */) override { return std::any(); }

    /**
     * @brief Visits an assignment expression and determines if the assignment is valid.
     * Note: Valid LHS expressions are identifiers and access expressions.
     * Furthermore, the LHS expression must be mutable.
     *
     * @param expr The assignment expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_assign_expr(Expr::Assign* expr) override;

    /**
     * @brief Visits a logical expression and determines if the logical expression is valid.
     * Note: Both sides of the logical expression must be of type bool.
     *
     * @param expr The logical expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_logical_expr(Expr::Logical* expr) override;

    /**
     * @brief Visits a binary expression and determines if the binary expression is valid.
     * Note: Both sides of the binary expression must be of the same type.
     *
     * @param expr The binary expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_binary_expr(Expr::Binary* expr) override;

    /**
     * @brief Visits a unary expression and determines if the unary expression is valid.
     * Note: The unary expression must be of type int or float.
     *
     * @param expr The unary expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_unary_expr(Expr::Unary* expr) override;

    /**
     * @brief Visits a call expression and determines if the call expression is valid.
     * Note: The function being called must exist and the arguments must match the function's parameters.
     *
     * @param expr The call expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_call_expr(Expr::Call* expr) override;

    /**
     * @brief Visits an access expression and determines if the access expression is valid.
     * Note: The access expression must be of type array or tuple.
     *
     * @param expr The access expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_access_expr(Expr::Access* expr) override;

    /**
     * @brief Visits a grouping expression and determines if the grouping expression is valid.
     *
     * @param expr The grouping expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_grouping_expr(Expr::Grouping* expr) override;

    /**
     * @brief Visits an identifier expression and determines if the identifier expression is valid.
     * Note: The identifier must exist and be mutable.
     *
     * @param expr The identifier expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_identifier_expr(Expr::Identifier* expr) override;

    /**
     * @brief Visits a literal expression and determines if the literal expression is valid.
     *
     * @param expr The literal expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_literal_expr(Expr::Literal* expr) override;

    /**
     * @brief Visits an array expression and determines if the array expression is valid.
     *
     * @param expr The array expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_array_expr(Expr::Array* expr) override;

    /**
     * @brief Visits a tuple expression and determines if the tuple expression is valid.
     *
     * @param expr The tuple expression to visit.
     * @return std::any The empty std::any value always.
     */
    std::any visit_tuple_expr(Expr::Tuple* expr) override;

public:
    /**
     * @brief Runs the local type checker on a list of statements.
     *
     * @param stmts The list of statements to check.
     */
    void type_check(std::vector<std::shared_ptr<Stmt>> stmts);
};

#endif // LOCAL_CHECKER_H
