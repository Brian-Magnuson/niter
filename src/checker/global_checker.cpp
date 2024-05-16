#include "global_checker.h"
#include "../logger/error_code.h"
#include "../logger/logger.h"
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

    // Declare the variable, defer if necessary
    auto [_, result] = Environment::inst().declare_variable(decl->name.lexeme, decl->declarer, decl->type_annotation, true);

    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name.location, result, "A symbol with the same name has already been declared in this scope.");
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name.location, E_IMPOSSIBLE, "Function `declare_variable` issued error " + std::to_string(result) + " in global type checking.");
    }
    // E_UNKNOWN_TYPE is not handled here since variables with unknown types are deferred here.

    return std::any();
}

std::any GlobalChecker::visit_fun_decl(Decl::Fun* decl) {

    // Declare the function, defer if necessary
    auto [variable, result] = Environment::inst().declare_variable(decl->name.lexeme, decl->declarer, decl->type_annotation, true);

    if (decl->name.lexeme == "main") {
        // The function declarer must be "fun"
        if (decl->declarer != KW_FUN) {
            ErrorLogger::inst().log_error(decl->name.location, E_INVALID_MAIN_SIGNATURE, "The main function must be declared with the 'fun' keyword.");
        }

        // The function type must be either fun() => i32 or fun(int, char**) => i32
        auto type_annotation = decl->type_annotation->to_string();
        auto type_string = variable != nullptr ? variable->type->to_string() : "";

        if (type_annotation != "fun() => ::i32" && type_annotation != "fun(::int, ::char**) => ::i32") {
            ErrorLogger::inst().log_error(decl->name.location, E_INVALID_MAIN_SIGNATURE, "The main function must have the signature 'fun() => i32' or 'fun(int, char**) => i32'.");
        }
    }

    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name.location, result, "A symbol with the same name has already been declared in this scope.");
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name.location, result, "An error occurred while declaring the function.");
    }

    return std::any();
}

std::any GlobalChecker::visit_struct_decl(Decl::Struct* decl) {
    // TODO: Implement
    ErrorLogger::inst().log_error(decl->name.location, E_UNIMPLEMENTED, "Struct declarations are not yet implemented.");
    return std::any();
}

void GlobalChecker::type_check(std::vector<std::shared_ptr<Stmt>> stmts) {
    for (auto& stmt : stmts) {
        try {
            stmt->accept(this);
        } catch (const GlobalTypeException&) {
            // Do nothing
        } catch (const std::bad_any_cast& e) {
            ErrorLogger::inst().log_error(stmt->location, E_ANY_CAST, "Any cast failed in global type checking.");
            std::cerr << e.what() << std::endl;
        } catch (const std::exception& e) {
            ErrorLogger::inst().log_error(stmt->location, E_UNKNOWN, "An error occurred while type checking the statement.");
            std::cerr << e.what() << std::endl;
        }
    }
}
