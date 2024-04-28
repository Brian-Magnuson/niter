#include "ast_printer.h"
#include <iomanip>
#include <sstream>

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
        result += std::any_cast<std::string>(expr->accept(this));
    }
    result += ")";
    return result;
}

std::string AstPrinter::double_to_string(double value, int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

std::string AstPrinter::any_to_string(const std::any& value) {
    if (value.type() == typeid(long long)) {
        return std::to_string(std::any_cast<long long>(value));
    }
    if (value.type() == typeid(double)) {
        return double_to_string(std::any_cast<double>(value));
    }
    if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? "true" : "false";
    }
    if (value.type() == typeid(char)) {
        return "'" + std::string(1, std::any_cast<char>(value)) + "'";
    }
    if (value.type() == typeid(std::string)) {
        return "\"" + std::any_cast<std::string>(value) + "\"";
    }
    return std::string("[object]");
}

std::any AstPrinter::visit_print_stmt(Stmt::Print* stmt) {
    std::string result = "(stmt:print";
    result += " " + std::any_cast<std::string>(stmt->value->accept(this));
    result += ")";
    return result;
}

std::any AstPrinter::visit_block_stmt(Stmt::Block* /*stmt*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_conditional_stmt(Stmt::Conditional* /*stmt*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_loop_stmt(Stmt::Loop* /*stmt*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_return_stmt(Stmt::Return* stmt) {
    std::string result = "(stmt:return";
    if (stmt->value != nullptr) {
        result += " " + std::any_cast<std::string>(stmt->value->accept(this));
    }
    result += ")";
    return result;
}

std::any AstPrinter::visit_break_stmt(Stmt::Break* /*stmt*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_continue_stmt(Stmt::Continue* /*stmt*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_declaration_stmt(Stmt::Declaration* stmt) {
    return stmt->declaration->accept(this);
}

std::any AstPrinter::visit_eof_stmt(Stmt::EndOfFile* /*stmt*/) {
    return std::string("(stmt:eof)");
}

std::any AstPrinter::visit_expression_stmt(Stmt::Expression* stmt) {
    return stmt->expression->accept(this);
}

std::any AstPrinter::visit_var_decl(Decl::Var* decl) {
    std::string result = "(decl:";
    if (decl->declarer == KW_VAR) {
        result += "var";
    } else if (decl->declarer == KW_CONST) {
        result += "const";
    } else {
        result += "unknown";
    }
    result += " ";
    result += decl->name.lexeme;
    if (decl->type_annotation != nullptr) {
        result += " " + decl->type_annotation->token.lexeme;
    }
    if (decl->initializer != nullptr) {
        result += " " + std::any_cast<std::string>(decl->initializer->accept(this));
    }
    result += ")";
    return result;
}

std::any AstPrinter::visit_fun_decl(Decl::Fun* decl) {
    std::string result = "(decl:fun ";
    result += decl->name.lexeme;
    result += " ";
    result += decl->return_type->token.lexeme;
    result += " ";
    for (const auto& param : decl->parameters) {
        result += std::any_cast<std::string>(param->accept(this));
        result += " ";
    }
    result += "{ ";
    for (const auto& stmt : decl->body) {
        result += std::any_cast<std::string>(stmt->accept(this));
        result += " ";
    }
    result += "})";
    return result;
    /*
    Example:
    fun add(a: int, b: int): int {
        return a + b;
    }
    ->
    (decl:fun add int (decl:var a int nil) (decl:var b int nil) { (stmt:return (expr:+ (expr:identifier a) (expr:identifier b))) })
    */
}

std::any AstPrinter::visit_struct_decl(Decl::Struct* /*decl*/) {
    // TODO: Implement this
    return std::any();
}

std::any AstPrinter::visit_assign_expr(Expr::Assign* expr) {
    return parenthesize("=", {expr->left, expr->right});
}

std::any AstPrinter::visit_logical_expr(Expr::Logical* expr) {
    return parenthesize(expr->op.lexeme, {expr->left, expr->right});
}

std::any AstPrinter::visit_binary_expr(Expr::Binary* expr) {
    return parenthesize(expr->op.lexeme, {expr->left, expr->right});
}

std::any AstPrinter::visit_unary_expr(Expr::Unary* expr) {
    return parenthesize(expr->op.lexeme, {expr->right});
}

std::any AstPrinter::visit_call_expr(Expr::Call* expr) {
    std::vector<std::shared_ptr<Expr>> args;
    args.push_back(expr->callee);
    for (const auto& arg : expr->arguments) {
        args.push_back(arg);
    }
    return parenthesize("call", args);
}

std::any AstPrinter::visit_access_expr(Expr::Access* expr) {
    if (expr->op.tok_type == TOK_LEFT_SQUARE) {
        return parenthesize("[]", {expr->left, expr->right});
    } else {
        return parenthesize(expr->op.lexeme, {expr->left, expr->right});
    }
}

std::any AstPrinter::visit_grouping_expr(Expr::Grouping* expr) {
    return parenthesize("group", {expr->expression});
}

std::any AstPrinter::visit_identifier_expr(Expr::Identifier* expr) {
    auto current = expr;
    std::string name = current->token.lexeme;
    current = current->contained.get();
    while (current != nullptr) {
        name += "::" + current->token.lexeme;
        current = current->contained.get();
    }
    return name;
}

std::any AstPrinter::visit_literal_expr(Expr::Literal* expr) {
    if (expr->token.literal.has_value()) {
        return any_to_string(expr->token.literal);
    } else {
        return std::string("nil");
    }
}

std::any AstPrinter::visit_array_expr(Expr::Array* expr) {
    std::vector<std::shared_ptr<Expr>> elements;
    for (const auto& element : expr->elements) {
        elements.push_back(element);
    }
    return parenthesize("array", elements);
}

std::any AstPrinter::visit_tuple_expr(Expr::Tuple* expr) {
    std::vector<std::shared_ptr<Expr>> elements;
    for (const auto& element : expr->elements) {
        elements.push_back(element);
    }
    return parenthesize("tuple", elements);
}
