#ifndef LOCAL_CHECKER_H
#define LOCAL_CHECKER_H

#include "../utility/decl.h"
#include "../utility/expr.h"
#include "../utility/stmt.h"
#include "environment.h"
#include <any>
#include <exception>
#include <memory>
#include <vector>

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

    // The current depth of loops; useful for checking break and continue statements
    int loop_depth = 0;

    /**
     * @brief Checks if a token type is of a certain type.
     *
     * @param token The token type to check.
     * @param types The list of types to check against.
     * @return true If the token is of one of the types.
     * @return false If the token is not of one of the types.
     */
    bool check_token(TokenType token, const std::vector<TokenType>& types) const;

    /**
     * @brief Visits a declaration statement and determines if the declaration is valid.
     *
     * @param stmt The declaration statement to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Visits an expression statement and determines if the expression is valid.
     *
     * @param stmt The expression statement to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    /**
     * @brief Visits a block statement and determines if the block is valid.
     *
     * @param stmt The block statement to visit.
     * @return std::any An std::shared_ptr<Type> if the block has a return type, std::shared_ptr<Type>(nullptr) otherwise.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_block_stmt(Stmt::Block* stmt) override;

    /**
     * @brief Visits a conditional statement and determines if the condition is valid.
     *
     * @param stmt The conditional statement to visit.
     * @return std::any An std::shared_ptr<Type> if the block has a return type, std::shared_ptr<Type>(nullptr) otherwise.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;

    /**
     * @brief Visits a loop statement and determines if the loop is valid.
     *
     * @param stmt The loop statement to visit.
     * @return std::any An std::shared_ptr<Type> if the block has a return type, std::shared_ptr<Type>(nullptr) otherwise.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;

    /**
     * @brief Visits a return statement and determines if the return type is valid.
     *
     * @param stmt The return statement to visit.
     * @return std::any The std::shared_ptr<Type> of the return type.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_return_stmt(Stmt::Return* stmt) override;

    /**
     * @brief Visits a break statement and determines if the break is valid.
     *
     * @param stmt The break statement to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_break_stmt(Stmt::Break* stmt) override;

    /**
     * @brief Visits a continue statement and determines if the continue is valid.
     *
     * @param stmt The continue statement to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;

    /**
     * @brief Visits an end of file statement and determines if the end of file is valid.
     *
     * @param stmt The end of file statement to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_eof_stmt(Stmt::EndOfFile* stmt) override;

    /**
     * @brief Visits a variable declaration and determines if the initialization is valid.
     *
     * @param decl The variable declaration to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_var_decl(Decl::Var* decl) override;

    /**
     * @brief Visits a function declaration and determines if the return type and body are valid.
     *
     * @param decl The function declaration to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_fun_decl(Decl::Fun* decl) override;

    /**
     * @brief Visits an external function declaration and determines if the declaration is not in local space.
     * The global checker should have already checked the function signature.
     *
     *
     * @param decl The external function declaration to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_extern_fun_decl(Decl::ExternFun* decl) override;

    /**
     * @brief Visits a struct declaration and determines if the struct is valid.
     * Checks if the struct is not in local space, then visits each declaration.
     *
     * @param decl The struct declaration to visit.
     * @return std::any std::shared_ptr<Type>(nullptr) always.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_struct_decl(Decl::Struct* decl) override;

    /**
     * @brief Visits an assignment expression and determines if the assignment is valid.
     * Note: Valid LHS expressions are identifiers and access expressions.
     * Furthermore, the LHS expression must be mutable.
     *
     * @param expr The assignment expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_assign_expr(Expr::Assign* expr) override;

    /**
     * @brief Visits a logical expression and determines if the logical expression is valid.
     * Note: Both sides of the logical expression must be of type bool.
     *
     * @param expr The logical expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_logical_expr(Expr::Logical* expr) override;

    /**
     * @brief Visits a binary expression and determines if the binary expression is valid.
     * Note: Both sides of the binary expression must be of the same type.
     *
     * @param expr The binary expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_binary_expr(Expr::Binary* expr) override;

    /**
     * @brief Visits a unary expression and determines if the unary expression is valid.
     * Note: The unary expression must be of type int or float.
     *
     * @param expr The unary expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_unary_expr(Expr::Unary* expr) override;

    /**
     * @brief Visits a dereference expression and determines if the dereference expression is valid.
     * The inner expression must be of type pointer.
     *
     * @param expr The dereference expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_dereference_expr(Expr::Dereference* expr) override;

    /**
     * @brief Visits an access expression and determines if the access expression is valid.
     *
     * @param expr The access expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_access_expr(Expr::Access* expr) override;

    /**
     * @brief Visits an index expression and determines if the index expression is valid.
     *
     * @param expr The index expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_index_expr(Expr::Index* expr) override;

    /**
     * @brief Visits a call expression and determines if the call expression is valid.
     * Note: The function being called must exist and the arguments must match the function's parameters.
     *
     * @param expr The call expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_call_expr(Expr::Call* expr) override;

    /**
     * @brief Visits a cast expression and determines if the cast expression is valid.
     * Cast expressions are type conversions that use the `as` keyword.
     * Only a few types can be casted to each other.
     *
     * @param expr The cast expression to visit.
     * @return std::any An std::shared_ptr<Type> representing the type of the casted expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_cast_expr(Expr::Cast* expr) override;

    /**
     * @brief Visits a grouping expression and determines if the grouping expression is valid.
     *
     * @param expr The grouping expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_grouping_expr(Expr::Grouping* expr) override;

    /**
     * @brief Visits an identifier expression and determines if the identifier expression is valid.
     * Note: The identifier must exist and be mutable.
     *
     * @param expr The identifier expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_identifier_expr(Expr::Identifier* expr) override;

    /**
     * @brief Visits a literal expression and determines if the literal expression is valid.
     *
     * @param expr The literal expression to visit.
     * @return An std::shared_ptr<Type> representing the type of the expression.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_literal_expr(Expr::Literal* expr) override;

    /**
     * @brief Visits an array expression and determines if the array expression is valid.
     *
     * @param expr The array expression to visit.
     * @return std::any An std::shared_ptr<Type> representing the type of the array.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_array_expr(Expr::Array* expr) override;

    /**
     * @brief Visit an array generator expression and determine if the array generator expression is valid.
     *
     * @param expr The array generator expression to visit.
     * @return std::any An std::shared_ptr<Type> representing the type of the array.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_array_gen_expr(Expr::ArrayGen* expr) override;

    /**
     * @brief Visits a tuple expression and determines if the tuple expression is valid.
     *
     * @param expr The tuple expression to visit.
     * @return std::any An std::shared_ptr<Type> representing the type of the tuple.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_tuple_expr(Expr::Tuple* expr) override;

    /**
     * @brief Visits an object expression and determines if the object expression is valid.
     * An object expression is valid if:
     * - The struct exists
     * - The object expression has the same fields as the struct
     * - The object expression does not have any extra fields or static fields
     * - The types of the fields match the types of the struct fields
     * - Any fields missing from the object have default values in the struct declaration
     * (We currently do not support default values in struct declarations, so this is not implemented yet.)
     *
     * @param expr The object expression to visit.
     * @return std::any An std::shared_ptr<Type> representing the type of the object.
     * @throws LocalTypeException If an error occurs during type checking. Will be caught by the type_check function.
     */
    std::any visit_object_expr(Expr::Object* expr) override;

public:
    /**
     * @brief Runs the local type checker on a list of statements.
     *
     * @param stmts The list of statements to check.
     */
    void type_check(std::vector<std::shared_ptr<Stmt>> stmts);
};

#endif // LOCAL_CHECKER_H
