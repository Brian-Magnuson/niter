#include "code_generator.h"
#include "../checker/environment.h"
#include "../logger/logger.h"
#include "../utility/node.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <filesystem>
#include <iostream>

void CodeGenerator::declare_all_structs() {

    auto struct_scopes = Environment::inst().get_struct_scopes();
    // First pass: create all the struct types without bodies
    for (auto& struct_scope : struct_scopes) {
        auto llvm_safe_name = struct_scope->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');
        llvm::StructType* llvm_struct_type = llvm::StructType::create(*context, llvm_safe_name);
        struct_scope->ir_type = llvm_struct_type;
    }

    // Second pass: set the bodies of all the struct types
    for (auto& struct_scope : struct_scopes) {
        std::vector<llvm::Type*> member_types;

        for (auto& [name, decl] : struct_scope->instance_members) {
            llvm::Type* member_type = decl->type->to_llvm_type(context);
            member_types.push_back(member_type);
        }

        auto llvm_struct_type = llvm::cast<llvm::StructType>(struct_scope->ir_type);
        llvm_struct_type->setBody(member_types);
    }
}

void CodeGenerator::declare_all_functions() {
    auto fun_nodes = Environment::inst().get_global_functions();

    for (auto& fun_node : fun_nodes) {
        // Create the function type
        llvm::FunctionType* fun_type = llvm::cast<llvm::FunctionType>(fun_node->decl->type->to_llvm_type(context));

        auto llvm_safe_name = fun_node->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        // Create the function
        // If the declaration is an extern function, create an external function
        llvm::Function* fun;
        if (dynamic_cast<Decl::ExternFun*>(fun_node->decl) != nullptr) {
            llvm_safe_name = fun_node->decl->name.lexeme;
            fun = llvm::Function::Create(fun_type, llvm::Function::ExternalLinkage, llvm_safe_name, ir_module.get());
        } else {
            fun = llvm::Function::Create(fun_type, llvm::Function::InternalLinkage, llvm_safe_name, ir_module.get());
        }
        fun_node->llvm_allocation = fun;
    }
}

std::any CodeGenerator::visit_declaration_stmt(Stmt::Declaration* stmt) {
    stmt->declaration->accept(this);
    return nullptr;
}

std::any CodeGenerator::visit_expression_stmt(Stmt::Expression* stmt) {
    stmt->expression->accept(this);
    return nullptr;
}

std::any CodeGenerator::visit_block_stmt(Stmt::Block*) {
    // TODO: Implement block statements
    return nullptr;
}

std::any CodeGenerator::visit_conditional_stmt(Stmt::Conditional*) {
    // TODO: Implement conditional statements
    return nullptr;
}

std::any CodeGenerator::visit_loop_stmt(Stmt::Loop*) {
    // TODO: Implement loop statements
    return nullptr;
}

std::any CodeGenerator::visit_return_stmt(Stmt::Return* stmt) {
    if (stmt->value != nullptr) {
        auto value = std::any_cast<llvm::Value*>(stmt->value->accept(this));
        builder->CreateStore(value, Environment::inst().get_variable({"__return_val__"})->llvm_allocation);
    }
    if (block_stack.size() == 0) {
        ErrorLogger::inst().log_error(stmt->location, E_IMPOSSIBLE, "Return statement outside of function.");
        throw CodeGenException();
    }

    // LLVM requires that each block has one terminator instruction at the end.
    builder->CreateBr(block_stack.front());

    // We allow statements to appear after a return statement. Theoretically, these statements should be unreachable.
    // We allow this anyway because unreachable code is not an error (might generate a warning, though).

    // To prevent LLVM from complaining about terminators in the middle of a block, we create a new block right here.
    auto new_block = llvm::BasicBlock::Create(*context, "unreachable", block_stack.front()->getParent());
    builder->SetInsertPoint(new_block);
    // If this code is truly unreachable, it will be removed by the optimizer (keep things consistent, then optimize).

    return nullptr;
}

std::any CodeGenerator::visit_break_stmt(Stmt::Break*) {
    // TODO: Implement break statement
    return nullptr;
}

std::any CodeGenerator::visit_continue_stmt(Stmt::Continue*) {
    // TODO: Implement continue statement
    return nullptr;
}

std::any CodeGenerator::visit_eof_stmt(Stmt::EndOfFile*) {
    // TODO: Implement eof statement
    return nullptr;
}

