#include "ast_printer.h"

std::string AstPrinter::print(std::shared_ptr<Expr> expr) {
    return std::any_cast<std::string>(expr->accept(this));
}

std::string AstPrinter::print(std::shared_ptr<Stmt> stmt) {
    return std::any_cast<std::string>(stmt->accept(this));
}

std::string AstPrinter::parenthesize(const std::string& name, const std::vector<std::shared_ptr<Expr>>& exprs) {
    std::string result = "(" + name;
    for (const auto& expr : exprs) {
        result += " ";
        result += expr->accept(this).type().name();
    }
    result += ")";
    return result;
}
