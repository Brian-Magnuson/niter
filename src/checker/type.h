#ifndef TYPE_H
#define TYPE_H

class Type;
// Forward declaration of Type to avoid circular dependency with Node

#include "../scanner/token.h"
#include "node.h"
#include <memory>
#include <string>

enum class TypeKind {
    STRUCT,
    FUNCTION,
    ARRAY,
    POINTER,
    TUPLE,
    BLANK
};

/**
 * @brief A base class representing a type.
 * Unlike annotations, types store pointers to the nodes in the namespace tree.
 * This results in the namespace tree being the single source of truth for types.
 * Note: This class is currently experimental.
 *
 */
class Type {
public:
    class Struct;
    class Function;
    class Array;
    class Pointer;
    class Tuple;
    class Blank;

    virtual ~Type() = default;
    virtual TypeKind kind() const = 0;
    virtual std::string to_string() const = 0;

    /**
     * @brief Determines if two types are compatible.
     * If one of the types is a blank type, that type will be mutated to match the other type.
     *
     * @param a A shared ptr reference to the first type. Should not be nullptr.
     * @param b A shared ptr reference to the second type. Should not be nullptr.
     * @return true If the types are compatible.
     * @return false If the types are not compatible.
     */
    static bool are_compatible(std::shared_ptr<Type>& a, std::shared_ptr<Type>& b) {
        if (a->kind() == b->kind()) {
            return a->to_string() == b->to_string();
        } else if (a->kind() == TypeKind::BLANK) {
            a = b;
            return true;
        } else if (b->kind() == TypeKind::BLANK) {
            b = a;
            return true;
        }
        return false;
    }

    bool is_int() {
        if (
            to_string() == "::i8" || to_string() == "::i16" || to_string() == "::i32" || to_string() == "::i64" || to_string() == "::char"
        ) {
            return true;
        }
        return false;
    }

    bool is_float() {
        if (
            to_string() == "::f32" || to_string() == "::f64"
        ) {
            return true;
        }
        return false;
    }
};

/**
 * @brief A class representing a struct type.
 * Struct types are the most basic type, pointing to a single struct node in the namespace tree.
 *
 */
class Type::Struct : public Type {
public:
    std::shared_ptr<Node::StructScope> struct_scope = nullptr;

    virtual ~Struct() = default;
    TypeKind kind() const override { return TypeKind::STRUCT; }
    std::string to_string() const override { return struct_scope->unique_name; }

    Struct(std::shared_ptr<Node::StructScope> struct_scope) : struct_scope(struct_scope) {}
};

/**
 * @brief A class representing a function type.
 * Function have multiple parameters and a return type.
 *
 */
class Type::Function : public Type {
public:
    std::vector<std::pair<TokenType, std::shared_ptr<Type>>> params;
    TokenType return_declarer;
    std::shared_ptr<Type> return_type = nullptr;

    virtual ~Function() = default;
    TypeKind kind() const override { return TypeKind::FUNCTION; }
    std::string to_string() const override {
        std::string str = "fun(";
        for (auto& param : params) {
            if (param.first == KW_VAR) {
                str += "var ";
            }
            str += param.second->to_string() + ", ";
        }
        str += ") => ";
        if (return_declarer == KW_VAR) {
            str += "var ";
        }
        str += return_type->to_string();
        return str;
    }

    Function(
        std::vector<std::pair<TokenType, std::shared_ptr<Type>>>& params,
        TokenType return_declarer,
        std::shared_ptr<Type> return_type
    )
        : params(params), return_declarer(return_declarer), return_type(return_type) {}
};

/**
 * @brief A class representing an array type.
 * Array types have a single element type and a size.
 *
 */
class Type::Array : public Type {
public:
    std::shared_ptr<Type> inner_type = nullptr;
    unsigned size;

    virtual ~Array() = default;
    TypeKind kind() const override { return TypeKind::ARRAY; }
    std::string to_string() const override { return inner_type->to_string() + "[" + std::to_string(size) + "]"; }

    Array(std::shared_ptr<Type> inner_type, unsigned size) : inner_type(inner_type), size(size) {}
};

/**
 * @brief A class representing a pointer type.
 * Pointer types have a single element type.
 *
 */
class Type::Pointer : public Type {
public:
    TokenType declarer = KW_CONST;
    std::shared_ptr<Type> inner_type = nullptr;

    virtual ~Pointer() = default;
    TypeKind kind() const override { return TypeKind::POINTER; }
    std::string to_string() const override { return inner_type->to_string() + "*"; }

    Pointer(std::shared_ptr<Type> inner_type) : inner_type(inner_type) {}
};

/**
 * @brief A class representing a tuple type.
 * Tuple types have multiple element types.
 *
 */
class Type::Tuple : public Type {
public:
    std::vector<std::shared_ptr<Type>> elements;

    virtual ~Tuple() = default;
    TypeKind kind() const override { return TypeKind::TUPLE; }
    std::string to_string() const override {
        std::string str = "(";
        for (auto& elem : elements) {
            str += elem->to_string() + ", ";
        }
        str += ")";
        return str;
    }

    Tuple(std::vector<std::shared_ptr<Type>>& elements) : elements(elements) {}
};

/**
 * @brief A class representing a blank type.
 * Blank types are placeholders for when a type is not known.
 * They do not point to any node in the namespace tree.
 *
 */
class Type::Blank : public Type {
public:
    virtual ~Blank() = default;
    TypeKind kind() const override { return TypeKind::BLANK; }
    std::string to_string() const override { return ""; }

    Blank() {}
};

#endif // TYPE_H
