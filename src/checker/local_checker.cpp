#include "local_checker.h"
#include "../logger/logger.h"
#include "../scanner/token.h"
#include "../utility/utils.h"
#include "type.h"
#include <iostream>
#include <tuple>

bool LocalChecker::check_token(TokenType token, const std::vector<TokenType>& types) const {
    for (auto type : types) {
        if (token == type) {
            return true;
        }
    }
    return false;
}

// MARK: Statements

std::any LocalChecker::visit_declaration_stmt(Stmt::Declaration* stmt) {
    // Visit the declaration
    return stmt->declaration->accept(this);
    return std::any();
}

std::any LocalChecker::visit_expression_stmt(Stmt::Expression* stmt) {
    // This is a local expression statement. Any global expression statements should have been caught by the global checker.
    // Visit the expression.
    stmt->expression->accept(this);
    // Expression statements do not produce any value.
    return std::any();
}
std::any LocalChecker::visit_block_stmt(Stmt::Block* /* stmt */) {
    // Not yet implemented
    // TODO: Implement block statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Block statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_conditional_stmt(Stmt::Conditional* /* stmt */) {
    // Log error with location
    // TODO: Implement conditional statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Conditional statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_loop_stmt(Stmt::Loop* /* stmt */) {
    // Log error with location
    // TODO: Implement loop statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Loop statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_return_stmt(Stmt::Return* stmt) {
    // Return statements are not allowed in global scope
    // We already checked for this in the global checker

    if (stmt->value == nullptr) {
        return std::any();
    }

    std::shared_ptr<Type> ret_type = std::any_cast<std::shared_ptr<Type>>(stmt->value->accept(this));

    return ret_type;
}

std::any LocalChecker::visit_break_stmt(Stmt::Break* /* stmt */) {
    // Log error with location
    // TODO: Implement break statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Break statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_continue_stmt(Stmt::Continue* /* stmt */) {
    // Log error with location
    // TODO: Implement continue statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Continue statements are not yet implemented.");
    return std::any();
}

std::any LocalChecker::visit_eof_stmt(Stmt::EndOfFile* /* stmt */) {
    // Does nothing (for now)
    return std::any();
}

// MARK: Declarations

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
    }

    // Get the type of the initializer
    std::shared_ptr<Type> init_type = std::make_shared<Type::Blank>();
    if (decl->initializer != nullptr) {
        init_type = std::any_cast<std::shared_ptr<Type>>(decl->initializer->accept(this));
    }

    std::shared_ptr<Node::Locatable> node;
    ErrorCode result = (ErrorCode)0;

    // If we are in global scope, the variable is already declared
    if (Environment::inst().in_global_scope()) {
        node = Environment::inst().get_variable({decl->name.lexeme});
    } else {
        // Declare the variable, do not defer
        std::tie(node, result) = Environment::inst().declare_variable(decl->location, decl->name.lexeme, decl->declarer, decl->type_annotation);
    }

    // Verify that the variable was declared successfully
    if (result == E_SYMBOL_ALREADY_DECLARED) {
        ErrorLogger::inst().log_error(decl->name.location, E_LOCAL_ALREADY_DECLARED, "A symbol with the same name has already been declared in this scope.");
        ErrorLogger::inst().log_note(node->location, "Previous declaration was here.");
        throw LocalTypeException();
    } else if (result == E_UNKNOWN_TYPE) {
        ErrorLogger::inst().log_error(decl->name.location, result, "Could not resolve type annotation");
        throw LocalTypeException();
    } else if (result != 0) {
        ErrorLogger::inst().log_error(decl->name.location, E_IMPOSSIBLE, "Function `declare_variable` issued error " + std::to_string(result) + " in LocalChecker::visit_var_decl.");
        throw LocalTypeException();
    }
    // Neither variable->type nor init_type should be nullptr at this point.
    auto variable = std::dynamic_pointer_cast<Node::Variable>(node);

    // Verify that the type of the initializer matches the type annotation
    if (!Type::are_compatible(init_type, variable->type)) {
        ErrorLogger::inst().log_error(decl->name.location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + init_type->to_string() + " to " + variable->type->to_string() + ".");
        throw LocalTypeException();
    }
    // Verify that none of the types are blank
    if (variable->type->kind() == TypeKind::BLANK || init_type->kind() == TypeKind::BLANK) {
        ErrorLogger::inst().log_error(decl->name.location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
        throw LocalTypeException();
    }
    // Verify that the right side is not a const pointer being assigned to a non-const pointer
    auto init_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(init_type);
    if (init_ptr_type != nullptr && init_ptr_type->declarer == KW_CONST && variable->declarer != KW_CONST) {
        ErrorLogger::inst().log_error(decl->name.location, E_INVALID_PTR_DECLARER, "Cannot assign a const pointer to a non-const pointer.");
        throw LocalTypeException();
    } else if (init_ptr_type != nullptr && variable->declarer == KW_CONST) {
        // If the variable is const, the pointer must be const as well
        init_ptr_type->declarer = KW_CONST;
    }

    return std::any();
}

