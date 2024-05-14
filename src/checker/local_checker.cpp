#include "local_checker.h"
#include "../logger/logger.h"
#include "../utility/utils.h"
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

        // Verify that the type of the initializer matches the type annotation
        if (!decl->type_annotation->is_compatible_with(init_type)) {
            ErrorLogger::inst().log_error(decl->name.location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + init_type->to_string() + " to " + decl->type_annotation->to_string() + ".");
            throw LocalTypeException();
        }
    }

    // Verify the type of the variable, do not defer
    // Note that `auto` not installed as a primitive type. If any part of the type is `auto`, verification will fail.
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
                if (!fun_type->ret->is_compatible_with(ret_type)) {
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
    // There are four unary operators: `!`, `-`, `*`, and `&`

    std::shared_ptr<Annotation> operand_type = std::any_cast<std::shared_ptr<Annotation>>(expr->right->accept(this));

    if (expr->op.tok_type == TOK_BANG) {
        // The operand must be of type `bool`
        if (operand_type->to_string() != "bool") {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply unary operator '!' to type " + operand_type->to_string() + ". Expected type 'bool'.");
            throw LocalTypeException();
        }
        expr->type_annotation = operand_type;
    } else if (expr->op.tok_type == TOK_MINUS) {
        // The operand must be of type `int` or `float`
        if (operand_type->is_int() || operand_type->is_float()) {
            expr->type_annotation = operand_type;
        } else {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply unary operator '-' to type " + operand_type->to_string() + ". Expected type 'int' or 'float'.");
            throw LocalTypeException();
        }
    } else if (expr->op.tok_type == TOK_STAR) {
        // The operand must be a pointer type
        auto operand_ptr_type = std::dynamic_pointer_cast<Annotation::Pointer>(operand_type);
        if (operand_ptr_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot dereference non-pointer type " + operand_type->to_string() + ".");
            throw LocalTypeException();
        }
        // The type of the expression is the type of the pointer
        expr->type_annotation = operand_ptr_type->name;
    } else if (expr->op.tok_type == TOK_AMP) {
        // Only Expr::Access and Expr::Identifier are allowed as operands
        auto operand_access = std::dynamic_pointer_cast<Expr::Access>(expr->right);
        auto operand_ident = std::dynamic_pointer_cast<Expr::Identifier>(expr->right);
        if (operand_access == nullptr && operand_ident == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INVALID_ADDRESS_OF, "Cannot take the address of a non-lvalue.");
            throw LocalTypeException();
        }
        // The type of the expression is a pointer to the type of the operand
        expr->type_annotation = std::make_shared<Annotation::Pointer>(operand_type);

    } else {
        // Unreachable
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Unknown unary operator.");
    }

    return expr->type_annotation;
}

std::any LocalChecker::visit_call_expr(Expr::Call* expr) {
    // The left side of the call expression must be callable; i.e. a function pointer type
    std::shared_ptr<Annotation> left_type = std::any_cast<std::shared_ptr<Annotation>>(expr->callee->accept(this));
    if (!IS_TYPE(left_type, Annotation::Function)) {
        ErrorLogger::inst().log_error(expr->location, E_CALL_ON_NON_FUN, "Expression is not callable.");
        throw LocalTypeException();
    }
    auto fun_type = std::dynamic_pointer_cast<Annotation::Function>(left_type);

    // First, the number of arguments must match the number of parameters
    if (expr->arguments.size() != fun_type->params.size()) {
        ErrorLogger::inst().log_error(expr->location, E_INVALID_ARITY, "Expected " + std::to_string(fun_type->params.size()) + " arguments, found " + std::to_string(expr->arguments.size()) + ".");
        throw LocalTypeException();
    }
    // Then, the types of the arguments must match the types of the parameters
    for (unsigned i = 0; i < expr->arguments.size(); i++) {
        std::shared_ptr<Annotation> arg_type = std::any_cast<std::shared_ptr<Annotation>>(expr->arguments[i]->accept(this));
        if (!fun_type->params[i].second->is_compatible_with(arg_type)) {
            ErrorLogger::inst().log_error(expr->arguments[i]->location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + arg_type->to_string() + " to " + fun_type->params[i].second->to_string() + ".");
            throw LocalTypeException();
        }
    }

    // The type of the call expression is the return type of the function
    expr->type_annotation = fun_type->ret;
    return expr->type_annotation;
}