std::any CodeGenerator::visit_var_decl(Decl::Var* decl) {

    // This function behaves differently depending on whether this is a global or local variable.
    // If it is a global variable, we need to create a global variable instead of an alloca instruction.
    if (Environment::inst().in_global_scope()) {
        // The node already exists in the tree, so we can just get it.
        auto var_node = Environment::inst().get_variable({decl->name.lexeme});
        // Generate code for the initializer expression.
        llvm::Value* initializer = nullptr;
        if (decl->initializer != nullptr) {
            initializer = std::any_cast<llvm::Value*>(decl->initializer->accept(this));
        } else {
            // If there is no initializer, store the default value.
            initializer = llvm::Constant::getNullValue(var_node->decl->type->to_llvm_type(context));
        }
        // This should never be nullptr
        auto llvm_safe_name = var_node->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        // Attempt to cast the initializer to an llvm constant
        auto constant_initializer = llvm::dyn_cast<llvm::Constant>(initializer);
        if (constant_initializer == nullptr) {
            ErrorLogger::inst().log_error(decl->location, E_NOT_A_CONSTANT, "Global variable initializer is not a constant.");
            throw CodeGenException();
        }

        // Create the global variable
        llvm::GlobalVariable* global = new llvm::GlobalVariable(
            *ir_module,
            var_node->decl->type->to_llvm_type(context),
            false,
            llvm::GlobalValue::InternalLinkage,
            constant_initializer,
            llvm_safe_name
        );
        var_node->llvm_allocation = global;
    } else {
        // For local variables, we need to create an alloca instruction.
        // The node does not exist in the tree, so we need to create it.
        auto [node, _] = Environment::inst().declare_variable(decl);
        auto var_node = std::dynamic_pointer_cast<Node::Variable>(node);
        // This should never be nullptr

        // Generate code for the initializer expression.
        llvm::Value* initializer = nullptr;
        std::shared_ptr<Type> init_type = std::make_shared<Type::Blank>();

        if (decl->initializer != nullptr) {
            initializer = std::any_cast<llvm::Value*>(decl->initializer->accept(this));
            init_type = decl->initializer->type;
        } else {
            // If there is no initializer, store the default value.
            initializer = llvm::Constant::getNullValue(var_node->decl->type->to_llvm_type(context));
        }

        auto llvm_safe_name = var_node->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        llvm::AllocaInst* alloca = builder->CreateAlloca(var_node->decl->type->to_llvm_type(context), nullptr, llvm_safe_name);
        // Store the value in the alloca instruction.
        builder->CreateStore(initializer, alloca);

        // Save the alloca instruction in the variable node.
        var_node->llvm_allocation = alloca;
    }

    return nullptr;
}

std::any CodeGenerator::visit_fun_decl(Decl::Fun* decl) {
    // Get the function node from the environment tree
    auto fun_node = Environment::inst().get_variable({decl->name});
    // This should never be nullptr

    // The function is already created
    auto fun = llvm::cast<llvm::Function>(fun_node->llvm_allocation);

    // If the function is named `main`, set the linkage to external
    if (decl->name.lexeme == "main") {
        fun->setLinkage(llvm::Function::ExternalLinkage);
        fun->setName("main");
    }

    // Create the entry block
    Environment::inst().increase_local_scope();
    auto entry_block = llvm::BasicBlock::Create(*context, "entry", fun);
    auto exit_block = llvm::BasicBlock::Create(*context, "exit", fun);
    block_stack.push_back(exit_block);
    builder->SetInsertPoint(entry_block);

    // Handle the return variable
    if (decl->return_var != nullptr) {
        decl->return_var->accept(this);
    }

    auto llvm_arg_iter = fun->arg_begin();
    // Visit each parameter
    for (auto& param : decl->parameters) {
        param->accept(this);
        auto var_node = Environment::inst().get_variable({param->name});
        builder->CreateStore(&*llvm_arg_iter, llvm::cast<llvm::AllocaInst>(var_node->llvm_allocation));
        llvm_arg_iter++;
    }

    // Increase scope again
    Environment::inst().increase_local_scope();

    // Visit the body
    for (auto& stmt : decl->body) {
        stmt->accept(this);
    }

    // Exit the function
    builder->CreateBr(exit_block);
    builder->SetInsertPoint(exit_block);
    if (decl->return_var != nullptr) {

        // Get the variable node for the return variable
        auto var_node = Environment::inst().get_variable({decl->return_var->name});
        auto return_alloc = llvm::cast<llvm::AllocaInst>(var_node->llvm_allocation);
        var_node->decl->type->to_llvm_type(context);
        llvm::Value* return_value = builder->CreateLoad(var_node->decl->type->to_llvm_type(context), return_alloc);

        // If this is an aggregate type, we should load the value again
        auto aggregate_type = std::dynamic_pointer_cast<Type::Aggregate>(var_node->decl->type);
        if (aggregate_type != nullptr) {
            return_value = builder->CreateLoad(aggregate_type->to_llvm_aggregate_type(context), return_value);
        }

        builder->CreateRet(return_value);
    } else {
        builder->CreateRetVoid();
    }

    block_stack.clear();
    // Decrease scope
    Environment::inst().exit_scope();
    Environment::inst().exit_scope();

    return nullptr;
}

