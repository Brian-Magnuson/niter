#include "code_generator.h"
#include "../checker/environment.h"
#include "../checker/node.h"
#include "../logger/logger.h"
#include "llvm/Support/Casting.h"

void CodeGenerator::declare_all_structs() {
    // First loop for opaque declarations
    auto root = Environment::inst().get_global_tree();
    std::vector<std::shared_ptr<Node::Scope>> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        // Pop the top scope from the stack
        auto node = stack.back();
        stack.pop_back();

        // If the node is a struct, declare it
        auto node_struct = std::dynamic_pointer_cast<Node::StructScope>(node);
        if (node_struct != nullptr && !node_struct->is_primitive) {
            // Create the struct type
            std::vector<llvm::Type*> member_types;
            for (auto& [name, member] : node_struct->instance_members) {
                member_types.push_back(member->type->to_llvm_type(context));
            }
            // The struct type is already created. Just set the body.
            auto llvm_struct_type = llvm::cast<llvm::StructType>(node_struct->ir_type);
            llvm_struct_type->setBody(member_types);
        }

        // Add the children to the stack
        for (auto& [_, child] : node->children) {
            auto child_scope = std::dynamic_pointer_cast<Node::Scope>(child);
            if (child_scope != nullptr) {
                stack.push_back(child_scope);
            }
        }
    }
}

std::any CodeGenerator::visit_declaration_stmt(Stmt::Declaration* stmt) {
    stmt->declaration->accept(this);
    return nullptr;
}

std::any CodeGenerator::visit_return_stmt(Stmt::Return* stmt) {
    if (stmt->value != nullptr) {
        auto value = std::any_cast<llvm::Value*>(stmt->value->accept(this));
        builder->CreateStore(value, Environment::inst().get_variable({"__return_val__"})->llvm_allocation);
    }
    if (block_stack.size() == 0) {
        ErrorLogger::inst().log_error(stmt->location, E_IMPOSSIBLE, "Return statement outside of function.");
        throw std::runtime_error("Return statement outside of function.");
    }
    builder->CreateBr(block_stack.front());
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
            initializer = llvm::Constant::getNullValue(var_node->type->to_llvm_type(context));
        }
        // This should never be nullptr
        auto llvm_safe_name = var_node->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        // Attempt to cast the initializer to an llvm constant
        auto constant_initializer = llvm::dyn_cast<llvm::Constant>(initializer);
        if (constant_initializer == nullptr) {
            ErrorLogger::inst().log_error(decl->location, E_NOT_A_CONSTANT, "Global variable initializer is not a constant.");
            throw std::runtime_error("Global variable initializer is not a constant.");
        }

        // Create the global variable
        llvm::GlobalVariable* global = new llvm::GlobalVariable(
            *ir_module,
            var_node->type->to_llvm_type(context),
            false,
            llvm::GlobalValue::InternalLinkage,
            constant_initializer,
            llvm_safe_name
        );
        var_node->llvm_allocation = global;
    } else {
        // For local variables, we need to create an alloca instruction.
        // The node does not exist in the tree, so we need to create it.
        auto [node, _] = Environment::inst().declare_variable(decl->location, decl->name.lexeme, decl->declarer, decl->type_annotation);
        auto var_node = std::dynamic_pointer_cast<Node::Variable>(node);
        // This should never be nullptr

        // Generate code for the initializer expression.
        llvm::Value* initializer = nullptr;
        if (decl->initializer != nullptr) {
            initializer = std::any_cast<llvm::Value*>(decl->initializer->accept(this));
        } else {
            // If there is no initializer, store the default value.
            initializer = llvm::Constant::getNullValue(var_node->type->to_llvm_type(context));
        }

        auto llvm_safe_name = var_node->unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        // Create the alloca instruction for the variable.
        llvm::AllocaInst* alloca = builder->CreateAlloca(var_node->type->to_llvm_type(context), nullptr, llvm_safe_name);

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

    // Create the function type
    llvm::FunctionType* fun_type = llvm::cast<llvm::FunctionType>(fun_node->type->to_llvm_type(context));

    auto llvm_safe_name = fun_node->unique_name;
    std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

    // Create the function
    llvm::Function* fun = llvm::Function::Create(fun_type, llvm::Function::InternalLinkage, llvm_safe_name, ir_module.get());

    // If the function is named `main`, set the linkage to external
    if (decl->name.lexeme == "main") {
        fun->setLinkage(llvm::Function::ExternalLinkage);
    }

    // Create the entry block
    Environment::inst().increase_local_scope();
    auto entry_block = llvm::BasicBlock::Create(*context, "entry", fun);
    auto exit_block = llvm::BasicBlock::Create(*context, "exit", fun);
    builder->SetInsertPoint(entry_block);

    // Handle the return variable
    if (decl->return_var != nullptr) {
        decl->return_var->accept(this);
    }
    // Visit each parameter
    for (auto& param : decl->parameters) {
        param->accept(this);
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
        auto return_alloc = llvm::cast<llvm::AllocaInst>(Environment::inst().get_variable({decl->return_var->name})->llvm_allocation);
        llvm::Value* return_value = builder->CreateLoad(return_alloc->getAllocatedType(), return_alloc);
        builder->CreateRet(return_value);
    } else {
        builder->CreateRetVoid();
    }

    // Decrease scope
    Environment::inst().exit_scope();
    Environment::inst().exit_scope();

    // Create a global variable for the function
    llvm::GlobalVariable* global = new llvm::GlobalVariable(
        *ir_module,
        fun->getType(),
        true,
        llvm::GlobalValue::InternalLinkage,
        fun,
        llvm_safe_name
    );
    fun_node->llvm_allocation = global;

    return nullptr;
}

CodeGenerator::CodeGenerator() {
    context = Environment::inst().get_llvm_context();
    builder = std::make_shared<llvm::IRBuilder<>>(*context);
    ir_module = std::make_shared<llvm::Module>("main", *context);
    declare_all_structs();
}