std::any LocalChecker::visit_fun_decl(Decl::Fun* decl) {
    // Function declarations are not allowed in local scope
    if (!Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(decl->name.location, E_FUN_IN_LOCAL_SCOPE, "Function declarations are not allowed in local scope.");
        throw LocalTypeException();
    }

    // No need to declare the function; we've already done that in the global checker
    auto variable = Environment::inst().get_variable({decl->name.lexeme});

    // We could verify the entire function pointer, but then we'd get less specific error messages
    // So we'll just verify the return type and the parameter types separately
    auto fun_annotation = std::dynamic_pointer_cast<Annotation::Function>(decl->type_annotation);

    // Increase the local scope for the parameters and return variable
    Environment::inst().increase_local_scope();

    // We don't need to declare the return variable; it's not designed to be accessed directly
    // The return variable is more important during the code generation phase
    // We can instead just verify that the return type is correct

    for (unsigned i = 0; i < decl->parameters.size(); i++) {
        auto [param_node, param_result] = Environment::inst().declare_variable(
            decl->parameters[i]->location,
            decl->parameters[i]->name.lexeme,
            decl->parameters[i]->declarer,
            decl->parameters[i]->type_annotation
        );
        if (param_result == E_SYMBOL_ALREADY_DECLARED) {
            ErrorLogger::inst().log_error(decl->parameters[i]->name.location, E_DUPLICATE_PARAM_NAME, "A parameter with the same name has already been declared here.");
            ErrorLogger::inst().log_note(param_node->location, "Previous declaration was here.");
            throw LocalTypeException();
        } else if (param_result == E_UNKNOWN_TYPE) {
            ErrorLogger::inst().log_error(decl->parameters[i]->name.location, param_result, "Could not resolve type annotation.");
            throw LocalTypeException();
        } else if (param_result != 0) {
            ErrorLogger::inst().log_error(decl->parameters[i]->name.location, E_IMPOSSIBLE, "Function `declare_variable` issued error " + std::to_string(param_result) + " in LocalChecker::visit_fun_decl.");
            throw LocalTypeException();
        }
        auto param_var = std::dynamic_pointer_cast<Node::Variable>(param_node);
        // Params do not have type `auto`; the parser already checks for this
        // If the parameter is a pointer, make the declarer in the pointer type match the declarer of the parameter
        auto param_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(param_var->type);
        if (param_ptr_type != nullptr) {
            param_ptr_type->declarer = param_var->declarer;
        }

        // We don't worry about initializers for parameters; this is just a type checker
    }

    // At this point, the function type should be fully resolved
    if (variable == nullptr) {
        ErrorLogger::inst().log_error(decl->name.location, E_IMPOSSIBLE, "Function pointer variable is nullptr in LocalChecker::visit_fun_decl.");
        throw LocalTypeException();
    }
    auto variable_fun_type = std::dynamic_pointer_cast<Type::Function>(variable->type);

    // Increase the local scope for the function body
    Environment::inst().increase_local_scope();

    // Verify the body of the function
    // If any statement returns a non-empty any, either the statement is a return statement or is a block containing one
    // Verify the return statement types and log the appropriate error messages
    bool has_return = false;
    for (auto& stmt : decl->body) {
        std::any stmt_type = stmt->accept(this);
        if (stmt_type.has_value()) {
            has_return = true;
            if (variable->type->to_string() == "::void") {
                ErrorLogger::inst().log_error(stmt->location, E_RETURN_IN_VOID_FUN, "Function with return type 'void' cannot return a value.");
                throw LocalTypeException();
            } else {
                auto ret_type = std::any_cast<std::shared_ptr<Type>>(stmt_type);

                // The return type of the function must match the return type of the return statement
                // This will also catch the case where the function return type is `void` and the return statement has a value
                if (!Type::are_compatible(ret_type, variable_fun_type->return_type)) {
                    ErrorLogger::inst().log_error(stmt->location, E_RETURN_INCOMPATIBLE, "Cannot convert from " + ret_type->to_string() + " to return type " + fun_annotation->return_annotation->to_string() + ".");
                    throw LocalTypeException();
                }
            }
        }
    }
    if (!has_return && variable_fun_type->return_type->to_string() != "::void") {
        ErrorLogger::inst().log_error(decl->name.location, E_NO_RETURN_IN_NON_VOID_FUN, "Function with non-void return type must return a value.");
        throw LocalTypeException();
    }

    // Exit both local scopes
    Environment::inst().exit_scope();
    Environment::inst().exit_scope();

    return std::any();
}

