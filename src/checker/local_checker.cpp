#include "local_checker.h"
#include "../logger/logger.h"
#include <iostream>

std::any LocalChecker::visit_declaration_stmt(Stmt::Declaration* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_expression_stmt(Stmt::Expression* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_block_stmt(Stmt::Block* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_conditional_stmt(Stmt::Conditional* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_loop_stmt(Stmt::Loop* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_return_stmt(Stmt::Return* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_break_stmt(Stmt::Break* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_continue_stmt(Stmt::Continue* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_print_stmt(Stmt::Print* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_eof_stmt(Stmt::EndOfFile* /* stmt */) {
    // TODO: Implement this
    return std::any();
}

// var_decl, fun_decl
std::any LocalChecker::visit_var_decl(Decl::Var* /* decl */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_fun_decl(Decl::Fun* /* decl */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_assign_expr(Expr::Assign* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_logical_expr(Expr::Logical* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_binary_expr(Expr::Binary* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_unary_expr(Expr::Unary* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_call_expr(Expr::Call* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_access_expr(Expr::Access* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_identifier_expr(Expr::Identifier* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any LocalChecker::visit_literal_expr(Expr::Literal* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any visit_array_expr(Expr::Array* /* expr */) {
    // TODO: Implement this
    return std::any();
}

std::any visit_tuple_expr(Expr::Tuple* /* expr */) {
    // TODO: Implement this
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
