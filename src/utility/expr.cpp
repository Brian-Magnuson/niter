#include "expr.h"

#include "../checker/environment.h"
#include "../codegen/code_generator.h"

TokenType Expr::Dereference::get_lvalue_declarer() {
    auto ptr_type = std::dynamic_pointer_cast<Type::Pointer>(right->type);
    return ptr_type->declarer;
}

llvm::Value* Expr::Dereference::get_llvm_allocation(CodeGenerator* code_generator) {
    // If this is a dereferenced pointer...
    auto value = std::any_cast<llvm::Value*>(right->accept(code_generator));
    // Then the pointer is the llvm allocation.
    return value;
}

TokenType Expr::LAccess::get_lvalue_declarer() {
    // An access expression is const if any part of it is const.
    // This should never be nullptr
    if (left_lvalue->get_lvalue_declarer() == KW_CONST) {
        return KW_CONST;
    }
    // If the left side is not const, then the declarer is the declarer of the right side.
    auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    auto member_name = ident.lexeme;
    auto decl = Environment::inst().get_instance_variable(l_struct_type, member_name);
    return decl->declarer;
}

llvm::Value* Expr::LAccess::get_llvm_allocation(CodeGenerator* code_generator) {
    // Get the llvm allocation of the left side
    auto l_llvm_allocation = left_lvalue->get_llvm_allocation(code_generator);
    // This is the address of the struct

    // Get the index of the member in the struct
    auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    auto l_llvm_type = l_struct_type->struct_scope->ir_type;
    auto index = l_struct_type->struct_scope->instance_members.get_index(ident.lexeme);

    // Create a GEP instruction to get the member
    auto gep = code_generator->builder->CreateStructGEP(l_llvm_type, l_llvm_allocation, index);
    // We use GEP to calculate the address of the member
    return gep;
}

TokenType Expr::LIndex::get_lvalue_declarer() {
    // An access expression is const if the left side is const.
    return left_lvalue->get_lvalue_declarer();
}

llvm::Value* Expr::LIndex::get_llvm_allocation(CodeGenerator* code_generator) {
    // Currently only tuples are supported
    auto tuple_type = std::dynamic_pointer_cast<Type::Tuple>(left->type);
    if (tuple_type != nullptr) {

        // Get the llvm allocation of the left side
        auto l_llvm_allocation = left_lvalue->get_llvm_allocation(code_generator);
        // This is the address of the tuple

        // Get the index of the member in the tuple
        auto literal_right = std::dynamic_pointer_cast<Expr::Literal>(right);
        auto index = std::any_cast<int>(literal_right->token.literal);

        auto context = Environment::inst().get_llvm_context();

        // Create a GEP instruction to get the member
        auto gep = code_generator->builder->CreateStructGEP(tuple_type->to_llvm_type(context), l_llvm_allocation, index);
        // We use GEP to calculate the address of the member
        return gep;
    }
    return nullptr;
}

TokenType Expr::Identifier::get_lvalue_declarer() {
    auto var_node = Environment::inst().get_variable(tokens);
    return var_node->decl->declarer;
}

llvm::Value* Expr::Identifier::get_llvm_allocation(CodeGenerator* /* code_generator */) {
    auto var_node = Environment::inst().get_variable(tokens);
    return var_node->llvm_allocation;
}
