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
    std::string print(std::shared_ptr<Expr> expr);
    std::string print(std::shared_ptr<Stmt> stmt);
    std::string parenthesize(const std::string& name, const std::vector<std::shared_ptr<Expr>>& exprs);

    std::any visit_binary_expr(Expr::Binary* expr) override;
    std::any visit_grouping_expr(Expr::Grouping* expr) override;
    std::any visit_literal_expr(Expr::Literal* expr) override;
    std::any visit_unary_expr(Expr::Unary* expr) override;
    std::any visit_variable_expr(Expr::Variable* expr) override;
    std::any visit_assign_expr(Expr::Assign* expr) override;
    std::any visit_logical_expr(Expr::Logical* expr) override;
    std::any visit_call_expr(Expr::Call* expr) override;

    std::any visit_expression_stmt(Stmt::Expression* stmt) override;
    std::any visit_print_stmt(Stmt::Print* stmt) override;
    // std::any visit_var_stmt(Stmt::Var* stmt) override;
    // std::any visit_block_stmt(Stmt::Block* stmt) override;
    // std::any visit_if_stmt(Stmt::If* stmt) override;
    // std::any visit_while_stmt(Stmt::While* stmt) override;
    // std::any visit_for_stmt(Stmt::For* stmt) override;
    // std::any visit_return_stmt(Stmt::Return* stmt) override;
    // std::any visit_break_stmt(Stmt::Break* stmt) override;
    // std::any visit_continue_stmt(Stmt::Continue* stmt) override;
};

#endif // AST_PRINTER_H
