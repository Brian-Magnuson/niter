#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "../utility/expr.h"
#include "../utility/stmt.h"
#include <any>
#include <memory>
#include <string>
#include <vector>

class AstPrinter : public Expr::Visitor, public Stmt::Visitor, public Decl::Visitor {
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
     * @brief Converts a double value to a string with a specified precision using a stringstream object.
     *
     * @param value The double value to convert.
     * @param precision The number of decimal places to include in the string. Defaults to 4.
     * @return std::string The string representation of the double value.
     */
    std::string double_to_string(double value, int precision = 4);

    /**
     * @brief Converts an any object to a string.
     * any values can be either longs, doubles, bools, chars, or strings.
     * Anything else will be printed as [object].
     *
     * @param value The any object to convert.
     * @return std::string The string representation of the any object.
     */
    std::string any_to_string(const std::any& value);

    // MARK: Statements

    /**
     * @brief Visits a block statement and returns a string representation of it.
     *
     * @param stmt The block statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_block_stmt(Stmt::Block* stmt) override;

    /**
     * @brief Visits a conditional statement and returns a string representation of it.
     *
     * @param stmt The conditional statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_conditional_stmt(Stmt::Conditional* stmt) override;

    /**
     * @brief Visits a loop statement and returns a string representation of it.
     *
     * @param stmt The loop statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_loop_stmt(Stmt::Loop* stmt) override;

    /**
     * @brief Visits a return statement and returns a string representation of it.
     *
     * @param stmt The return statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_return_stmt(Stmt::Return* stmt) override;

    /**
     * @brief Visits a break statement and returns a string representation of it.
     *
     * @param stmt The break statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_break_stmt(Stmt::Break* stmt) override;

    /**
     * @brief Visits a continue statement and returns a string representation of it.
     *
     * @param stmt The continue statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_continue_stmt(Stmt::Continue* stmt) override;

    /**
     * @brief Visits a declaration statement and returns a string representation of it.
     *
     * @param stmt The declaration statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_declaration_stmt(Stmt::Declaration* stmt) override;

    /**
     * @brief Visits an end of file statement and returns a string representation of it.
     *
     * @param stmt The end of file statement to visit.
     * @return std::any An any object containing the string representation of the statement.
     */
    std::any visit_eof_stmt(Stmt::EndOfFile* stmt) override;

    /**
     * @brief Visits a get expression and returns a string representation of it.
     *
     * @param expr The get expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_expression_stmt(Stmt::Expression* stmt) override;

    // MARK: Declarations

    /**
     * @brief Visits a variable declaration and returns a string representation of it.
     *
     * @param decl The variable declaration to visit.
     * @return std::any An any object containing the string representation of the declaration.
     */
    std::any visit_var_decl(Decl::Var* decl) override;

    /**
     * @brief Visits a function declaration and returns a string representation of it.
     *
     * @param decl The function declaration to visit.
     * @return std::any An any object containing the string representation of the declaration.
     */
    std::any visit_fun_decl(Decl::Fun* decl) override;

    /**
     * @brief Visits an external function declaration and returns a string representation of it.
     *
     * @param decl The external function declaration to visit.
     * @return std::any An any object containing the string representation of the declaration.
     */
    std::any visit_extern_fun_decl(Decl::ExternFun* decl) override;

    /**
     * @brief Visits a struct declaration and returns a string representation of it.
     *
     * @param decl The struct declaration to visit.
     * @return std::any An any object containing the string representation of the declaration.
     */
    std::any visit_struct_decl(Decl::Struct* decl) override;

    // MARK: Expressions

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
     * @brief Visits a binary expression and returns a string representation of it.
     *
     * @param expr The binary expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_binary_expr(Expr::Binary* expr) override;

    /**
     * @brief Visits a unary expression and returns a string representation of it.
     *
     * @param expr The unary expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_unary_expr(Expr::Unary* expr) override;

    /**
     * @brief Visits a dereference expression and returns a string representation of it.
     *
     * @param expr The dereference expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_dereference_expr(Expr::Dereference* expr) override;

    /**
     * @brief Visits an access expression and returns a string representation of it.
     *
     * @param expr The access expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_access_expr(Expr::Access* expr) override;

    /**
     * @brief Visits an index expression and returns a string representation of it.
     *
     * @param expr The index expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_index_expr(Expr::Index* expr) override;

    /**
     * @brief Visits a call expression and returns a string representation of it.
     *
     * @param expr The call expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_call_expr(Expr::Call* expr) override;

    /**
     * @brief Visits a cast expression and returns a string representation of it.
     *
     * @param expr The cast expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_cast_expr(Expr::Cast* expr) override;

    /**
     * @brief Visits a grouping expression and returns a string representation of it.
     *
     * @param expr The grouping expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_grouping_expr(Expr::Grouping* expr) override;

    /**
     * @brief Visits a variable expression and returns a string representation of it.
     *
     * @param expr The variable expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_identifier_expr(Expr::Identifier* expr) override;

    /**
     * @brief Visits a literal expression and returns a string representation of it.
     *
     * @param expr The literal expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_literal_expr(Expr::Literal* expr) override;

    /**
     * @brief Visits an array expression and returns a string representation of it.
     *
     * @param expr The array expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_array_expr(Expr::Array* expr) override;

    /**
     * @brief Visits a tuple expression and returns a string representation of it.
     *
     * @param expr The tuple expression to visit.
     * @return std::any An any object containing the string representation of the expression.
     */
    std::any visit_tuple_expr(Expr::Tuple* expr) override;
};

#endif // AST_PRINTER_H