std::any LocalChecker::visit_extern_fun_decl(Decl::ExternFun* decl) {
    // Function declarations are not allowed in local scope
    if (!Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(decl->name.location, E_FUN_IN_LOCAL_SCOPE, "Function declarations are not allowed in local scope.");
        throw LocalTypeException();
    }

    // No need to do further checks; we've already done that in the global checker
    return std::any();
}

// MARK: Expressions

std::any LocalChecker::visit_assign_expr(Expr::Assign* expr) {
    // The left side of the assignment must be an lvalue
    // Currently, lvalues can be Expr::Identifier or Expr::Access
    // Visit the left and right sides of the assignment
    auto l_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));
    auto r_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    auto l_value = std::dynamic_pointer_cast<Expr::Locatable>(expr->left);
    if (l_value == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_ASSIGN_TO_NON_LVALUE, "Left side of assignment is not an lvalue.");
        throw LocalTypeException();
    }
    auto declarer = l_value->get_lvalue_declarer();

    if (declarer == KW_CONST) {
        ErrorLogger::inst().log_error(expr->location, E_ASSIGN_TO_CONST, "Cannot assign to a constant.");
        throw LocalTypeException();
    }

    // The types of the left and right sides must match
    if (!Type::are_compatible(l_type, r_type)) {
        ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + r_type->to_string() + " to " + l_type->to_string() + ".");
        throw LocalTypeException();
    }

    // The type of the expression is the type of the left side
    expr->type = l_type;
    return expr->type;
}

std::any LocalChecker::visit_logical_expr(Expr::Logical* expr) {
    // There are 2 logical operators: `&&` and `||`
    // In both cases, the operands must be of type `bool` and the result is of type `bool`

    auto l_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));
    auto r_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    if (!Type::are_compatible(l_type, r_type)) {
        ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to types " + l_type->to_string() + " and " + r_type->to_string() + ".");
        throw LocalTypeException();
    }

    if (l_type->to_string() != "::bool") {
        ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to type " + l_type->to_string() + ". Expected type 'bool'.");
        throw LocalTypeException();
    }

    expr->type = l_type;
    return expr->type;
}

std::any LocalChecker::visit_binary_expr(Expr::Binary* expr) {
    // There are 12 binary operators: `+`, `-`, `*`, `/`, `%`, `^`, `==`, `!=`, `<`, `<=`, `>`, `>=`

    // For now, we require that operands have the exact required types (no implicit conversions)
    auto l_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));
    auto r_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    TokenType op = expr->op.tok_type;

    if (check_token(op, {TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_CARET})) {
        // For PLUS, MINUS, STAR, SLASH the operands must be equal and must be of type `int` or `float` and the result is of the same type

        if (!Type::are_compatible(l_type, r_type)) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to types " + l_type->to_string() + " and " + r_type->to_string() + ".");
            throw LocalTypeException();
        }
        if (!l_type->is_int() && !l_type->is_float()) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to type " + l_type->to_string() + ". Expected int or float.");
            throw LocalTypeException();
        }
        expr->type = l_type;
    } else if (check_token(op, {TOK_PERCENT})) {
        // For PERCENT, the operands must be equal and must be of type `int` and the result is of type `int`
        if (!Type::are_compatible(l_type, r_type)) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to types " + l_type->to_string() + " and " + r_type->to_string() + ".");
            throw LocalTypeException();
        }
        if (!l_type->is_int()) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to type " + l_type->to_string() + ". Expected int.");
            throw LocalTypeException();
        }
        expr->type = l_type;
    } else if (check_token(op, {TOK_CARET})) {
        // For CARET, the operands must be equal and must be either `int` or `float`. Unlike the other operators, the result is always of type `f64`
        if (!Type::are_compatible(l_type, r_type)) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to types " + l_type->to_string() + " and " + r_type->to_string() + ".");
            throw LocalTypeException();
        }
        if (!l_type->is_int() && !l_type->is_float()) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to type " + l_type->to_string() + ". Expected int or float.");
            throw LocalTypeException();
        }
        expr->type = Environment::inst().get_type("f64");
    } else if (check_token(op, {TOK_EQ_EQ, TOK_BANG_EQ, TOK_LT, TOK_LE, TOK_GT, TOK_GE})) {
        // For EQ_EQ, BANG_EQ, LT, LE, GT, GE the operands must be equal and must be of type `int` or `float` and the result is of type `bool`
        if (!Type::are_compatible(l_type, r_type)) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to types " + l_type->to_string() + " and " + r_type->to_string() + ".");
            throw LocalTypeException();
        }
        if (!l_type->is_int() && !l_type->is_float()) {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply operator '" + expr->op.lexeme + "' to type " + l_type->to_string() + ". Expected int or float.");
            throw LocalTypeException();
        }
        expr->type = Environment::inst().get_type("bool");
    } else {
        // Unreachable
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Unknown binary operator.");
    }

    return expr->type;
}

