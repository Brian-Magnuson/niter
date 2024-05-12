#include "global_checker.h"
#include "../logger/error_code.h"
#include "../logger/logger.h"
#include "environment.h"
#include <iostream>

std::any GlobalChecker::visit_declaration_stmt(Stmt::Declaration* stmt) {
    // Visit the declaration
    return stmt->declaration->accept(this);
}

std::any GlobalChecker::visit_eof_stmt(Stmt::EndOfFile* /* stmt */) {
    // TODO: This doesn't really do anything right now. This becomes more useful when `using` is implemented.
    return std::any();
}

std::any GlobalChecker::visit_var_decl(Decl::Var* decl) {

    // Verify the type of the variable, defer if necessary
    Environment::inst().verify_type(decl->type_annotation, true);
    // Declare the variable
    ErrorCode result = Environment::inst().declare_variable(decl->name.lexeme, decl->type_annotation);
    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name, result, "A symbol with the same name has already been declared in this scope.");
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name, result, "An error occurred while declaring the variable.");
    }

    return std::any();
}

std::any GlobalChecker::visit_fun_decl(Decl::Fun* decl) {
    if (decl->name.lexeme == "main") {
        // The function declarer must be "fun"
        if (decl->declarer != KW_FUN) {
            ErrorLogger::inst().log_error(decl->name, E_INVALID_MAIN_SIGNATURE, "The main function must be declared with the 'fun' keyword.");
        }

        // The function type must be either fun() => i32 or fun(int, char**) => i32
        auto type_annotation = decl->type_annotation->to_string();

        if (type_annotation != "fun() => i32" && type_annotation != "fun(int, char**) => i32") {
            ErrorLogger::inst().log_error(decl->name, E_INVALID_MAIN_SIGNATURE, "The main function must have the signature 'fun() => i32' or 'fun(int, char**) => i32'.");
        }
    }

    // Verify the return type of the function, defer if necessary
    Environment::inst().verify_type(decl->type_annotation, true);

    ErrorCode result = Environment::inst().declare_variable(decl->name.lexeme, decl->type_annotation);

    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name, result, "A symbol with the same name has already been declared in this scope.");
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name, result, "An error occurred while declaring the function.");
    }

    return std::any();
}

std::any GlobalChecker::visit_struct_decl(Decl::Struct* decl) {
    // TODO: Implement
    ErrorLogger::inst().log_error(decl->name, E_UNIMPLEMENTED, "Struct declarations are not yet implemented.");
    return std::any();
}

void GlobalChecker::type_check(std::vector<std::shared_ptr<Stmt>> stmts) {
    for (auto& stmt : stmts) {
        try {
            stmt->accept(this);
        } catch (const GlobalTypeException&) {
            // Do nothing
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