std::any CodeGenerator::visit_extern_fun_decl(Decl::ExternFun*) {
    // The function is already declared; we don't need to do anything here.
    return nullptr;
}

std::any CodeGenerator::visit_struct_decl(Decl::Struct* decl) {
    // This function should visit all static members of the struct.
    // Right now, that's just the declarations that are functions.

    Environment::inst().enter_scope(decl->name.lexeme);

    for (auto& declaration : decl->declarations) {
        if (IS_TYPE(declaration, Decl::Fun)) {
            declaration->accept(this);
        }
    }

    Environment::inst().exit_scope();

    return nullptr;
}

std::any CodeGenerator::visit_assign_expr(Expr::Assign* expr) {
    // Get the llvm allocation of the left side
    auto lvalue = std::dynamic_pointer_cast<Expr::LValue>(expr->left);
    // This should never be nullptr
    auto llvm_allocation = lvalue->get_llvm_allocation(this);

    // Get the value of the right side
    auto value = std::any_cast<llvm::Value*>(expr->right->accept(this));

    // Store the value in the llvm allocation
    builder->CreateStore(value, llvm_allocation);

    return value;
}

std::any CodeGenerator::visit_logical_expr(Expr::Logical*) {
    // TODO: Implement logical expressions
    return nullptr;
}