std::any LocalChecker::visit_unary_expr(Expr::Unary* expr) {
    // There are 4 unary operators: `!`, `-`, `*`, and `&`

    auto operand_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    if (expr->op.tok_type == TOK_BANG) {
        // The operand must be of type `bool`
        if (operand_type->to_string() != "::bool") {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply unary operator '!' to type " + operand_type->to_string() + ". Expected type 'bool'.");
            throw LocalTypeException();
        }
        expr->type = operand_type;
    } else if (expr->op.tok_type == TOK_MINUS) {
        // The operand must be of type `int` or `float`
        if (operand_type->is_int() || operand_type->is_float()) {
            expr->type = operand_type;
        } else {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot apply unary operator '-' to type " + operand_type->to_string() + ". Expected int or float.");
            throw LocalTypeException();
        }
    } else if (expr->op.tok_type == TOK_AMP) {
        auto l_value = std::dynamic_pointer_cast<Expr::Locatable>(expr->right);
        if (l_value == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_ADDRESS_OF_NON_LVALUE, "Cannot take the address of a non-lvalue.");
            throw LocalTypeException();
        }
        // The type of the expression is a pointer to the type of the operand
        auto ptr_type = std::make_shared<Type::Pointer>(operand_type);
        ptr_type->declarer = l_value->get_lvalue_declarer();
        expr->type = ptr_type;

    } else {
        // Unreachable
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Unknown unary operator.");
    }

    return expr->type;
}

std::any LocalChecker::visit_dereference_expr(Expr::Dereference* expr) {
    auto operand_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    // The operand must be a pointer type
    auto operand_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(operand_type);
    if (operand_ptr_type == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_DEREFERENCE_NON_POINTER, "Cannot dereference non-pointer type " + operand_type->to_string() + ".");
        throw LocalTypeException();
    }
    // The type of the expression is the type of the pointer
    expr->type = operand_ptr_type->inner_type;

    return expr->type;
}

std::any LocalChecker::visit_call_expr(Expr::Call* expr) {
    // The left side of the call expression must be callable; i.e. a function pointer type
    auto left_type = std::any_cast<std::shared_ptr<Type>>(expr->callee->accept(this));
    if (!IS_TYPE(left_type, Type::Function)) {
        ErrorLogger::inst().log_error(expr->location, E_CALL_ON_NON_FUN, "Expression is not callable.");
        throw LocalTypeException();
    }
    auto fun_type = std::dynamic_pointer_cast<Type::Function>(left_type);

    // First, the number of arguments must match the number of parameters
    // If the function is variadic, the number of arguments must be greater than or equal to the number of parameters
    if (!fun_type->is_variadic && expr->arguments.size() != fun_type->params.size()) {
        ErrorLogger::inst().log_error(expr->location, E_INVALID_ARITY, "Expected " + std::to_string(fun_type->params.size()) + " arguments, found " + std::to_string(expr->arguments.size()) + ".");
        throw LocalTypeException();
    } else if (fun_type->is_variadic && expr->arguments.size() < fun_type->params.size()) {
        ErrorLogger::inst().log_error(expr->location, E_INVALID_ARITY, "Expected at least " + std::to_string(fun_type->params.size() - 1) + " arguments, found " + std::to_string(expr->arguments.size()) + ".");
        throw LocalTypeException();
    }

    // Then, the types of the arguments must match the types of the parameters
    for (unsigned i = 0; i < expr->arguments.size(); i++) {
        // All arguments must be visited to ensure that the types are resolved
        auto arg_type = std::any_cast<std::shared_ptr<Type>>(expr->arguments[i]->accept(this));
        // We add a range check here in case the function is variadic
        // If there are more args than params, the extra args will be visited, but not compared against any params
        if (i < fun_type->params.size() && !Type::are_compatible(arg_type, fun_type->params[i].second)) {
            ErrorLogger::inst().log_error(expr->arguments[i]->location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + arg_type->to_string() + " to " + fun_type->params[i].second->to_string() + ".");
            throw LocalTypeException();
        }
    }

    // The type of the call expression is the return type of the function
    expr->type = fun_type->return_type;
    return expr->type;
}

