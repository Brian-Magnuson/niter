#include "global_checker.h"
#include "../logger/error_code.h"
#include "../logger/logger.h"

std::any GlobalChecker::visit_declaration_stmt(Stmt::Declaration* stmt) {
    // Visit the declaration
    return stmt->declaration->accept(this);
}

std::any GlobalChecker::visit_eof_stmt(Stmt::EndOfFile* /* stmt */) {
    // TODO: This doesn't really do anything right now. This becomes more useful when `using` is implemented.
    return std::any();
}

std::any GlobalChecker::visit_var_decl(Decl::Var* decl) {
    // TODO: Implement
    ErrorLogger::inst().log_error(decl->name, E_UNIMPLEMENTED, "Variable declarations are not yet implemented.");
    return std::any();
}

std::any GlobalChecker::visit_fun_decl(Decl::Fun* /* decl */) {
    // TODO: Implement
    return std::any();
}

std::any GlobalChecker::visit_struct_decl(Decl::Struct* decl) {
    // TODO: Implement
    ErrorLogger::inst().log_error(decl->name, E_UNIMPLEMENTED, "Struct declarations are not yet implemented.");
    return std::any();
}