std::any CodeGenerator::visit_binary_expr(Expr::Binary* expr) {
    // There are 12 binary operators: `+`, `-`, `*`, `/`, `%`, `^`, `==`, `!=`, `<`, `<=`, `>`, `>=`
    // TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT, TOK_CARET, TOK_EQ_EQ, TOK_NE, TOK_LT, TOK_LE, TOK_GT, TOK_GE
    auto left_val = std::any_cast<llvm::Value*>(expr->left->accept(this));
    auto right_val = std::any_cast<llvm::Value*>(expr->right->accept(this));

    if (expr->op.tok_type == TOK_CARET) {
        // This is exponentiation, not bitwise XOR
        // Get the function; declare it if it doesn't exist
        auto pow_fun = ir_module->getFunction("pow");
        if (pow_fun == nullptr) {
            std::vector<llvm::Type*> pow_args = {llvm::Type::getDoubleTy(*context), llvm::Type::getDoubleTy(*context)};
            llvm::FunctionType* pow_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), pow_args, false);
            pow_fun = llvm::Function::Create(pow_type, llvm::Function::ExternalLinkage, "pow", ir_module.get());
        }
        // If the operands are ints, convert them to doubles
        if (left_val->getType()->isIntegerTy()) {
            left_val = builder->CreateSIToFP(left_val, llvm::Type::getDoubleTy(*context));
            right_val = builder->CreateSIToFP(right_val, llvm::Type::getDoubleTy(*context));
        }
        // Call the function
        std::vector<llvm::Value*> args = {left_val, right_val};
        llvm::Value* ret = builder->CreateCall(pow_fun, args);
        return ret;
    }

    if (expr->left->type->is_int()) {
        if (expr->op.tok_type == TOK_PLUS) {
            return builder->CreateAdd(left_val, right_val);
        } else if (expr->op.tok_type == TOK_MINUS) {
            return builder->CreateSub(left_val, right_val);
        } else if (expr->op.tok_type == TOK_STAR) {
            return builder->CreateMul(left_val, right_val);
        } else if (expr->op.tok_type == TOK_SLASH) {
            return builder->CreateSDiv(left_val, right_val);
        } else if (expr->op.tok_type == TOK_PERCENT) {
            return builder->CreateSRem(left_val, right_val);
        } else if (expr->op.tok_type == TOK_EQ_EQ) {
            return builder->CreateICmpEQ(left_val, right_val);
        } else if (expr->op.tok_type == TOK_BANG_EQ) {
            return builder->CreateICmpNE(left_val, right_val);
        } else if (expr->op.tok_type == TOK_LT) {
            return builder->CreateICmpSLT(left_val, right_val);
        } else if (expr->op.tok_type == TOK_LE) {
            return builder->CreateICmpSLE(left_val, right_val);
        } else if (expr->op.tok_type == TOK_GT) {
            return builder->CreateICmpSGT(left_val, right_val);
        } else if (expr->op.tok_type == TOK_GE) {
            return builder->CreateICmpSGE(left_val, right_val);
        }
    } else if (expr->left->type->is_float()) {
        if (expr->op.tok_type == TOK_PLUS) {
            return builder->CreateFAdd(left_val, right_val);
        } else if (expr->op.tok_type == TOK_MINUS) {
            return builder->CreateFSub(left_val, right_val);
        } else if (expr->op.tok_type == TOK_STAR) {
            return builder->CreateFMul(left_val, right_val);
        } else if (expr->op.tok_type == TOK_SLASH) {
            return builder->CreateFDiv(left_val, right_val);
        } else if (expr->op.tok_type == TOK_EQ_EQ) {
            return builder->CreateFCmpOEQ(left_val, right_val);
        } else if (expr->op.tok_type == TOK_BANG_EQ) {
            return builder->CreateFCmpONE(left_val, right_val);
        } else if (expr->op.tok_type == TOK_LT) {
            return builder->CreateFCmpOLT(left_val, right_val);
        } else if (expr->op.tok_type == TOK_LE) {
            return builder->CreateFCmpOLE(left_val, right_val);
        } else if (expr->op.tok_type == TOK_GT) {
            return builder->CreateFCmpOGT(left_val, right_val);
        } else if (expr->op.tok_type == TOK_GE) {
            return builder->CreateFCmpOGE(left_val, right_val);
        }
    }

    ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Code generator could not perform binary operation.");
    throw CodeGenException();
}

std::any CodeGenerator::visit_unary_expr(Expr::Unary* expr) {
    auto right_val = std::any_cast<llvm::Value*>(expr->inner->accept(this));
    if (expr->op.tok_type == TOK_BANG) {
        return builder->CreateICmpEQ(right_val, llvm::ConstantInt::get(right_val->getType(), 0));
    } else if (expr->op.tok_type == TOK_MINUS) {
        return builder->CreateNeg(right_val);
    } else if (expr->op.tok_type == TOK_AMP) {
        auto right_lvalue = std::dynamic_pointer_cast<Expr::LValue>(expr->inner);
        // This should never be nullptr
        return right_lvalue->get_llvm_allocation(this);
    } else {
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Code generator could not perform unary operation.");
        throw CodeGenException();
    }
}

std::any CodeGenerator::visit_dereference_expr(Expr::Dereference* expr) {
    auto right_val = std::any_cast<llvm::Value*>(expr->inner->accept(this));
    std::shared_ptr<Type> right_type = expr->inner->type;
    auto right_ptr_type = std::dynamic_pointer_cast<Type::Pointer>(right_type);
    // This should never be nullptr
    auto right_inner_type = right_ptr_type->inner_type;

    llvm::Value* ret = builder->CreateLoad(right_inner_type->to_llvm_type(context), right_val);
    return ret;
}

std::any CodeGenerator::visit_access_expr(Expr::Access* expr) {
    // First visit the left side of the access expression
    auto struct_alloca = std::any_cast<llvm::Value*>(expr->left->accept(this));
    // This is a pointer to the struct

    auto struct_type = std::dynamic_pointer_cast<Type::Struct>(expr->left->type);
    // This should never be nullptr

    // Check if this is a struct member
    int index = struct_type->struct_scope->instance_members.get_index(expr->ident.lexeme);
    if (index != -1) {
        // Create a GEP instruction to get the member
        auto gep = builder->CreateStructGEP(struct_type->to_llvm_aggregate_type(context), struct_alloca, index);
        llvm::Value* ret = builder->CreateLoad(expr->type->to_llvm_type(context), gep);
        return ret;
    }

    // Else, we are accessing a static member
    auto node = struct_type->struct_scope->children.at(expr->ident.lexeme);
    // This should never throw
    auto var_node = std::dynamic_pointer_cast<Node::Variable>(node);
    // This should never be nullptr

    // If var_node is a function, the llvm_allocation is the function itself
    if (var_node->decl->type->kind() == Type::Kind::FUNCTION) {
        return var_node->llvm_allocation;
    }
    // Load the value from the variable
    llvm::Value* ret = builder->CreateLoad(expr->type->to_llvm_type(context), var_node->llvm_allocation);
    return ret;
}

