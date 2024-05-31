#ifndef CORE_H
#define CORE_H

#include "../scanner/token.h"
#include "llvm/IR/Type.h"
#include <any>
#include <memory>
#include <string>

/**
 * @brief A base class representing a type.
 * Unlike annotations, types store pointers to the nodes in the namespace tree.
 * This results in the namespace tree being the single source of truth for types.
 *
 */
class Type {
public:
    /**
     * @brief An enum class representing the kind of the type.
     *
     */
    enum class Kind {
        // The basic kind of type; used for primitives and user-defined types.
        STRUCT,
        // A function type.
        FUNCTION,
        // An array type.
        ARRAY,
        // A pointer type.
        POINTER,
        // A tuple type.
        TUPLE,
        // A blank type; used for type inference.
        BLANK
    };

    class Struct;
    class Function;
    class Array;
    class Pointer;
    class Tuple;
    class Blank;

    virtual ~Type() = default;

    /**
     * @brief Get the kind of the type.
     *
     * @return Type::Kind An enum representing the kind of the type.
     */
    virtual Kind kind() const = 0;

    /**
     * @brief Get the string representation of the type.
     *
     * @return std::string The string representation of the type.
     */
    virtual std::string to_string() const = 0;

    /**
     * @brief Get the LLVM type representation of the type.
     *
     * @param context The LLVM context; can be obtained from the Environment singleton.
     * @return llvm::Type* The LLVM type representation of the type.
     */
    virtual llvm::Type* to_llvm_type(std::shared_ptr<llvm::LLVMContext> context) const = 0;

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
        } else if (a->kind() == Type::Kind::BLANK) {
            a = b;
            return true;
        } else if (b->kind() == Type::Kind::BLANK) {
            b = a;
            return true;
        }
        return false;
    }

    /**
     * @brief Checks if the type is a primitive integer type.
     * One of these: `::i8`, `::i16`, `::i32`, `::i64`, `::char`.
     * Uses the unique type name to determine if the type is an integer.
     *
     * @return true If the type is a primitive integer type.
     * @return false Otherwise.
     */
    bool is_int() {
        if (
            to_string() == "::i8" || to_string() == "::i16" || to_string() == "::i32" || to_string() == "::i64" || to_string() == "::char"
        ) {
            return true;
        }
        return false;
    }

    /**
     * @brief Checks if the type is a primitive floating point type.
     * One of these: `::f32`, `::f64`.
     * Uses the unique type name to determine if the type is a floating point type.
     *
     * @return true If the type is a primitive floating point type.
     * @return false Otherwise.
     */
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
 * @brief An abstract base class for all statements.
 * Includes statements for expressions and declarations.
 *
 */
class Stmt {
public:
    class Declaration;
    class Expression;
    class Block;
    class Conditional;
    class Loop;
    class Return;
    class Break;
    class Continue;
    class EndOfFile;

    virtual ~Stmt() {}

    // The location of the statement. Useful for error messages.
    Location location;

    /**
     * @brief A visitor class for statements.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_declaration_stmt(Declaration* stmt) = 0;
        virtual std::any visit_expression_stmt(Expression* stmt) = 0;
        virtual std::any visit_block_stmt(Block* stmt) = 0;
        virtual std::any visit_conditional_stmt(Conditional* stmt) = 0;
        virtual std::any visit_loop_stmt(Loop* stmt) = 0;
        virtual std::any visit_return_stmt(Return* stmt) = 0;
        virtual std::any visit_break_stmt(Break* stmt) = 0;
        virtual std::any visit_continue_stmt(Continue* stmt) = 0;
        virtual std::any visit_eof_stmt(EndOfFile* stmt) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief An abstract base class for all declarations.
 *
 */
class Decl {
public:
    class VarDeclarable;

    class Var;
    class Fun;
    class ExternFun;
    class Struct;

    virtual ~Decl() {}

    // The location of the declaration. Useful for error messages.
    Location location;

    /**
     * @brief A visitor class for declarations.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_var_decl(Var* decl) = 0;
        virtual std::any visit_fun_decl(Fun* decl) = 0;
        virtual std::any visit_extern_fun_decl(ExternFun* decl) = 0;
        virtual std::any visit_struct_decl(Struct* decl) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief An abstract base class for all expressions.
 *
 */
class Expr {
public:
    class Locatable;

    class Assign;
    class Logical;
    class Binary;
    class Unary;
    class Dereference;
    class Call;
    class Access;
    class Grouping;
    class Identifier;
    class Literal;
    class Array;
    class Tuple;

    // An annotation representing the type of the expression. Set to nullptr, to be filled in by the type checker.
    // std::shared_ptr<Annotation> type_annotation = nullptr;

    // The type of the expression. Set to nullptr, to be filled in by the type checker.
    std::shared_ptr<Type> type = nullptr;

    // A location useful for error messages.
    Location location;

    virtual ~Expr() {}

    /**
     * @brief A visitor class for expressions.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_assign_expr(Assign* expr) = 0;
        virtual std::any visit_logical_expr(Logical* expr) = 0;
        virtual std::any visit_binary_expr(Binary* expr) = 0;
        virtual std::any visit_unary_expr(Unary* expr) = 0;
        virtual std::any visit_dereference_expr(Dereference* expr) = 0;
        virtual std::any visit_call_expr(Call* expr) = 0;
        virtual std::any visit_access_expr(Access* expr) = 0;
        virtual std::any visit_grouping_expr(Grouping* expr) = 0;
        virtual std::any visit_identifier_expr(Identifier* expr) = 0;
        virtual std::any visit_literal_expr(Literal* expr) = 0;
        virtual std::any visit_array_expr(Array* expr) = 0;
        virtual std::any visit_tuple_expr(Tuple* expr) = 0;
    };

    /**
     * @brief Accepts a visitor class. Information may be passed upward in the return value.
     * CAUTION: Improper casting of the std::any value can result in obscure runtime errors.
     *
     * @param visitor The visitor class to accept.
     * @return std::any The return value from the visitor class.
     */
    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief A base class for nodes in the namespace tree.
 *
 */
class Node {
public:
    class Scope;
    class Locatable;
    class RootScope;
    class NamespaceScope;
    class StructScope;
    class LocalScope;
    class Variable;

    virtual ~Node() = default;

    // static int local_scope_count;

    // A unique name for this node. Used for type comparison and LLVM IR generation.
    std::string unique_name;
    // The parent scope of this node. This is never a variable since variables do not have children.
    std::shared_ptr<Scope> parent = nullptr;
};

#endif // CORE_H
