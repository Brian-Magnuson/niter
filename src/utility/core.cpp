#include "core.h"
#include "type.h"

ErrorCode Type::are_compatible(std::shared_ptr<Type>& a, std::shared_ptr<Type>& b) {

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
                return (ErrorCode)0;
            }
            if (a_array->size == -1) {
                // If a has an unknown size, b may have any size.
                return (ErrorCode)0;
                // We're allowed to "forget" the size of an array.
            }
            if (b_array->size == -1) {
                // If b has an unknown size, a must also have an unknown size.
                return a_array->size == -1 ? (ErrorCode)0 : E_ARRAY_SIZE_UNKNOWN;
                // We can't suddenly "remember" the size of an array.
            }
        }

        return a->to_string() == b->to_string() ? (ErrorCode)0 : E_INCOMPATIBLE_TYPES;
    }

    if (a->kind() == Type::Kind::BLANK) {
        // If b is an array of blank types, then they are not compatible.
        auto b_array = std::dynamic_pointer_cast<Type::Array>(b);
        if (b_array != nullptr && b_array->inner_type->kind() == Type::Kind::BLANK) {
            return E_INDETERMINATE_ARRAY_TYPE;
        }
        a = b;
        return (ErrorCode)0;
    }

    if (b->kind() == Type::Kind::BLANK) {
        // If a is an array of blank types, then they are not compatible.
        // Also, if a has a known size, then they are not compatible.
        auto a_array = std::dynamic_pointer_cast<Type::Array>(a);
        if (a_array != nullptr && a_array->inner_type->kind() == Type::Kind::BLANK) {
            return E_INDETERMINATE_ARRAY_TYPE;
        }
        if (a_array != nullptr && a_array->size != -1) {
            return E_SIZED_ARRAY_WITHOUT_INITIALIZER;
        }
        b = a;
        return (ErrorCode)0;
    }
    return E_INCOMPATIBLE_TYPES;
}
