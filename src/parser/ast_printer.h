#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "expr.h"
#include "stmt.h"
#include <any>
#include <memory>
#include <string>
#include <vector>

class AstPrinter : public Expr::Visitor, public Stmt::Visitor {
public:
    /**
     * @brief Prints a string representation of an expression.
     *
     * @param expr The expression to print.
     * @return std::string The string representation of the expression.
     */
    std::string print(std::shared_ptr<Expr> expr);

    /**
     * @brief Prints a string representation of a statement.
     *
     * @param stmt The statement to print.
     * @return std::string The string representation of the statement.
     */
    std::string print(std::shared_ptr<Stmt> stmt);

    /**
     * @brief Creates a parenthesized string representation of an expression.
     * E.g. "1 + 2 * 3;" -> "(+ 1 (* 2 3))"
     *
     * @param name The name of the expression.
     * @param exprs The vector of expressions to parenthesize.
     * @return std::string The parenthesized string representation of the expression.
     */
    std::string parenthesize(const std::string& name, const std::vector<std::shared_ptr<Expr>>& exprs);

    /**
     * @brief Visits a binary expression and returns a string representation of it.
     *
     * @param expr The binary expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_binary_expr(Expr::Binary* expr) override;

    /**
     * @brief Visits a grouping expression and returns a string representation of it.
     *
     * @param expr The grouping expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_grouping_expr(Expr::Grouping* expr) override;

    /**
     * @brief Visits a literal expression and returns a string representation of it.
     *
     * @param expr The literal expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_literal_expr(Expr::Literal* expr) override;

    /**
     * @brief Visits a unary expression and returns a string representation of it.
     *
     * @param expr The unary expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_unary_expr(Expr::Unary* expr) override;

    /**
     * @brief Visits a variable expression and returns a string representation of it.
     *
     * @param expr The variable expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_variable_expr(Expr::Variable* expr) override;

    /**
     * @brief Visits an assign expression and returns a string representation of it.
     *
     * @param expr The assign expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_assign_expr(Expr::Assign* expr) override;

    /**
     * @brief Visits a logical expression and returns a string representation of it.
     *
     * @param expr The logical expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_logical_expr(Expr::Logical* expr) override;

    /**
     * @brief Visits a call expression and returns a string representation of it.
     *
     * @param expr The call expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_call_expr(Expr::Call* expr) override;

    /**
     * @brief Visits a get expression and returns a string representation of it.
     *
     * @param expr The get expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    /**
     * @brief Visits a print statement and returns a string representation of it.
     *
     * @param stmt The print statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_print_stmt(Stmt::Print* stmt) override;

    /**
     * @brief Visits a var statement and returns a string representation of it.
     *
     * @param stmt The var statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_var_stmt(Stmt::Var* stmt) override;

    /**
     * @brief Visits a block statement and returns a string representation of it.
     *
     * @param stmt The block statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_block_stmt(Stmt::Block* stmt) override;

    /**
     * @brief Visits an if statement and returns a string representation of it.
     *
     * @param stmt The if statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_if_stmt(Stmt::If* stmt) override;

    /**
     * @brief Visits a while statement and returns a string representation of it.
     *
     * @param stmt The while statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_while_stmt(Stmt::While* stmt) override;

    /**
     * @brief Visits a for statement and returns a string representation of it.
     *
     * @param stmt The for statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_for_stmt(Stmt::For* stmt) override;

    /**
     * @brief Visits a return statement and returns a string representation of it.
     *
     * @param stmt The return statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_return_stmt(Stmt::Return* stmt) override;

    /**
     * @brief Visits a break statement and returns a string representation of it.
     *
     * @param stmt The break statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_break_stmt(Stmt::Break* stmt) override;

    /**
     * @brief Visits a continue statement and returns a string representation of it.
     *
     * @param stmt The continue statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    // std::any visit_continue_stmt(Stmt::Continue* stmt) override;
};

#endif // AST_PRINTER_H
