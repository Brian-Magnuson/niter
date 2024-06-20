#include "expr.h"

#include "../checker/environment.h"
#include "../codegen/code_generator.h"

TokenType Expr::Dereference::get_lvalue_declarer() {
    auto ptr_type = std::dynamic_pointer_cast<Type::Pointer>(inner->type);
    return ptr_type->declarer;
}

llvm::Value* Expr::Dereference::get_llvm_allocation(CodeGenerator* code_generator) {
    // If this is a dereferenced pointer...
    auto value = std::any_cast<llvm::Value*>(inner->accept(code_generator));
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
    auto l_struct_type = std::dynamic_pointer_cast<Type::Named>(left->type);
    auto member_name = ident.lexeme;
    auto decl = Environment::inst().get_instance_variable(l_struct_type, member_name);
    return decl->declarer;
}

llvm::Value* Expr::LAccess::get_llvm_allocation(CodeGenerator* code_generator) {
    // // Get the llvm allocation of the left side
    // auto l_llvm_allocation = left_lvalue->get_llvm_allocation(code_generator);
    // // This is the address of the struct

    // // Get the index of the member in the struct
    // auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    // auto context = Environment::inst().get_llvm_context();

    // auto l_llvm_type = l_struct_type->to_llvm_aggregate_type(context);
    // auto index = l_struct_type->struct_scope->instance_members.get_index(ident.lexeme);

    // // Create a GEP instruction to get the member
    // auto gep = code_generator->builder->CreateStructGEP(l_llvm_type, l_llvm_allocation, index);
    // // We use GEP to calculate the address of the member
    // return gep;

    auto struct_alloca = std::any_cast<llvm::Value*>(left->accept(code_generator));
    auto struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);

    auto member_name = ident.lexeme;
    auto member_index = struct_type->struct_scope->instance_members.get_index(member_name);

    auto context = Environment::inst().get_llvm_context();

    // Create a GEP instruction to get the member
    llvm::Value* val = code_generator->builder->CreateStructGEP(
        struct_type->to_llvm_aggregate_type(context),
        struct_alloca,
        member_index
    );
    return val;
}

TokenType Expr::LIndex::get_lvalue_declarer() {
    // An access expression is const if the left side is const.
    return left_lvalue->get_lvalue_declarer();
}

llvm::Value* Expr::LIndex::get_llvm_allocation(CodeGenerator* code_generator) {
    auto tuple_type = std::dynamic_pointer_cast<Type::Tuple>(left->type);
    if (tuple_type != nullptr) {
        // Get the tuple alloca
        auto tuple_alloca = std::any_cast<llvm::Value*>(left->accept(code_generator));
        // Get the index
        auto literal_right = std::dynamic_pointer_cast<Expr::Literal>(right);
        // This should never be nullptr
        auto index = std::any_cast<int>(literal_right->token.literal);

        auto context = Environment::inst().get_llvm_context();
        // Create a GEP instruction to get the member
        llvm::Value* val = code_generator->builder->CreateStructGEP(
            tuple_type->to_llvm_aggregate_type(context),
            tuple_alloca,
            index
        );
        return val;
        // It's pretty much like visiting an index expression, but we don't add the load instruction
    }
    auto array_type = std::dynamic_pointer_cast<Type::Array>(left->type);
    if (array_type != nullptr) {
        // Get the array alloca
        auto array_alloca = std::any_cast<llvm::Value*>(left->accept(code_generator));
        // Get the index
        auto index_value = std::any_cast<llvm::Value*>(right->accept(code_generator));

        auto context = Environment::inst().get_llvm_context();
        // Create a GEP instruction to get the member
        llvm::Value* val = code_generator->builder->CreateGEP(
            array_type->to_llvm_aggregate_type(context),
            array_alloca,
            {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), index_value}
        );
        return val;
        // It's pretty much like visiting an index expression, but we don't add the load instruction
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