std::any LocalChecker::visit_access_expr(Expr::Access* expr) {
    auto left_type = std::any_cast<std::shared_ptr<Annotation>>(expr->left->accept(this));

    if (expr->op.tok_type == TOK_DOT) {
        std::shared_ptr<Expr::Identifier> right = std::dynamic_pointer_cast<Expr::Identifier>(expr->right);

        if (right == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_NO_IDENT_AFTER_DOT, "Expected identifier after '.'.");
            throw LocalTypeException();
        }
        auto left_seg_type = std::dynamic_pointer_cast<Annotation::Segmented>(left_type);
        if (left_seg_type == nullptr) {
            if (IS_TYPE(left_type, Annotation::Pointer)) {
                // This is still an error, but we can give a more specific error message
                ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type. Did you mean to use '->' instead of '.'?");
                throw LocalTypeException();
            }
            ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
            throw LocalTypeException();
        }

        expr->type_annotation = Environment::inst().get_instance_member_type(left_seg_type, right->to_string());
        // If this returns nullptr, the member was not found
        if (expr->type_annotation == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INVALID_STRUCT_MEMBER, "Struct type " + left_seg_type->to_string() + " does not have member " + right->to_string() + ".");
            throw LocalTypeException();
        }

        return expr->type_annotation;
    } else if (expr->op.tok_type == TOK_ARROW) {
        // Pretty much the same as the dot operator, but the left side is dereferenced first
        auto left_ptr_type = std::dynamic_pointer_cast<Annotation::Pointer>(left_type);
        if (left_ptr_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
            throw LocalTypeException();
        }
        auto left_seg_type = std::dynamic_pointer_cast<Annotation::Segmented>(left_ptr_type->name);
        if (left_seg_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
            throw LocalTypeException();
        }

        std::shared_ptr<Expr::Identifier> right = std::dynamic_pointer_cast<Expr::Identifier>(expr->right);
        if (right == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_NO_IDENT_AFTER_DOT, "Expected identifier after '->'.");
            throw LocalTypeException();
        }

        expr->type_annotation = Environment::inst().get_instance_member_type(left_seg_type, right->to_string());
        // If this returns nullptr, the member was not found
        if (expr->type_annotation == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INVALID_STRUCT_MEMBER, "Struct type " + left_seg_type->to_string() + " does not have member " + right->to_string() + ".");
            throw LocalTypeException();
        }

        return expr->type_annotation;
    } else if (expr->op.tok_type == TOK_LEFT_SQUARE) {
        // Right now, this only works with arrays
        auto left_arr_type = std::dynamic_pointer_cast<Annotation::Array>(left_type);
        if (left_arr_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INDEX_ON_NON_ARRAY, "Cannot index non-array type.");
            throw LocalTypeException();
        }

        // The index must be an integer
        auto index_type = std::any_cast<std::shared_ptr<Annotation>>(expr->right->accept(this));
        if (index_type->to_string() != "i32") {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot index array with type " + index_type->to_string() + ". Expected type 'i32'.");
            throw LocalTypeException();
        }

        // The type of the expression is the type of the array elements
        expr->type_annotation = left_arr_type->name;
        return expr->type_annotation;
    } else {
        // Unreachable
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Unknown access operator.");
        throw LocalTypeException();
    }
}

std::any LocalChecker::visit_grouping_expr(Expr::Grouping* expr) {
    expr->type_annotation = std::any_cast<std::shared_ptr<Annotation>>(expr->expression->accept(this));
    return expr->type_annotation;
}

std::any LocalChecker::visit_identifier_expr(Expr::Identifier* expr) {
    expr->type_annotation = Environment::inst().get_type(expr);
    if (expr->type_annotation == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
        throw LocalTypeException();
    }
    return expr->type_annotation;
}

std::any LocalChecker::visit_literal_expr(Expr::Literal* expr) {

    std::shared_ptr<Annotation> type = nullptr;

    switch (expr->token.tok_type) {
    case TOK_INT:
        type = std::make_shared<Annotation::Segmented>("i32");
        break;
    case TOK_FLOAT:
        type = std::make_shared<Annotation::Segmented>("f64");
        break;
    case TOK_CHAR:
        type = std::make_shared<Annotation::Segmented>("char");
        break;
    case TOK_STR:
        type = std::make_shared<Annotation::Segmented>("char");
        type = std::make_shared<Annotation::Pointer>(type);
        // Creates type `char*`
        break;
    case TOK_BOOL:
        type = std::make_shared<Annotation::Segmented>("bool");
        break;
    case TOK_NIL:
        type = std::make_shared<Annotation::Segmented>("auto");
        type = std::make_shared<Annotation::Pointer>(type);
        // Creates type `auto*`
        // Note: `auto` is not a primitive type, so this is not a valid type.
        // However, when later checked for type compatibility, the type will be updated to the correct type.
        break;
    default:
        ErrorLogger::inst().log_error(expr->location, E_UNRECOGNIZED_LITERAL, "Unknown literal type.");
        throw LocalTypeException();
    }

    expr->type_annotation = type;
    return type;
}

std::any LocalChecker::visit_array_expr(Expr::Array* expr) {
    // Handle the case where the array is empty
    if (expr->elements.empty()) {
        expr->type_annotation = std::make_shared<Annotation::Segmented>("auto");
        expr->type_annotation = std::make_shared<Annotation::Array>(expr->type_annotation);
        return expr->type_annotation;
    } else {
        // Ensure all elements have the same type
        std::shared_ptr<Annotation> type = std::any_cast<std::shared_ptr<Annotation>>(expr->elements[0]->accept(this));
        // In the case that this element is an `auto` type, the repeated compatibility checks in the following loop will build the type until it is complete.
        for (auto& elem : expr->elements) {
            std::shared_ptr<Annotation> elem_type = std::any_cast<std::shared_ptr<Annotation>>(elem->accept(this));
            if (!type->is_compatible_with(elem_type)) {
                ErrorLogger::inst().log_error(elem->location, E_INCONSISTENT_ARRAY_TYPES, "Array elements must have the same type. Expected " + type->to_string() + ", found " + elem_type->to_string() + ".");
                throw LocalTypeException();
            }
        }
        // If, after this loop, the type still contains `auto`, it will later be checked against the type annotation on the left-hand side of the assignment
        type = std::make_shared<Annotation::Array>(type);
        expr->type_annotation = type;
        return type;
    }
}

std::any LocalChecker::visit_tuple_expr(Expr::Tuple* expr) {
    std::vector<std::shared_ptr<Annotation>> types;
    for (auto& elem : expr->elements) {
        std::shared_ptr<Annotation> elem_type = std::any_cast<std::shared_ptr<Annotation>>(elem->accept(this));
        types.push_back(elem_type);
    }
    expr->type_annotation = std::make_shared<Annotation::Tuple>(types);
    return expr->type_annotation;
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
