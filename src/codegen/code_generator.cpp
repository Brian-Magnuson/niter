#include "code_generator.h"
#include "../checker/environment.h"
#include "../checker/node.h"
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

std::any CodeGenerator::visit_var_decl(Decl::Var* decl) {

    // Get the variable node from the environment tree
    auto [node, _] =
        Environment::inst().declare_variable(decl->location, decl->name.lexeme, decl->declarer, decl->type_annotation);
    // We are using the declare_variable function because it will get the node if it already exists in the tree and will create it if it doesn't.
    auto var_node = std::dynamic_pointer_cast<Node::Variable>(node);
    // This should never be nullptr

    auto llvm_safe_name = var_node->unique_name;
    std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

    // Create the alloca instruction for the variable.
    llvm::AllocaInst* alloca = builder->CreateAlloca(var_node->type->to_llvm_type(context), nullptr, llvm_safe_name);

    // Generate code for the initializer expression.
    if (decl->initializer != nullptr) {
        auto initializer = std::any_cast<llvm::Value*>(decl->initializer->accept(this));
        // Store the value in the alloca instruction.
        builder->CreateStore(initializer, alloca);
    } else {
        // If there is no initializer, store the default value.
        llvm::Value* default_value = llvm::Constant::getNullValue(var_node->type->to_llvm_type(context));
        builder->CreateStore(default_value, alloca);
    }

    // Save the alloca instruction in the variable node.
    var_node->alloca = alloca;

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
    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(*context, "entry", fun);
    builder->SetInsertPoint(entry_block);
    Environment::inst().increase_local_scope();

    // Visit each parameter
    for (auto& param : decl->parameters) {
        param->accept(this);
    }

    // FIXME: Consider creating variable decl node for the return type on the AST so we can generate code for it.

    // Increase scope again
    Environment::inst().increase_local_scope();

    // Visit the body
    for (auto& stmt : decl->body) {
        stmt->accept(this);
    }

    // Decrease scope
    Environment::inst().exit_scope();
    Environment::inst().exit_scope();

    // TODO: Figure out what else to do here.

    return nullptr;
}

CodeGenerator::CodeGenerator() {
    context = Environment::inst().get_llvm_context();
    builder = std::make_shared<llvm::IRBuilder<>>(*context);
    ir_module = std::make_shared<llvm::Module>("main", *context);
    declare_all_structs();
}
