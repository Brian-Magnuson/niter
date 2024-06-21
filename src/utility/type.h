#ifndef TYPE_H
#define TYPE_H

#include "../utility/core.h"

#include "../scanner/token.h"
#include "node.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief A base class for aggregate types.
 * This distinction is important because llvm aggregate types must be created using `alloca` instructions.
 * This means that their llvm type is a pointer to the actual type.
 *
 */
class Type::Aggregate : virtual public Type {
protected:
    Aggregate() = default;

public:
    /**
     * @brief Returns the true llvm type of the aggregate type.
     * For aggregate types, `to_llvm_type` will return a pointer to the actual type.
     * This is because aggregate types must be created using `alloca` instructions.
     * However, sometimes, the actual type is needed for operations like `getelementptr`.
     * This function will return the actual type.
     *
     * @param context The llvm context.
     * @return llvm::Type* The actual llvm type of the aggregate type.
     */
    virtual llvm::Type* to_llvm_aggregate_type(std::shared_ptr<llvm::LLVMContext> context) const = 0;
};

/**
 * @brief A class representing a named type.
 * Includes both primitive and non-primitive structs.
 * Named types are the most basic type, pointing to a single struct node in the namespace tree.
 *
 */
class Type::Named : virtual public Type {
public:
    // The node representing the struct in the namespace tree. Note: if two struct types are the same, they will point to the same node.
    std::shared_ptr<Node::StructScope> struct_scope = nullptr;

    virtual ~Named() = default;
    Type::Kind kind() const override { return Type::Kind::STRUCT; }
    std::string to_string() const override { return struct_scope->unique_name; }

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return struct_scope->ir_type;
    }

    Named(std::shared_ptr<Node::StructScope> struct_scope) : struct_scope(struct_scope) {}
};

/**
 * @brief A class representing a non-primitive struct type.
 * Non-primitive structs are structs that are not built-in to the language.
 * They are distinct from primitive Named types because they also inherit from Aggregate and may be treated differently by the code generator.
 *
 */
class Type::Struct : public Type::Named, public Type::Aggregate {
public:
    virtual ~Struct() = default;
    Type::Kind kind() const override { return Type::Kind::STRUCT; }

    Struct(std::shared_ptr<Node::StructScope> struct_scope) : Named(struct_scope) {}

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return llvm::PointerType::getUnqual(struct_scope->ir_type);
    }

    llvm::Type* to_llvm_aggregate_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return struct_scope->ir_type;
    }
};

/**
 * @brief A class representing a function type.
 * Function have multiple parameters and a return type.
 *
 */
class Type::Function : public Type {
public:
    // A list of pairs containing the parameter declarer and the parameter type.
    std::vector<std::pair<TokenType, std::shared_ptr<Type>>> params;
    // The return type declarer.
    TokenType return_declarer;
    // The return type.
    std::shared_ptr<Type> return_type = nullptr;
    // Whether the function is variadic. Currently only allowed for external functions.
    bool is_variadic = false;

    virtual ~Function() = default;
    Type::Kind kind() const override { return Type::Kind::FUNCTION; }
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

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        std::vector<llvm::Type*> param_types;
        for (auto& param : params) {
            param_types.push_back(param.second->to_llvm_type(context));
        }
        auto fun_type = llvm::FunctionType::get(return_type->to_llvm_type(context), param_types, is_variadic);
        return fun_type;
    }

    Function(
        std::vector<std::pair<TokenType, std::shared_ptr<Type>>>& params,
        TokenType return_declarer,
        std::shared_ptr<Type> return_type,
        bool is_variadic = false
    ) : params(params), return_declarer(return_declarer), return_type(return_type), is_variadic(is_variadic) {}
};

/**
 * @brief A class representing an array type.
 * Array types have a single element type and a size.
 *
 */
class Type::Array : public Type::Aggregate {
public:
    // The element type of the array.
    std::shared_ptr<Type> inner_type = nullptr;
    // The size of the array. -1 if the size is not known.
    int size = -1;

    virtual ~Array() = default;
    Type::Kind kind() const override { return Type::Kind::ARRAY; }
    std::string to_string() const override {
        std::string size_string = size == -1 ? "*" : std::to_string(size);
        return "[" + inner_type->to_string() + "; " + size_string + "]";
    }

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return llvm::PointerType::getUnqual(to_llvm_aggregate_type(context));
    }

    llvm::Type* to_llvm_aggregate_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return llvm::ArrayType::get(inner_type->to_llvm_type(context), size);
    }

    Array(std::shared_ptr<Type> inner_type, int size) : inner_type(inner_type), size(size) {}
};

/**
 * @brief A class representing a pointer type.
 * Pointer types have a single element type.
 *
 */
class Type::Pointer : public Type {
public:
    // The declarer of the pointer type. Use to determine if the object can be mutated through this pointer. Default is KW_VAR.
    TokenType declarer = KW_VAR;

    // The element type of the pointer.
    std::shared_ptr<Type> inner_type = nullptr;

    virtual ~Pointer() = default;
    Type::Kind kind() const override { return Type::Kind::POINTER; }
    std::string to_string() const override { return inner_type->to_string() + "*"; }

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return llvm::PointerType::get(inner_type->to_llvm_type(context), 0);
    }

    Pointer(std::shared_ptr<Type> inner_type) : inner_type(inner_type) {}
};

/**
 * @brief A class representing a tuple type.
 * Tuple types have multiple element types.
 *
 */
class Type::Tuple : public Type::Aggregate {
public:
    // A list of element types in the tuple.
    std::vector<std::shared_ptr<Type>> element_types;

    virtual ~Tuple() = default;
    Type::Kind kind() const override { return Type::Kind::TUPLE; }
    std::string to_string() const override {
        std::string str = "(";
        for (auto& elem : element_types) {
            str += elem->to_string() + ", ";
        }
        str += ")";
        return str;
    }

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        return llvm::PointerType::getUnqual(to_llvm_aggregate_type(context));
    }

    llvm::Type* to_llvm_aggregate_type(std::shared_ptr<llvm::LLVMContext> context) const override {
        std::vector<llvm::Type*> elem_llvm_types;
        for (auto& elem : element_types) {
            elem_llvm_types.push_back(elem->to_llvm_type(context));
        }
        return llvm::StructType::get(*context, elem_llvm_types);
    }

    Tuple(std::vector<std::shared_ptr<Type>>& elements) : element_types(elements) {}
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
    Type::Kind kind() const override { return Type::Kind::BLANK; }
    std::string to_string() const override { return ""; }

    llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const override { return nullptr; }

    Blank() {}
};

#endif // TYPE_H
