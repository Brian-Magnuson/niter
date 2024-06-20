#include "core.h"
#include "type.h"

bool Type::are_compatible(std::shared_ptr<Type>& a, std::shared_ptr<Type>& b) {

    if (a->kind() == b->kind()) {

        /*
        For arrays, check if the sizes are compatible.
        For a := b:
        - If both sizes are known, they must be equal.
        - If both sizes are unknown, they are compatible.
        - If `a` has an unknown size, then `b` may have any size.
        - If `b` has an unknown size, then `a` must also have an unknown size.
        */
        if (a->kind() == Type::Kind::ARRAY) {
            auto a_array = std::dynamic_pointer_cast<Type::Array>(a);
            auto b_array = std::dynamic_pointer_cast<Type::Array>(b);
            if (a_array->size == b_array->size) {
                return true;
            }
            if (a_array->size == -1) {
                // If a has an unknown size, b may have any size.
                return true;
                // We're allowed to "forget" the size of an array.
            }
            if (b_array->size == -1) {
                // If b has an unknown size, a must also have an unknown size.
                return a_array->size == -1;
                // We can't suddenly "remember" the size of an array.
            }
        }

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