std::any CodeGenerator::visit_index_expr(Expr::Index* expr) {
    auto tuple_type = std::dynamic_pointer_cast<Type::Tuple>(expr->left->type);
    if (tuple_type != nullptr) {
        auto tuple_alloca = std::any_cast<llvm::Value*>(expr->left->accept(this));

        auto literal_right = std::dynamic_pointer_cast<Expr::Literal>(expr->right);
        // This should never be nullptr
        auto index = std::any_cast<int>(literal_right->token.literal);

        llvm::Value* val = builder->CreateStructGEP(tuple_type->to_llvm_aggregate_type(context), tuple_alloca, index);
        llvm::Value* ret = builder->CreateLoad(tuple_type->element_types[index]->to_llvm_type(context), val);

        return ret;
    }

    auto array_type = std::dynamic_pointer_cast<Type::Array>(expr->left->type);
    if (array_type != nullptr) {
        auto array_alloca = std::any_cast<llvm::Value*>(expr->left->accept(this));
        auto index_value = std::any_cast<llvm::Value*>(expr->right->accept(this));

        llvm::Value* val = builder->CreateGEP(
            array_type->to_llvm_aggregate_type(context),
            array_alloca,
            {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), index_value}
        );
        llvm::Value* ret = builder->CreateLoad(array_type->inner_type->to_llvm_type(context), val);
        return ret;
    }

    ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Code generator could not perform index operation.");
    throw CodeGenException();
}

std::any CodeGenerator::visit_call_expr(Expr::Call* expr) {
    llvm::Value* ret;
    // Get the function
    auto fun = llvm::cast<llvm::Function>(std::any_cast<llvm::Value*>(expr->callee->accept(this)));
    // Get the arguments
    std::vector<llvm::Value*> args;
    for (auto& arg : expr->arguments) {
        args.push_back(std::any_cast<llvm::Value*>(arg->accept(this)));
    }
    // Call the function
    ret = builder->CreateCall(fun, args);

    if (fun->getReturnType()->isAggregateType()) {
        // If the function returns an aggregate type, we need to store the return value in an alloca instruction
        auto alloca = builder->CreateAlloca(fun->getReturnType());
        builder->CreateStore(ret, alloca);
        return (llvm::Value*)alloca;
    }

    return ret;
}

std::any CodeGenerator::visit_cast_expr(Expr::Cast* expr) {
    auto left_value = std::any_cast<llvm::Value*>(expr->expression->accept(this));
    auto left_type = expr->expression->type;
    auto target_type = expr->type;
    if (left_type->is_int() && target_type->is_int()) {
        return builder->CreateIntCast(left_value, target_type->to_llvm_type(context), true);
    } else if (left_type->is_float() && target_type->is_float()) {
        return builder->CreateFPCast(left_value, target_type->to_llvm_type(context));
    } else if (left_type->is_int() && target_type->is_float()) {
        return builder->CreateSIToFP(left_value, target_type->to_llvm_type(context));
    } else if (left_type->is_float() && target_type->is_int()) {
        return builder->CreateFPToSI(left_value, target_type->to_llvm_type(context));
    } else if ((left_type->is_numeric() || left_type->kind() == Type::Kind::POINTER) && target_type->to_string() == "::bool") {
        return builder->CreateICmpNE(left_value, llvm::Constant::getNullValue(left_value->getType()));
    } else {
        ErrorLogger::inst().log_error(expr->location, E_UNREACHABLE, "Code generator could not perform cast from " + left_type->to_string() + " to " + target_type->to_string() + ".");
        throw CodeGenException();
    }
}

std::any CodeGenerator::visit_grouping_expr(Expr::Grouping* expr) {
    return std::any_cast<llvm::Value*>(expr->expression->accept(this));
}

