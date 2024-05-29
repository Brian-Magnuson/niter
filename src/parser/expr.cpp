#include "expr.h"

#include "../checker/environment.h"

TokenType Expr::Dereference::get_lvalue_declarer() {
    auto ptr_type = std::dynamic_pointer_cast<Type::Pointer>(right->type);
    return ptr_type->declarer;
}

TokenType Expr::Access::get_lvalue_declarer() {
    auto l_struct_type = std::dynamic_pointer_cast<Type::Struct>(left->type);
    auto member_name = std::dynamic_pointer_cast<Expr::Identifier>(right)->to_string();
    auto var_node = Environment::inst().get_instance_variable(l_struct_type, member_name);
    return var_node->declarer;
}

TokenType Expr::Identifier::get_lvalue_declarer() {
    auto var_node = Environment::inst().get_variable(tokens);
    return var_node->declarer;
}
