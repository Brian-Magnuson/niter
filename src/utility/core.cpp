#include "core.h"
#include "type.h"

bool Type::are_compatible(std::shared_ptr<Type>& a, std::shared_ptr<Type>& b) {

    if (a->kind() == b->kind()) {
        return a->to_string() == b->to_string();
    }

    if (a->kind() == Type::Kind::BLANK) {
        // If b is an array of blank types, then they are not compatible.
        auto b_array = std::dynamic_pointer_cast<Type::Array>(b);
        if (b_array != nullptr && b_array->inner_type->kind() == Type::Kind::BLANK) {
            return false;
        }
        a = b;
        return true;
    }

    if (b->kind() == Type::Kind::BLANK) {
        // If a is an array of blank types, then they are not compatible.
        auto a_array = std::dynamic_pointer_cast<Type::Array>(a);
        if (a_array != nullptr && a_array->inner_type->kind() == Type::Kind::BLANK) {
            return false;
        }
        b = a;
        return true;
    }
    return false;
}