std::any CodeGenerator::visit_identifier_expr(Expr::Identifier* expr) {
    // Get the variable node
    auto var_node = Environment::inst().get_variable(expr->tokens);
    // This should never be nullptr

    // If var_node is a function, the llvm_allocation is the function itself
    if (var_node->decl->type->kind() == Type::Kind::FUNCTION) {
        return var_node->llvm_allocation;
    }

    // Load the value from the variable
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(var_node->llvm_allocation);
    llvm::GlobalVariable* global = llvm::dyn_cast<llvm::GlobalVariable>(var_node->llvm_allocation);
    if (alloca == nullptr && global == nullptr) {
        ErrorLogger::inst().log_error(expr->location, E_IMPOSSIBLE, "Variable allocation is neither alloca nor global.");
        throw CodeGenException();
    }
    llvm::Type* type = alloca != nullptr ? alloca->getAllocatedType() : global->getValueType();

    llvm::Value* ret = builder->CreateLoad(type, var_node->llvm_allocation);
    return ret;
}

std::any CodeGenerator::visit_literal_expr(Expr::Literal* expr) {
    llvm::Value* ret;

    if (expr->token.tok_type == TOK_NIL) {
        ret = llvm::Constant::getNullValue(llvm::PointerType::getUnqual(*context));
    } else if (expr->token.tok_type == TOK_BOOL) {
        ret = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), expr->token.lexeme == "true", false);
    } else if (expr->token.tok_type == TOK_INT) {
        auto value = std::any_cast<int>(expr->token.literal);
        ret = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), value, true);
    } else if (expr->token.tok_type == TOK_FLOAT) {
        auto value = std::any_cast<double>(expr->token.literal);
        ret = llvm::ConstantFP::get(*context, llvm::APFloat(value));
    } else if (expr->token.tok_type == TOK_CHAR) {
        auto value = std::any_cast<char>(expr->token.literal);
        ret = llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), value, false);
    } else if (expr->token.tok_type == TOK_STR) {
        auto value = std::any_cast<std::string>(expr->token.literal);
        ret = builder->CreateGlobalStringPtr(value);
    } else {
        ErrorLogger::inst().log_error(expr->location, E_IMPOSSIBLE, "Unknown literal type.");
        throw CodeGenException();
    }

    return ret;
}

std::any CodeGenerator::visit_array_expr(Expr::Array* expr) {
    auto array_type = std::dynamic_pointer_cast<Type::Array>(expr->type);

    // Allocate space for the array
    auto llvm_array_type = array_type->to_llvm_aggregate_type(context);
    auto array_alloca = builder->CreateAlloca(llvm_array_type);

    unsigned i = 0;
    for (auto& val_expr : expr->elements) {
        auto member_value = std::any_cast<llvm::Value*>(val_expr->accept(this));
        auto member_alloc = builder->CreateGEP(
            llvm_array_type,
            array_alloca,
            {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0),
             llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i)}

        );
        builder->CreateStore(member_value, member_alloc);

        i++;
    }

    return (llvm::Value*)array_alloca;
}

std::any CodeGenerator::visit_array_gen_expr(Expr::ArrayGen* expr) {
    auto array_type = std::dynamic_pointer_cast<Type::Array>(expr->type);

    // Allocate space for the array
    auto llvm_array_type = array_type->to_llvm_aggregate_type(context);
    auto array_alloca = builder->CreateAlloca(llvm_array_type);

    llvm::Value* start_val = llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true));
    llvm::AllocaInst* counter = builder->CreateAlloca(llvm::Type::getInt32Ty(*context), nullptr, "loop_counter");
    builder->CreateStore(start_val, counter);

    llvm::Function* current_fun = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* start_arraygen = llvm::BasicBlock::Create(*context, "start_arraygen", current_fun);
    llvm::BasicBlock* loop_arraygen = llvm::BasicBlock::Create(*context, "loop_arraygen", current_fun);
    llvm::BasicBlock* end_arraygen = llvm::BasicBlock::Create(*context, "end_arraygen", current_fun);
    builder->CreateBr(start_arraygen);

    // Start block: checks the loop condition
    builder->SetInsertPoint(start_arraygen);
    llvm::Value* counter_val = builder->CreateLoad(llvm::Type::getInt32Ty(*context), counter);
    llvm::Value* cond = builder->CreateICmpSLT(counter_val, llvm::ConstantInt::get(*context, llvm::APInt(32, array_type->size, true)));
    builder->CreateCondBr(cond, loop_arraygen, end_arraygen);

    // Loop block: runs the loop iteration
    builder->SetInsertPoint(loop_arraygen);
    llvm::Value* index = builder->CreateLoad(llvm::Type::getInt32Ty(*context), counter);
    llvm::Value* value = std::any_cast<llvm::Value*>(expr->generator->accept(this));
    llvm::Value* member_alloc = builder->CreateGEP(
        llvm_array_type,
        array_alloca,
        {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), index}
    );
    builder->CreateStore(value, member_alloc);
    llvm::Value* new_counter = builder->CreateAdd(counter_val, llvm::ConstantInt::get(*context, llvm::APInt(32, 1, true)));
    builder->CreateStore(new_counter, counter);
    builder->CreateBr(start_arraygen);

    // End block: continues after the loop
    builder->SetInsertPoint(end_arraygen);

    return (llvm::Value*)array_alloca;
}

