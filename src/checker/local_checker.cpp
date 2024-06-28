#include "local_checker.h"
#include "../logger/logger.h"
#include "../scanner/token.h"
#include "../utility/type.h"
#include "../utility/utils.h"
#include <iostream>
#include <tuple>
#include <unordered_set>

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
    stmt->declaration->accept(this);
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_expression_stmt(Stmt::Expression* stmt) {
    // This is a local expression statement. Any global expression statements should have been caught by the global checker.
    // Visit the expression.
    stmt->expression->accept(this);
    // Expression statements do not produce any value.
    return std::shared_ptr<Type>(nullptr);
}
std::any LocalChecker::visit_block_stmt(Stmt::Block* /* stmt */) {
    // Not yet implemented
    // TODO: Implement block statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Block statements are not yet implemented.");
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_conditional_stmt(Stmt::Conditional* stmt) {
    // First, check that the conditional expression is of type `bool`
    auto cond_type = std::any_cast<std::shared_ptr<Type>>(stmt->condition->accept(this));
    if (cond_type->to_string() != "::bool") {
        ErrorLogger::inst().log_error(stmt->condition->location, E_CONDITIONAL_WITHOUT_BOOL, "Expected expression of type `bool`; Found `" + cond_type->to_string() + "`.");
        throw LocalTypeException();
    }
    std::shared_ptr<Stmt> prev_ret_stmt = nullptr;
    std::shared_ptr<Type> ret_type = nullptr;
    // Increase the local scope for the then_branch
    Environment::inst().increase_local_scope();
    // Visit the then_branch
    for (auto& if_stmt : stmt->then_branch) {
        // If one of these statements returns something...
        auto temp_type = std::any_cast<std::shared_ptr<Type>>(if_stmt->accept(this));
        // Ensure the return type is consistent...
        if (ret_type != nullptr && temp_type != nullptr && Type::are_compatible(ret_type, temp_type) != 0) {
            ErrorLogger::inst().log_error(if_stmt->location, E_INCONSISTENT_RETURN_TYPES, "Return type is inconsistent with a previous return statement.");
            ErrorLogger::inst().log_note(prev_ret_stmt->location, "Previous return statement was here.");
            throw LocalTypeException();
        }
        // ...and store the return type
        if (temp_type != nullptr) {
            ret_type = temp_type;
            prev_ret_stmt = if_stmt;
        }
    }
    // Exit the local scope for the then_branch
    Environment::inst().exit_scope();
    // Do the same for the else_branch
    Environment::inst().increase_local_scope();
    // Visit the else_branch
    for (auto& if_stmt : stmt->else_branch) {
        auto temp = if_stmt->accept(this);
        // If one of these statements returns something...
        auto temp_type = std::any_cast<std::shared_ptr<Type>>(if_stmt->accept(this));
        // Ensure the return type is consistent...
        if (ret_type != nullptr && temp_type != nullptr && Type::are_compatible(ret_type, temp_type) != 0) {
            ErrorLogger::inst().log_error(if_stmt->location, E_INCONSISTENT_RETURN_TYPES, "Return type is inconsistent with a previous return statement.");
            ErrorLogger::inst().log_note(prev_ret_stmt->location, "Previous return statement was here.");
            throw LocalTypeException();
        }
        // ...and store the return type
        if (temp_type != nullptr) {
            ret_type = temp_type;
            prev_ret_stmt = if_stmt;
        }
    }

    return ret_type;
}

std::any LocalChecker::visit_loop_stmt(Stmt::Loop* /* stmt */) {
    // Log error with location
    // TODO: Implement loop statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Loop statements are not yet implemented.");
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_return_stmt(Stmt::Return* stmt) {
    // Return statements are not allowed in global scope
    // We already checked for this in the global checker

    if (stmt->value == nullptr) {
        return std::shared_ptr<Type>(nullptr);
    }

    std::shared_ptr<Type> ret_type = std::any_cast<std::shared_ptr<Type>>(stmt->value->accept(this));

    return ret_type;
}

std::any LocalChecker::visit_break_stmt(Stmt::Break* /* stmt */) {
    // Log error with location
    // TODO: Implement break statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Break statements are not yet implemented.");
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_continue_stmt(Stmt::Continue* /* stmt */) {
    // Log error with location
    // TODO: Implement continue statements
    // ErrorLogger::inst().log_error(stmt->location, E_UNIMPLEMENTED, "Continue statements are not yet implemented.");
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_eof_stmt(Stmt::EndOfFile* /* stmt */) {
    // Does nothing (for now)
    return std::shared_ptr<Type>(nullptr);
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
        std::tie(node, result) = Environment::inst().declare_variable(decl);
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
    ErrorCode ec = Type::are_compatible(variable->decl->type, init_type);
    if (ec != 0) {
        // If the error occurred because init_type is an array of blank types, give a more specific error message
        // auto init_array = std::dynamic_pointer_cast<Type::Array>(init_type);
        if (ec == E_INDETERMINATE_ARRAY_TYPE) {
            ErrorLogger::inst().log_error(decl->initializer->location, ec, "The type of this array could not be determined.");
            ErrorLogger::inst().log_note(decl->name.location, "Missing type annotation.");
            throw LocalTypeException();
        }
        // auto var_array = std::dynamic_pointer_cast<Type::Array>(variable->decl->type);
        if (ec == E_SIZED_ARRAY_WITHOUT_INITIALIZER) {
            ErrorLogger::inst().log_error(decl->name.location, ec, "An array with a known size must have an initializer.");
            throw LocalTypeException();
        }
        if (ec == E_ARRAY_SIZE_UNKNOWN) {
            ErrorLogger::inst().log_error(decl->name.location, ec, "Cannot implicitly convert from " + init_type->to_string() + " to " + variable->decl->type->to_string() + ".");
            ErrorLogger::inst().log_note(decl->initializer->location, "Size is unknown.");
            throw LocalTypeException();
        }

        ErrorLogger::inst().log_error(decl->name.location, ec, "Cannot convert from " + init_type->to_string() + " to " + variable->decl->type->to_string() + ".");
        throw LocalTypeException();
    }
    // Verify that none of the types are blank
    if (variable->decl->type->kind() == Type::Kind::BLANK || init_type->kind() == Type::Kind::BLANK) {
        ErrorLogger::inst().log_error(decl->name.location, E_UNKNOWN_TYPE, "Could not resolve type annotation.");
        throw LocalTypeException();
    }
    // Verify that the right side is not a const pointer being assigned to a non-const pointer
    auto init_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(init_type);
    if (init_ptr_type != nullptr && init_ptr_type->declarer == KW_CONST && variable->decl->declarer != KW_CONST) {
        ErrorLogger::inst().log_error(decl->name.location, E_INVALID_PTR_DECLARER, "Cannot assign a const pointer to a non-const pointer.");
        throw LocalTypeException();
    } else if (init_ptr_type != nullptr && variable->decl->declarer == KW_CONST) {
        // If the variable is const, the pointer must be const as well
        init_ptr_type->declarer = KW_CONST;
    }

    return std::shared_ptr<Type>(nullptr);
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
            decl->parameters[i].get()
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
        auto param_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(param_var->decl->type);
        if (param_ptr_type != nullptr) {
            param_ptr_type->declarer = param_var->decl->declarer;
        }

        // We don't worry about initializers for parameters; this is just a type checker
    }

    // At this point, the function type should be fully resolved
    if (variable == nullptr) {
        ErrorLogger::inst().log_error(decl->name.location, E_IMPOSSIBLE, "Function pointer variable is nullptr in LocalChecker::visit_fun_decl.");
        throw LocalTypeException();
    }
    auto variable_fun_type = std::dynamic_pointer_cast<Type::Function>(variable->decl->type);

    // Increase the local scope for the function body
    Environment::inst().increase_local_scope();

    // Verify the body of the function
    // If any statement returns a non-empty any, either the statement is a return statement or is a block containing one
    // Verify the return statement types and log the appropriate error messages
    bool has_return = false;
    for (auto& stmt : decl->body) {
        // std::any stmt_type = stmt->accept(this);
        std::shared_ptr<Type> stmt_type = std::any_cast<std::shared_ptr<Type>>(stmt->accept(this));
        if (stmt_type != nullptr) {
            has_return = true;
            if (variable->decl->type->to_string() == "::void") {
                ErrorLogger::inst().log_error(stmt->location, E_RETURN_IN_VOID_FUN, "Function with return type 'void' cannot return a value.");
                throw LocalTypeException();
            } else {
                // The return type of the function must match the return type of the return statement
                // This will also catch the case where the function return type is `void` and the return statement has a value
                if (Type::are_compatible(stmt_type, variable_fun_type->return_type) != 0) {
                    ErrorLogger::inst().log_error(stmt->location, E_RETURN_INCOMPATIBLE, "Cannot convert from " + stmt_type->to_string() + " to return type " + variable_fun_type->return_type->to_string() + ".");
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

    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_extern_fun_decl(Decl::ExternFun* decl) {
    // Function declarations are not allowed in local scope
    if (!Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(decl->name.location, E_FUN_IN_LOCAL_SCOPE, "Function declarations are not allowed in local scope.");
        throw LocalTypeException();
    }

    // No need to do further checks; we've already done that in the global checker
    return std::shared_ptr<Type>(nullptr);
}

std::any LocalChecker::visit_struct_decl(Decl::Struct* decl) {
    // Struct declarations are not allowed in local scope
    if (!Environment::inst().in_global_scope()) {
        ErrorLogger::inst().log_error(decl->name.location, E_STRUCT_IN_LOCAL_SCOPE, "Struct declarations are not allowed in local scope.");
        throw LocalTypeException();
    }

    // Enter the struct scope
    Environment::inst().enter_scope(decl->name.lexeme);

    // We check only the static declarations of the struct
    for (auto declaration : decl->declarations) {
        if (IS_TYPE(declaration, Decl::Fun)) {
            declaration->accept(this);
        }
    }

    // Exit the struct scope
    Environment::inst().exit_scope();

    return std::shared_ptr<Type>(nullptr);
}

// MARK: Expressions

std::any LocalChecker::visit_assign_expr(Expr::Assign* expr) {
    // The left side of the assignment must be an lvalue
    // Currently, lvalues can be Expr::Identifier or Expr::Access
    // Visit the left and right sides of the assignment
    auto l_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));
    auto r_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));

    auto l_value = std::dynamic_pointer_cast<Expr::LValue>(expr->left);
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
    if (Type::are_compatible(l_type, r_type) != 0) {
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

    if (Type::are_compatible(l_type, r_type) != 0) {
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

    if (check_token(op, {TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH})) {
        // For PLUS, MINUS, STAR, SLASH the operands must be equal and must be of type `int` or `float` and the result is of the same type

        if (Type::are_compatible(l_type, r_type) != 0) {
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
        if (Type::are_compatible(l_type, r_type) != 0) {
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
        if (Type::are_compatible(l_type, r_type) != 0) {
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
        if (Type::are_compatible(l_type, r_type) != 0) {
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
    // There are 3 unary operators: `!`, `-`, and `&`

    auto operand_type = std::any_cast<std::shared_ptr<Type>>(expr->inner->accept(this));

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
        auto l_value = std::dynamic_pointer_cast<Expr::LValue>(expr->inner);
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
    auto operand_type = std::any_cast<std::shared_ptr<Type>>(expr->inner->accept(this));

    // The operand must be a pointer type
    auto operand_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(operand_type);
    if (operand_ptr_type == nullptr) {
        ErrorLogger::inst().log_error(expr->inner->location, E_DEREFERENCE_NON_POINTER, "Cannot dereference non-pointer type " + operand_type->to_string() + ".");
        // If the operator is `->`, we can give a more specific error message
        if (expr->op.tok_type == TOK_ARROW) {
            ErrorLogger::inst().log_note(expr->op.location, "Did you mean to use '.' instead of '->'?");
        }
        throw LocalTypeException();
    }
    // The type of the expression is the type of the pointer
    expr->type = operand_ptr_type->inner_type;

    return expr->type;
}

std::any LocalChecker::visit_access_expr(Expr::Access* expr) {
    auto left_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));

    // The left side of the access must be a struct type
    auto left_seg_type = std::dynamic_pointer_cast<Type::Named>(left_type);
    if (left_seg_type == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_ACCESS_ON_NON_STRUCT, "Cannot access member of non-struct type.");
        // If the left side is a pointer, but the operator is `.`, we can give a more specific error message
        if (IS_TYPE(left_type, Type::Pointer) && expr->op.tok_type == TOK_DOT) {
            ErrorLogger::inst().log_note(expr->op.location, "Did you mean to use '->' instead of '.'?");
        }
        throw LocalTypeException();
    }

    // The right side of the access is an identifier
    auto decl = Environment::inst().get_instance_variable(left_seg_type, expr->ident.lexeme);
    if (decl != nullptr) {
        // The type of the expression is the type of the instance variable
        expr->type = decl->type;
        return expr->type;
    }
    // If the instance variable is not found, check static variables
    auto struct_scope = left_seg_type->struct_scope;
    auto static_var_iter = struct_scope->children.find(expr->ident.lexeme);
    if (static_var_iter != struct_scope->children.end()) {
        auto static_var = std::dynamic_pointer_cast<Node::Variable>(static_var_iter->second);
        expr->type = static_var->decl->type;
        return expr->type;
    }

    ErrorLogger::inst().log_error(expr->ident.location, E_INVALID_STRUCT_MEMBER, "Struct type " + left_seg_type->to_string() + " does not have member " + expr->ident.lexeme + ".");
    throw LocalTypeException();
}

std::any LocalChecker::visit_index_expr(Expr::Index* expr) {
    auto left_type = std::any_cast<std::shared_ptr<Type>>(expr->left->accept(this));

    // First, handle the case where expr is an array
    auto left_arr_type = std::dynamic_pointer_cast<Type::Array>(left_type);
    if (left_arr_type != nullptr) {
        // The index must be an integer
        auto index_type = std::any_cast<std::shared_ptr<Type>>(expr->right->accept(this));
        if (index_type != nullptr && index_type->to_string() != "::i32") {
            ErrorLogger::inst().log_error(expr->location, E_INCOMPATIBLE_TYPES, "Cannot index array with type " + index_type->to_string() + ". Expected type 'i32'.");
            throw LocalTypeException();
        }

        // The type of the expression is the type of the array elements
        expr->type = left_arr_type->inner_type;
        return expr->type;
    }

    // Then, handle the case where expr is a tuple
    auto left_tuple_type = std::dynamic_pointer_cast<Type::Tuple>(left_type);
    if (left_tuple_type != nullptr) {
        // The index must be a literal integer
        auto index_expr = std::dynamic_pointer_cast<Expr::Literal>(expr->right);
        // A literal is required to verify the type at compile time
        if (index_expr == nullptr || index_expr->token.tok_type != TOK_INT) {
            ErrorLogger::inst().log_error(expr->right->location, E_NO_LITERAL_INDEX_ON_TUPLE, "Tuple index must be a literal integer.");
            throw LocalTypeException();
        }
        auto index = std::any_cast<int>(index_expr->token.literal);
        if (index < 0 || index >= left_tuple_type->element_types.size()) {
            ErrorLogger::inst().log_error(expr->location, E_TUPLE_INDEX_OUT_OF_RANGE, "Index out of range for tuple of size " + std::to_string(left_tuple_type->element_types.size()) + ".");
            throw LocalTypeException();
        }
        expr->type = left_tuple_type->element_types[index];
        return expr->type;
    }

    // If neither of the above cases are true, the expression is invalid
    ErrorLogger::inst().log_error(expr->location, E_INDEX_ON_NON_ARRAY, "Subscript operator can only be used on arrays and tuples.");
    throw LocalTypeException();
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
        if (i < fun_type->params.size() && Type::are_compatible(arg_type, fun_type->params[i].second) != 0) {
            ErrorLogger::inst().log_error(expr->arguments[i]->location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + arg_type->to_string() + " to " + fun_type->params[i].second->to_string() + ".");
            throw LocalTypeException();
        }
    }

    // The type of the call expression is the return type of the function
    expr->type = fun_type->return_type;
    return expr->type;
}

std::any LocalChecker::visit_cast_expr(Expr::Cast* expr) {
    auto left_type = std::any_cast<std::shared_ptr<Type>>(expr->expression->accept(this));
    auto target_type = Environment::inst().get_type(expr->annotation);

    // We'll let the code generator handle the specifics of the cast
    if (left_type->is_numeric() && target_type->is_numeric()) {
        // If both types are numeric, the cast is allowed
        expr->type = target_type;
    } else if ((left_type->is_numeric() || left_type->kind() == Type::Kind::POINTER) && target_type->to_string() == "::bool") {
        // If the left type is numeric or a pointer, and the target type is bool, the cast is allowed
        expr->type = target_type;
        // This type cast allows people to check if a number is 0 or a pointer is null
    } else {
        ErrorLogger::inst().log_error(expr->location, E_INVALID_CAST, "Cannot cast from " + left_type->to_string() + " to " + target_type->to_string() + ".");
        throw LocalTypeException();
    }
    // Currently, no other casts are allowed
    // Pointer casts and array size casts may be allowed in the future,
    // but they will likely require more syntax rules since they are inherently unsafe

    return expr->type;
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
    expr->type = var_node->decl->type;
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
        expr->type = std::make_shared<Type::Array>(expr->type, 0);
        return expr->type;
    } else {
        // Ensure all elements have the same type
        std::shared_ptr<Type> inner_type = std::any_cast<std::shared_ptr<Type>>(expr->elements[0]->accept(this));
        // In the case that this element is an `auto` type, the repeated compatibility checks in the following loop will build the type until it is complete.
        for (auto& elem : expr->elements) {
            auto elem_type = std::any_cast<std::shared_ptr<Type>>(elem->accept(this));
            if (Type::are_compatible(inner_type, elem_type) != 0) {
                ErrorLogger::inst().log_error(elem->location, E_INCONSISTENT_ARRAY_TYPES, "Array elements must have the same type. Expected " + inner_type->to_string() + ", found " + elem_type->to_string() + ".");
                throw LocalTypeException();
            }
        }
        // If, after this loop, the type still contains `auto`, it will later be checked against the type annotation on the left-hand side of the assignment
        auto arr_type = std::make_shared<Type::Array>(inner_type, expr->elements.size());
        expr->type = arr_type;
        return expr->type;
    }
}

std::any LocalChecker::visit_array_gen_expr(Expr::ArrayGen* expr) {
    // The type of the array generator is the type of the elements
    auto elem_type = std::any_cast<std::shared_ptr<Type>>(expr->generator->accept(this));
    expr->type = std::make_shared<Type::Array>(elem_type, (int)expr->size);
    return expr->type;
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

std::any LocalChecker::visit_object_expr(Expr::Object* expr) {
    /*
     * An object expression is valid if:
     * - The struct exists
     * - The object expression has the same fields as the struct
     * - The object expression does not have any extra fields or static fields
     * - The types of the fields match the types of the struct fields
     * - Any fields missing from the object have default values in the struct declaration
     */

    // Get the struct type
    auto type = Environment::inst().get_type(expr->struct_annotation);
    auto struct_type = std::dynamic_pointer_cast<Type::Named>(type);
    if (struct_type == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_UNKNOWN_TYPE, "Struct type `" + expr->struct_annotation->to_string() + "` was not declared.");
        throw LocalTypeException();
    }

    // Create a set of the required fields
    std::unordered_set<std::string> required_fields;
    for (auto& field : struct_type->struct_scope->instance_members) {
        auto vardeclarable_decl = field.second;
        auto var_decl = dynamic_cast<Decl::Var*>(vardeclarable_decl);
        if (var_decl == nullptr) {
            // This should never happen, since all instance members are variables
            ErrorLogger::inst().log_error(field.second->location, E_IMPOSSIBLE, "Instance member is not a variable.");
            throw LocalTypeException();
        }
        if (var_decl->initializer == nullptr) {
            // If the field does not have a default value, it is required
            required_fields.insert(field.first);
        } else if (!HAS_KEY(expr->fields, field.first)) {
            // If the field has a default value, and the object expression does not have that field, add the default value to the object expression
            expr->fields[field.first] = var_decl->initializer;
        }
    }

    // Check that the object expression has the same fields as the struct
    for (auto& field : expr->fields) {
        if (!HAS_KEY(struct_type->struct_scope->instance_members, field.first)) {
            // If the field is actually a static field, we can give a more specific error message
            if (HAS_KEY(struct_type->struct_scope->children, field.first)) {
                ErrorLogger::inst().log_error(field.second->location, E_STATIC_FIELD_IN_OBJ, "Cannot assign to static field `" + field.first + "` in object expression.");
            } else {
                ErrorLogger::inst().log_error(field.second->location, E_INVALID_STRUCT_MEMBER, "Struct type `" + struct_type->to_string() + "` does not have instance member `" + field.first + "`.");
            }
            throw LocalTypeException();
        }

        // Remove the field from the set of required fields
        required_fields.erase(field.first);
        auto field_decl = struct_type->struct_scope->instance_members.at(field.first);
        // This should always be valid, since the required fields were taken from the struct's instance members

        // Check that the types of the fields match
        auto field_type = std::any_cast<std::shared_ptr<Type>>(field.second->accept(this));
        if (Type::are_compatible(field_type, field_decl->type) != 0) {
            ErrorLogger::inst().log_error(field.second->location, E_INCOMPATIBLE_TYPES, "Cannot convert from " + field_type->to_string() + " to " + field_decl->type->to_string() + ".");
            ErrorLogger::inst().log_note(field_decl->location, "Field declared here with type " + field_decl->type->to_string() + ".");
            throw LocalTypeException();
        }
    }

    // If there are any required fields left, the object expression is invalid
    if (!required_fields.empty()) {
        ErrorLogger::inst().log_error(expr->location, E_MISSING_FIELD_IN_OBJ, "Object expression is missing required fields.");
        for (auto& field : required_fields) {
            auto field_decl = struct_type->struct_scope->instance_members.at(field);
            ErrorLogger::inst().log_note(field_decl->location, "This field is required.");
        }
        throw LocalTypeException();
    }

    // The object expression is valid
    expr->type = struct_type;
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
