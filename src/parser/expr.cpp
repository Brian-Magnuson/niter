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

TokenType Expr::Access::get_lvalue_declarer() {
    auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    auto member_name = std::dynamic_pointer_cast<Expr::Identifier>(right)->to_string();
    auto var_node = Environment::inst().get_instance_variable(l_struct_type, member_name);
    return var_node->declarer;
}

llvm::Value* Expr::Access::get_llvm_allocation(CodeGenerator* /* code_generator */) {
    auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    auto member_name = std::dynamic_pointer_cast<Expr::Identifier>(right)->to_string();
    auto var_node = Environment::inst().get_instance_variable(l_struct_type, member_name);
    return var_node->llvm_allocation;
}

TokenType Expr::Identifier::get_lvalue_declarer() {
    auto var_node = Environment::inst().get_variable(tokens);
    return var_node->declarer;
}

llvm::Value* Expr::Identifier::get_llvm_allocation(CodeGenerator* /* code_generator */) {
    auto var_node = Environment::inst().get_variable(tokens);
    return var_node->llvm_allocation;
}
