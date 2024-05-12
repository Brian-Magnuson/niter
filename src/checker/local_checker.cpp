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
    // Handle the case where the initializer is not present
    if (decl->initializer == nullptr) {
        // Ensure the type annotation is not auto and the declarer is not const.
        if (decl->type_annotation->to_string() == "auto") {
            ErrorLogger::inst().log_error(decl->name.location, E_AUTO_WITHOUT_INITIALIZER, "Cannot infer type without an initializer.");
            throw LocalTypeException();
        }
        if (decl->declarer == KW_CONST) {
            ErrorLogger::inst().log_error(decl->name.location, E_UNINITIALIZED_CONST, "Cannot declare a constant without an initializer.");
            throw LocalTypeException();
        }
    } else {
        // Get the type of the initializer
        std::shared_ptr<Annotation> init_type = std::any_cast<std::shared_ptr<Annotation>>(decl->initializer->accept(this));

        // If the type annotation is auto, set it to the type of the initializer
        if (decl->type_annotation->to_string() == "auto") {
            decl->type_annotation = init_type;
        } else {
            // Verify that the type of the initializer matches the type annotation
            if (decl->type_annotation->to_string() != init_type->to_string()) {
                ErrorLogger::inst().log_error(decl->name.location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + init_type->to_string() + " to " + decl->type_annotation->to_string() + ".");
                throw LocalTypeException();
            }
        }
    }

    // Verify the type of the variable, do not defer
    bool type_verified = Environment::inst().verify_type(decl->type_annotation);
    if (!type_verified) {
        ErrorLogger::inst().log_error(decl->name.location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
        throw LocalTypeException();
    }

    // Declare the variable
    ErrorCode result = Environment::inst().declare_variable(decl->name.lexeme, decl->declarer, decl->type_annotation);

    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name.location, result, "A symbol with the same name has already been declared in this scope.");
        throw LocalTypeException();
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name.location, result, "An error occurred while declaring the variable.");
        throw LocalTypeException();
    }

    return std::any();
}

std::any LocalChecker::visit_fun_decl(Decl::Fun* decl) {
    // Function declarations are not allowed in local scope
    if (!Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(decl->name.location, E_FUN_IN_LOCAL_SCOPE, "Function declarations are not allowed in local scope.");
        throw LocalTypeException();
    }

    // Declare the function
    ErrorCode result = Environment::inst().declare_variable(decl->name.lexeme, decl->declarer, decl->type_annotation);
    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name.location, result, "A symbol with the same name has already been declared in this scope.");
        throw LocalTypeException();
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name.location, result, "An error occurred while declaring the function.");
        throw LocalTypeException();
    }

    // We could verify the entire function pointer, but then we'd get less specific error messages
    // So we'll just verify the return type and the parameter types separately
    auto fun_type = std::dynamic_pointer_cast<Annotation::Function>(decl->type_annotation);

    std::shared_ptr<Annotation> ret_type = fun_type->ret;
    if (!Environment::inst().verify_type(ret_type)) {
        ErrorLogger::inst().log_error(decl->name.location, E_UNKNOWN_TYPE, "Could not resolve return type annotation.");
        throw LocalTypeException();
    }

    // Increase the local scope
    Environment::inst().increase_local_scope();

    for (unsigned i = 0; i < decl->parameters.size(); i++) {
        // Verify the type of the parameter, do not defer
        bool type_verified = Environment::inst().verify_type(decl->parameters[i]->type_annotation);
        if (!type_verified) {
            ErrorLogger::inst().log_error(decl->name.location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
            throw LocalTypeException();
        }
        // The type annotation won't be `auto` by the way; the parser already checks for that

        // Declare the parameter
        ErrorCode result = Environment::inst().declare_variable(
            decl->parameters[i]->name.lexeme,
            decl->parameters[i]->declarer,
            decl->parameters[i]->type_annotation
        );

        if (result == E_SYMBOL_ALREADY_DECLARED) {
            // We just increased the local scope, so the only way this could happen is if there are multiple parameters with the same name
            ErrorLogger::inst().log_error(decl->parameters[i]->name.location, E_DUPLICATE_PARAM_NAME, "A parameter with the same name has already been declared here.");
            throw LocalTypeException();
        } else if (result != 0) {
            ErrorLogger::inst().log_error(decl->parameters[i]->name.location, result, "An error occurred while declaring the parameter.");
            throw LocalTypeException();
        }
    }

    // Increase the local scope
    Environment::inst().increase_local_scope();

    // Verify the body of the function
    // If any statement returns a non-empty any, either the statement is a return statement or is a block containing one
    // Verify the return statement types and log the appropriate error messages

    bool has_return = false;
    for (auto& stmt : decl->body) {
        std::any stmt_type = stmt->accept(this);
        if (stmt_type.has_value()) {
            has_return = true;
            if (fun_type->ret->to_string() == "void") {
                ErrorLogger::inst().log_error(stmt->location, E_RETURN_IN_VOID_FUN, "Function with return type 'void' cannot return a value.");
                throw LocalTypeException();
            } else {
                std::shared_ptr<Annotation> ret_type = std::any_cast<std::shared_ptr<Annotation>>(stmt_type);
                if (ret_type->to_string() != fun_type->ret->to_string()) {
                    ErrorLogger::inst().log_error(stmt->location, E_RETURN_INCOMPATIBLE, "Cannot convert from " + ret_type->to_string() + " to return type " + fun_type->ret->to_string() + ".");
                    throw LocalTypeException();
                }
            }
        }
    }
    if (!has_return && fun_type->ret->to_string() != "void") {
        ErrorLogger::inst().log_error(decl->name.location, E_NO_RETURN_IN_NON_VOID_FUN, "Function with non-void return type must return a value.");
        throw LocalTypeException();
    }

    // Exit both local scopes
    Environment::inst().exit_scope();
    Environment::inst().exit_scope();

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
            Environment::inst().exit_all_local_scopes();
        } catch (const std::bad_any_cast& e) {
            ErrorLogger::inst().log_error(stmt->location, E_ANY_CAST, "Any cast failed in local type checking.");
            std::cerr << e.what() << std::endl;
        } catch (const std::exception& e) {
            ErrorLogger::inst().log_error(stmt->location, E_UNKNOWN, "An error occurred while type checking the statement.");
            std::cerr << e.what() << std::endl;
        }
    }
}