std::any LocalChecker::visit_access_expr(Expr::Access* expr) {
    auto left_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));

    if (expr->op.tok_type == TOK_DOT) {
        std::shared_ptr<Expr::Identifier> right = std::dynamic_pointer_cast<Expr::Identifier>(expr->right);

        if (right == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_NO_IDENT_AFTER_DOT, "Expected identifier after '.'.");
            throw LocalTypeException();
        }
        auto left_seg_type = std::dynamic_pointer_cast<Type::Struct>(left_type);
        if (left_seg_type == nullptr) {
            if (IS_TYPE(left_type, Type::Pointer)) {
                // This is still an error, but we can give a more specific error message
                ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type. Did you mean to use '->' instead of '.'?");
                throw LocalTypeException();
            }
            ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
            throw LocalTypeException();
        }

        expr->type = Environment::inst().get_instance_variable(left_seg_type, right->to_string())->type;
        // If this returns nullptr, the member was not found
        if (expr->type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INVALID_STRUCT_MEMBER, "Struct type " + left_seg_type->to_string() + " does not have member " + right->to_string() + ".");
            throw LocalTypeException();
        }

        return expr->type;
    } else if (expr->op.tok_type == TOK_ARROW) {
        // Pretty much the same as the dot operator, but the left side is dereferenced first
        auto left_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(left_type);
        if (left_ptr_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_DEREFERENCE_NON_POINTER, "Cannot dereference non-pointer type. Did you mean to use '.' instead of '->'?");
            throw LocalTypeException();
        }
        std::shared_ptr<Expr::Identifier> right = std::dynamic_pointer_cast<Expr::Identifier>(expr->right);
        if (right == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_NO_IDENT_AFTER_DOT, "Expected identifier after '->'.");
            throw LocalTypeException();
        }
        auto left_seg_type = std::dynamic_pointer_cast<Type::Struct>(left_ptr_type->inner_type);
        if (left_seg_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
            throw LocalTypeException();
        }

        expr->type = Environment::inst().get_instance_variable(left_seg_type, right->to_string())->type;
        // If this returns nullptr, the member was not found
        if (expr->type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INVALID_STRUCT_MEMBER, "Struct type " + left_seg_type->to_string() + " does not have member " + right->to_string() + ".");
            throw LocalTypeException();
        }

        return expr->type;
    } else if (expr->op.tok_type == TOK_LEFT_SQUARE) {
        // // Right now, this only works with arrays
        auto left_arr_type = std::dynamic_pointer_cast<Type::Array>(left_type);
        auto left_tuple_type = std::dynamic_pointer_cast<Type::Tuple>(left_type);

        if (left_arr_type == nullptr && left_tuple_type == nullptr) {
            ErrorLogger::inst().log_error(expr->location, E_INDEX_ON_NON_ARRAY, "Subscript operator can only be used on arrays and tuples.");
            throw LocalTypeException();
        }

        // The index must be an integer
        auto index_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));
        if (index_type != nullptr && index_type->to_string() != "::i32") {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot index array with type " + index_type->to_string() + ". Expected type 'i32'.");
            throw LocalTypeException();
        }

        // The type of the expression is the type of the array elements
        if (left_arr_type != nullptr) {
            expr->type = left_arr_type->inner_type;
        } else {
            // For tuples, the index must be a literal integer
            auto index_expr = std::dynamic_pointer_cast<Expr::Literal>(expr->right);
            if (index_expr == nullptr || index_expr->token.tok_type != TOK_INT) {
                ErrorLogger::inst().log_error(expr->location, E_NO_LITERAL_INDEX_ON_TUPLE, "Index must be a literal integer.");
                throw LocalTypeException();
            }
            auto index = std::any_cast<int>(index_expr->token.literal);
            if (index < 0 || index >= left_tuple_type->elements.size()) {
                ErrorLogger::inst().log_error(expr->location, E_TUPLE_INDEX_OUT_OF_RANGE, "Index out of range for tuple of size " + std::to_string(left_tuple_type->elements.size()) + ".");
                throw LocalTypeException();
            }
            expr->type = left_tuple_type->elements[index];
        }

        return expr->type;
    } else {
        // Unreachable
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Unknown access operator.");
        throw LocalTypeException();
    }
}

