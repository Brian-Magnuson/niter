#ifndef CORE_H
#define CORE_H

#include "../logger/error_code.h"
#include "../scanner/token.h"
#include "llvm/IR/Type.h"
#include <any>
#include <memory>
#include <string>

/**
 * @brief A base class representing a type.
 * Types are used to represent the kind of data that can be stored in a variable.
 * They can be resolved to LLVM types for code generation.
 * If a type contains a struct type, the struct type will point to a Node in the namespace tree.
 * Types are different from Annotations, which merely represent the label of a type applied to a declaration.
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

    class Aggregate;

    class Named;
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
     * Note that if the type is an aggregate type, the LLVM type will be a pointer to the aggregate type.
     * To get the actual aggregate type, use the to_llvm_aggregate_type of the Aggregate class.
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
     * @return 0 If the types are compatible.
     * E_INDETERMINATE_ARRAY_TYPE If the types are incompatible because `a` and `b` are arrays of blank types.
     * E_ARRAY_SIZE_UNKNOWN If the types are incompatible because `a` is a sized array and `b` has an unknown size.
     * E_SIZED_ARRAY_WITHOUT_INITIALIZER If the types are incompatible because `a` is a sized array and `b` is a blank type.
     * E_INCOMPATIBLE_TYPES If the types are incompatible for any other reason.
     */
    static ErrorCode are_compatible(std::shared_ptr<Type>& a, std::shared_ptr<Type>& b);

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

    /**
     * @brief Checks if the type is a primitive numeric type.
     * Numeric types are either integers or floating point numbers.
     *
     * @return true
     * @return false
     */
    bool is_numeric() {
        return is_int() || is_float();
    }
};

/**
 * @brief An abstract base class for all statements in the AST.
 * Includes statements for expressions and declarations.
 * A program is an ordered list of statements.
 * Function declarations and blocks may also contain a list of statements.
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

    /**
     * @brief Accepts a visitor class. Information may be passed upward in the return value.
     * CAUTION: Improper casting of the std::any value can result in obscure runtime errors.
     *
     * @param visitor The visitor class to accept.
     * @return std::any The return value from the visitor class.
     * Statements usually return the empty std::any value unless the statement contains a return statement.
     * If the visitor is a type checker, the return value will be a Type.
     * If the visitor is a code generator, usually nullptr.
     */
    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief An abstract base class for all declarations in the AST.
 * A declaration is a statement that introduces a new name into the program.
 * Structs are used to create Struct Nodes in the namespace tree.
 * Other declaration types are VarDeclarable, meaning they can be declared as variables in the Environment.
 * If a declaration is VarDeclarable, it may store its resolved Type in addition to its Annotation.
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

    /**
     * @brief Accepts a visitor class. Information may be passed upward in the return value.
     * CAUTION: Improper casting of the std::any value can result in obscure runtime errors.
     *
     * @param visitor The visitor class to accept.
     * @return std::any The return value from the visitor class.
     * In the type checkers, visiting a declaration usually yields the empty std::any value.
     * In the code generator, visiting a declaration usually yields nullptr.
     */
    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief An abstract base class for all expressions in the AST.
 * Expressions are statements that evaluate to a value.
 * In the type checkers, visiting an expression will yield a Type.
 * In the code generator, visiting an expression will yield an LLVM Value.
 * Some expressions are LValues; these expressions implement a special visit function for code generation.
 *
 */
class Expr {
public:
    class LValue;

    class Assign;
    class Logical;
    class Binary;
    class Unary;
    class Dereference;
    class Access;
    class Index;
    class Call;
    class Cast;
    class Grouping;
    class Identifier;
    class Literal;
    class Array;
    class ArrayGen;
    class Tuple;
    class Object;

    class LAccess;
    class LIndex;

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
        virtual std::any visit_access_expr(Access* expr) = 0;
        virtual std::any visit_index_expr(Index* expr) = 0;
        virtual std::any visit_call_expr(Call* expr) = 0;
        virtual std::any visit_cast_expr(Cast* expr) = 0;
        virtual std::any visit_grouping_expr(Grouping* expr) = 0;
        virtual std::any visit_identifier_expr(Identifier* expr) = 0;
        virtual std::any visit_literal_expr(Literal* expr) = 0;
        virtual std::any visit_array_expr(Array* expr) = 0;
        virtual std::any visit_array_gen_expr(ArrayGen* expr) = 0;
        virtual std::any visit_tuple_expr(Tuple* expr) = 0;
        virtual std::any visit_object_expr(Object* expr) = 0;
    };

    /**
     * @brief Accepts a visitor class. Information may be passed upward in the return value.
     * CAUTION: Improper casting of the std::any value can result in obscure runtime errors.
     *
     * @param visitor The visitor class to accept.
     * @return std::any The return value from the visitor class.
     * In the type checkers, visiting an expression will yield a Type.
     * In the code generator, visiting an expression will yield an LLVM Value.
     */
    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief A base class for nodes in the namespace tree.
 * The namespace tree is a tree structure representing the various *spaces* in the program
 * and the variables contained within them.
 * All nodes have a reference to their parent scope; the root scope has a nullptr parent.
 * Some nodes in the namespace tree are Scopes, which contain other nodes.
 * Some nodes are also Locatable, which means they have a location in the source code.
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

    // The number of local scopes created. Used for generating unique names.
    static int local_scope_count;
    // A unique name for this node. Used for type comparison and LLVM IR generation.
    std::string unique_name;
    // The parent scope of this node. This is never a variable since variables do not have children.
    std::shared_ptr<Scope> parent = nullptr;
};

#endif // CORE_H
