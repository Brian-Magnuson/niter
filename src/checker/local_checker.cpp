#include "local_checker.h"
#include "../logger/logger.h"
#include <iostream>

std::any LocalChecker::visit_declaration_stmt(Stmt::Declaration* stmt) {
    // Visit the declaration
    return stmt->declaration->accept(this);
    return std::any();
}

std::any LocalChecker::visit_expression_stmt(Stmt::Expression* stmt) {
    // Expression statements are not allowed in global scope
    if (Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(stmt->location, E_GLOBAL_EXPRESSION, "Expression statements are not allowed in global scope.");
        throw LocalTypeException();
    }
    return std::any();
}
std::any LocalChecker::visit_block_stmt(Stmt::Block* stmt) {
    // Not yet implemented
    // TODO: Implement block statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Block statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_conditional_stmt(Stmt::Conditional* stmt) {
    // Log error with location
    // TODO: Implement conditional statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Conditional statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_loop_stmt(Stmt::Loop* stmt) {
    // Log error with location
    // TODO: Implement loop statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Loop statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_return_stmt(Stmt::Return* stmt) {
    // Return statements are not allowed in global scope
    if (Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(stmt->location, E_GLOBAL_RETURN, "Return statements are not allowed in global scope.");
        throw LocalTypeException();
    }

    std::shared_ptr<Annotation> ret_type = std::any_cast<std::shared_ptr<Annotation>>(stmt->value->accept(this));

    return ret_type;
}

std::any LocalChecker::visit_break_stmt(Stmt::Break* stmt) {
    // Log error with location
    // TODO: Implement break statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Break statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_continue_stmt(Stmt::Continue* stmt) {
    // Log error with location
    // TODO: Implement continue statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Continue statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_print_stmt(Stmt::Print* stmt) {
    // Currently, print statements are only allowed to print objects of type `char*`
    std::shared_ptr<Annotation> print_type = std::any_cast<std::shared_ptr<Annotation>>(stmt->value->accept(this));
    if (print_type->to_string() != "char*") {
        ErrorLogger::inst().log_error(stmt->location, E_PUTS_WITHOUT_STRING, "Print statements are only allowed to print objects of type `char*`.");
        throw LocalTypeException();
    }
    return std::any();
}

std::any LocalChecker::visit_eof_stmt(Stmt::EndOfFile* stmt) {
    // Log error with location
    // TODO: Implement end of file statements
    ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "End of file statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_var_decl(Decl::Var* decl) {
    // Log error with location
    // TODO: Implement variable declarations
    ErrorLogger::inst().log_error(decl->location, E_UNIMPLEMENTED, "Variable declarations are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_fun_decl(Decl::Fun* decl) {
    // Log error with location
    // TODO: Implement function declarations
    ErrorLogger::inst().log_error(decl->location, E_UNIMPLEMENTED, "Function declarations are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_assign_expr(Expr::Assign* expr) {
    // Log error with location
    // TODO: Implement assignment expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Assignment expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_logical_expr(Expr::Logical* expr) {
    // Log error with location
    // TODO: Implement logical expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Logical expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_binary_expr(Expr::Binary* expr) {
    // Log error with location
    // TODO: Implement binary expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Binary expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_unary_expr(Expr::Unary* expr) {
    // Log error with location
    // TODO: Implement unary expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Unary expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_call_expr(Expr::Call* expr) {
    // Log error with location
    // TODO: Implement call expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Call expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_access_expr(Expr::Access* expr) {
    // Log error with location
    // TODO: Implement access expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Access expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_grouping_expr(Expr::Grouping* expr) {
    // Log error with location
    // TODO: Implement grouping expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Grouping expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_identifier_expr(Expr::Identifier* expr) {
    // Log error with location
    // TODO: Implement identifier expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Identifier expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_literal_expr(Expr::Literal* expr) {
    // Log error with location
    // TODO: Implement literal expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Literal expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_array_expr(Expr::Array* expr) {
    // Log error with location
    // TODO: Implement array expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Array expressions are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_tuple_expr(Expr::Tuple* expr) {
    // Log error with location
    // TODO: Implement tuple expressions
    ErrorLogger::inst().log_error(expr->location, E_UNIMPLEMENTED, "Tuple expressions are not yet implemented.");
    return std::any();
}

void LocalChecker::type_check(std::vector<std::shared_ptr<Stmt>> stmts) {
    for (auto stmt : stmts) {
        try {
            stmt->accept(this);
        } catch (const LocalTypeException&) {
            // Do nothing
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