std::any LocalChecker::visit_grouping_expr(Expr::Grouping* expr) {
    expr->type = std::any_cast<std::shared_ptr<Type>>(expr->expression->accept(this));
    return expr->type;
}

std::any LocalChecker::visit_identifier_expr(Expr::Identifier* expr) {
    auto var_node = Environment::inst().get_variable(expr->tokens);
    if (var_node == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_UNKNOWN_VAR, "Variable `" + expr->to_string() + "` was not declared.");
        throw LocalTypeException();
    }
    expr->type = var_node->type;
    if (expr->type == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
        throw LocalTypeException();
    }
    return expr->type;
}

std::any LocalChecker::visit_literal_expr(Expr::Literal* expr) {

    std::shared_ptr<Type> type = nullptr;

    switch (expr->token.tok_type) {
    case TOK_INT:
        type = Environment::inst().get_type("i32");
        break;
    case TOK_FLOAT:
        type = Environment::inst().get_type("f64");
        break;
    case TOK_CHAR:
        type = Environment::inst().get_type("char");
        break;
    case TOK_STR:
        type = Environment::inst().get_type("char");
        type = std::make_shared<Type::Pointer>(type);
        break;
    case TOK_BOOL:
        type = Environment::inst().get_type("bool");
        break;
    case TOK_NIL:
        type = std::make_shared<Type::Blank>();
        type = std::make_shared<Type::Pointer>(type);
        // Creates type `auto*`
        // Note: `auto` is not a primitive type, so this is not a valid type.
        // However, when later checked for type compatibility, the type will be updated to the correct type.
        break;
    default:
        ErrorLogger::inst().log_error(expr->location, E_UNRECOGNIZED_LITERAL, "Unknown literal type.");
        throw LocalTypeException();
    }

    if (type == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_IMPOSSIBLE, "Could not resolve primitive type annotation.");
        throw LocalTypeException();
    }

    expr->type = type;
    return type;
}

std::any LocalChecker::visit_array_expr(Expr::Array* expr) {
    // Handle the case where the array is empty
    if (expr->elements.empty()) {
        expr->type = std::make_shared<Type::Blank>();
        expr->type = std::make_shared<Type::Array>(expr->type);
        return expr->type;
    } else {
        // Ensure all elements have the same type
        std::shared_ptr<Type> type = std::any_cast<std::shared_ptr<Type>>(expr->elements[0]->accept(this));
        // In the case that this element is an `auto` type, the repeated compatibility checks in the following loop will build the type until it is complete.
        for (auto& elem : expr->elements) {
            auto elem_type = std::any_cast<std::shared_ptr<Type>>(elem->accept(this));
            if (Type::are_compatible(type, elem_type) == false) {
                ErrorLogger::inst().log_error(elem->location, E_INCONSISTENT_ARRAY_TYPES, "Array elements must have the same type. Expected " + type->to_string() + ", found " + elem_type->to_string() + ".");
                throw LocalTypeException();
            }
        }
        // If, after this loop, the type still contains `auto`, it will later be checked against the type annotation on the left-hand side of the assignment
        auto arr_type = std::make_shared<Type::Array>(type);
        expr->type = arr_type;
        return type;
    }
}

std::any LocalChecker::visit_tuple_expr(Expr::Tuple* expr) {
    std::vector<std::shared_ptr<Type>> types;
    for (auto& elem : expr->elements) {
        std::shared_ptr<Type> elem_type = std::any_cast<std::shared_ptr<Type>>(elem->accept(this));
        types.push_back(elem_type);
    }
    expr->type = std::make_shared<Type::Tuple>(types);
    return expr->type;
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
