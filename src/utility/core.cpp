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

bool Type::is_aggregate() {
    // FIXME: We have to do an extra check for structs because currently, primitives and non-primitives are stored as structs, meaning not all structs are aggregates.
    if (auto struct_type = dynamic_cast<Named*>(this)) {
        return !struct_type->struct_scope->is_primitive;
    }
    return kind() == Type::Kind::ARRAY || kind() == Type::Kind::TUPLE;
}