std::any CodeGenerator::visit_tuple_expr(Expr::Tuple* expr) {
    auto tuple_type = std::dynamic_pointer_cast<Type::Tuple>(expr->type);

    // Allocate space for the tuple
    auto llvm_tuple_type = tuple_type->to_llvm_aggregate_type(context);
    auto tuple_alloca = builder->CreateAlloca(llvm_tuple_type);
    // An llvm tuple is actually a struct
    // Structs in llvm do not have member names
    // Members are accessed by index, which is what we do for tuples anyway

    unsigned i = 0;
    for (auto& val_expr : expr->elements) {
        auto member_value = std::any_cast<llvm::Value*>(val_expr->accept(this));
        auto member_alloc = builder->CreateStructGEP(llvm_tuple_type, tuple_alloca, i);
        builder->CreateStore(member_value, member_alloc);

        i++;
    }

    return (llvm::Value*)tuple_alloca;
}

std::any CodeGenerator::visit_object_expr(Expr::Object* expr) {
    auto struct_type = std::dynamic_pointer_cast<Type::Struct>(expr->type);
    // This should never be nullptr

    // Create the struct
    auto llvm_struct_type = struct_type->to_llvm_aggregate_type(context);
    auto struct_alloca = builder->CreateAlloca(llvm_struct_type);

    unsigned i = 0;
    for (auto& [name, val_expr] : expr->fields) {
        // Add the value to the struct
        auto member_value = std::any_cast<llvm::Value*>(val_expr->accept(this));
        auto member_alloc = builder->CreateStructGEP(llvm_struct_type, struct_alloca, i);
        builder->CreateStore(member_value, member_alloc);

        i++;
    }

    return (llvm::Value*)struct_alloca;
}

CodeGenerator::CodeGenerator() {
    context = Environment::inst().get_llvm_context();
    builder = std::make_shared<llvm::IRBuilder<>>(*context);
    ir_module = std::make_shared<llvm::Module>("main", *context);
}

std::shared_ptr<llvm::Module> CodeGenerator::generate(std::vector<std::shared_ptr<Stmt>> stmts, const std::string& ir_target_destination) {
    declare_all_structs();
    declare_all_functions();
    try {
        // Visit each statement
        for (auto& stmt : stmts) {
            stmt->accept(this);
        }
    } catch (const CodeGenException&) {
        return nullptr;
    } catch (const std::bad_any_cast&) {
        ErrorLogger::inst().log_error(E_ANY_CAST, "Bad any cast in code generation.");
        return nullptr;
    } catch (const std::exception& e) {
        ErrorLogger::inst().log_error(E_UNKNOWN, e.what());
        return nullptr;
    }

    if (ir_target_destination != "") {
        this->dump_ir(ir_target_destination);
    }

    // Verify the module
    if (llvm::verifyModule(*ir_module, &llvm::errs())) {
        ErrorLogger::inst().log_error(E_UNVERIFIED_MODULE, "The generated module could not be verified.");
        return nullptr;
    }

    return ir_module;
}

void CodeGenerator::dump_ir(const std::string& filename) {

    std::error_code ec;
    llvm::raw_fd_ostream ir_stream(filename, ec, llvm::sys::fs::OF_Text);

    if (ec) {
        ErrorLogger::inst().log_error(E_IO, std::string("Could not dump IR to file: ") + ec.message());
        return;
    }

    ir_module->print(ir_stream, nullptr);
}
