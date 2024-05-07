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

std::any GlobalChecker::visit_fun_decl(Decl::Fun* decl) {
    if (decl->name.lexeme == "main") {
        // The function declarer must be "fun"
        if (decl->declarer != KW_FUN) {
            ErrorLogger::inst().log_error(decl->name, E_INVALID_MAIN_SIGNATURE, "The main function must be declared with the 'fun' keyword.");
        }
        // The return type must be i32
        if (decl->type_annotation->to_string() != "i32") {
            ErrorLogger::inst().log_error(decl->name, E_INVALID_MAIN_SIGNATURE, "The main function must indicate a return type of i32.");
        }
        // The function cannot have any parameters (unimplemented)
        if (decl->parameters.size() > 0) {
            ErrorLogger::inst().log_error(decl->parameters.front()->name, E_UNIMPLEMENTED, "The main function cannot have any parameters.");
        }
    } else {
        // TODO: Implement
        ErrorLogger::inst().log_error(decl->name, E_UNIMPLEMENTED, "Function declarations are not yet implemented.");
    }
    return std::any();
}

std::any GlobalChecker::visit_struct_decl(Decl::Struct* decl) {
    // TODO: Implement
    ErrorLogger::inst().log_error(decl->name, E_UNIMPLEMENTED, "Struct declarations are not yet implemented.");
    return std::any();
}
