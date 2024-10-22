#ifndef EXPR_H
#define EXPR_H

#include "../utility/core.h"

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include "../utility/dictionary.h"
#include "type.h"
#include "llvm/IR/Value.h"
#include <any>
#include <map>
#include <memory>
#include <vector>

class CodeGenerator;
// Forward declaration of the CodeGenerator class for the Locatable class.

/**
 * @brief A base class for lvalues, i.e., expressions that have a memory location.
 * Lvalues may be on the left side of an assignment, can be passed by reference, and can have their address taken.
 * These include Identifier, LAccess, LIndex, and Dereference expressions.
 *
 */
class Expr::LValue : virtual public Expr {
protected:
    LValue() {}

public:
    virtual ~LValue() {}

    /**
     * @brief Retrieves the declarer associated with the lvalue.
     * An lvalue is mutable iff its declarer KW_VAR.
     *
     * @return TokenType The token type of the declarer.
     */
    virtual TokenType get_lvalue_declarer() = 0;

    /**
     * @brief Get the llvm allocation pointer for the lvalue.
     * We require the code generator because, if the lvalue is a dereference expression, we need to visit the inner expression.
     *
     * @param code_generator The code generator to use.
     * @return llvm::Value* The llvm::Value* representing the memory location of the lvalue. Use CreateStore to store a value in this location.
     */
    virtual llvm::Value* get_llvm_allocation(CodeGenerator* code_generator) = 0;
};

/**
 * @brief A class representing an assignment expression.
 * E.g. a = b = c.
 *
 */
class Expr::Assign : public Expr {
public:
    Assign(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_assign_expr(this);
    }

    // TODO: All lvalues inherit Expr::LValue. Consider changinge the type of `left`.
    // Note that doing so would mean lvalue errors are caught in the parser, not the type checker.

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing a logical expression.
 * E.g. a AND b, a OR b.
 *
 */
class Expr::Logical : public Expr {
public:
    Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_logical_expr(this);
    }

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing a binary expression.
 * Includes arithmetic and comparison expressions, i.e. +, -, *, /, <, >, ==, !=, etc.
 *
 */
class Expr::Binary : public Expr {
public:
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_binary_expr(this);
    }

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing a unary expression. Includes unary minus and logical negation.
 *
 */
class Expr::Unary : public Expr {
public:
    Unary(Token op, std::shared_ptr<Expr> inner) : op(op), inner(inner) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_unary_expr(this);
    }

    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> inner;
};

/**
 * @brief A class representing a dereference expression.
 * A dereference expression is an expression where a pointer is dereferenced (with the * operator).
 * Structurally, it is similar to a unary expression, but these expressions can serve as lvalues.
 *
 */
class Expr::Dereference : public Expr::LValue {
public:
    Dereference(Token op, std::shared_ptr<Expr> inner) : op(op), inner(inner) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_dereference_expr(this);
    }

    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> inner;

    TokenType get_lvalue_declarer() override;
    llvm::Value* get_llvm_allocation(CodeGenerator* code_generator) override;
};

/**
 * @brief A class representing an access expression.
 * An access expression is an expression where a member of an object is accessed.
 * This could be using the dot or single arrow operator.
 * (. or ->)
 * Note: A single error operator simply dereferences the pointer and accesses the member.
 *
 */
class Expr::Access : virtual public Expr {
public:
    Access(std::shared_ptr<Expr> left, Token op, Token ident)
        : left(left), op(op), ident(ident) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_access_expr(this);
    }

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the operator.
    Token op;
    // The ident token on the right side.
    Token ident;
};

/**
 * @brief A class representing an lvalue access expression.
 * An lvalue access expression is an expression where a member of an object is accessed.
 * An access expression is an lvalue if the left side is an lvalue.
 * It is treated as an access expression when visited normally, but also implements the LValue interface.
 *
 */
class Expr::LAccess : public Expr::LValue, public Expr::Access {
public:
    LAccess(std::shared_ptr<Expr::LValue> left, Token op, Token ident)
        : Access(left, op, ident), left_lvalue(left) {
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_access_expr(this);
    }

    // A copy of the left side, casted to an lvalue for convenience.
    std::shared_ptr<Expr::LValue> left_lvalue;

    TokenType get_lvalue_declarer() override;
    llvm::Value* get_llvm_allocation(CodeGenerator* code_generator) override;
};

/**
 * @brief A class representing an index expression.
 * An index expression is an expression where an element of a collection is accessed.
 * It specifically uses the subscript operator ([]).
 *
 */
class Expr::Index : virtual public Expr {
public:
    Index(std::shared_ptr<Expr> left, Token bracket, std::shared_ptr<Expr> right)
        : left(left), bracket(bracket), right(right) {
        location = bracket.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_index_expr(this);
    }

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the opening bracket.
    Token bracket;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing an lvalue index expression.
 * An lvalue index expression is an expression where an element of a collection is accessed.
 * An index expression is an lvalue if the left side is an lvalue.
 * It is treated as an index expression when visited normally, but also implements the LValue interface.
 *
 */
class Expr::LIndex : public Expr::LValue, public Expr::Index {
public:
    LIndex(std::shared_ptr<Expr::LValue> left, Token bracket, std::shared_ptr<Expr> right)
        : Index(left, bracket, right), left_lvalue(left) {
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_index_expr(this);
    }

    // A copy of the left side, casted to an lvalue for convenience.
    std::shared_ptr<Expr::LValue> left_lvalue;

    TokenType get_lvalue_declarer() override;
    llvm::Value* get_llvm_allocation(CodeGenerator* code_generator) override;
};

/**
 * @brief A class representing a call expression.
 * Usually, this means a function call, indicated by parentheses.
 * E.g. add(1, 2)
 *
 */
class Expr::Call : public Expr {
public:
    Call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>>& arguments)
        : callee(callee), paren(paren), arguments(arguments) {
        location = paren.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_call_expr(this);
    }

    // The expression being called.
    std::shared_ptr<Expr> callee;
    // The token representing the opening parenthesis.
    Token paren;
    // The arguments to the call.
    std::vector<std::shared_ptr<Expr>> arguments;
};

/**
 * @brief A class representing a cast expression.
 * E.g. 42 as f32
 *
 */
class Expr::Cast : public Expr {
public:
    Cast(std::shared_ptr<Expr> expression, Token as_kw, std::shared_ptr<Annotation> annotation)
        : expression(expression), as_kw(as_kw), annotation(annotation) {
        location = as_kw.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_cast_expr(this);
    }

    // The expression to cast.
    std::shared_ptr<Expr> expression;
    // The keyword token indicating the cast.
    Token as_kw;
    // The annotation indicating the type to cast to.
    std::shared_ptr<Annotation> annotation;
};

/**
 * @brief A class representing a grouping expression.
 * E.g. (a + b) * c
 *
 */
class Expr::Grouping : public Expr {
public:
    Grouping(std::shared_ptr<Expr> expression) : expression(expression) {
        location = expression->location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_grouping_expr(this);
    }

    // The expression inside the grouping.
    std::shared_ptr<Expr> expression;
};

/**
 * @brief A class representing an identifier.
 * Identifiers may identify variables, functions, structs, namespaces, etc.
 *
 *
 */
class Expr::Identifier : public Expr::LValue {
public:
    Identifier(std::vector<Token>& tokens) : tokens(tokens) {
        location = tokens[0].location;
    }
    Identifier(Token token) : tokens({token}) {
        location = token.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_identifier_expr(this);
    }

    // The tokens representing the identifier. The most general identifier is at the front. The most specific is at the back.
    std::vector<Token> tokens;

    /**
     * @brief Converts the identifier to a string.
     *
     * @return std::string The string representation of the identifier.
     */
    std::string to_string() {
        std::string str = "";
        for (auto& token : tokens) {
            str += token.lexeme + "::";
        }
        return str.substr(0, str.size() - 2);
    }

    TokenType get_lvalue_declarer() override;
    llvm::Value* get_llvm_allocation(CodeGenerator* code_generator) override;
};

/**
 * @brief A class representing a literal expression.
 * E.g. a number, string, or boolean.
 *
 */
class Expr::Literal : public Expr {
public:
    Literal(Token token) : token(token) {
        location = token.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_literal_expr(this);
    }

    // The token representing the literal value.
    Token token;
};

/**
 * @brief A class representing an array expression.
 * E.g. [1, 2, 3]
 *
 */
class Expr::Array : public Expr {
public:
    Array(std::vector<std::shared_ptr<Expr>>& elements, Token bracket) : bracket(bracket), elements(elements) {
        location = bracket.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_array_expr(this);
    }
    // The token representing the opening bracket.
    Token bracket;

    // The elements of the array.
    std::vector<std::shared_ptr<Expr>> elements;
};

/**
 * @brief A class representing an array generator expression.
 * E.g. [Foo::new(); 10]
 *
 */
class Expr::ArrayGen : public Expr {
public:
    ArrayGen(Token bracket, std::shared_ptr<Expr> generator, unsigned size) : bracket(bracket), generator(generator), size(size) {
        location = bracket.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_array_gen_expr(this);
    }

    // The token representing the opening bracket.
    Token bracket;
    // The generator expression.
    std::shared_ptr<Expr> generator;
    // The size of the array, not to be confused with the size of this object.
    unsigned size;
};

/**
 * @brief A class representing a tuple expression.
 * E.g. (1, 2, 3)
 * Note: Parentheses are required for tuples. To prevent ambiguity:
 * (expr) is a grouping expression.
 * (expr,) is a tuple with one element.
 * () is a tuple with no elements.
 *
 */
class Expr::Tuple : public Expr {
public:
    Tuple(std::vector<std::shared_ptr<Expr>>& elements, Token paren) : elements(elements), paren(paren) {
        location = paren.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_tuple_expr(this);
    }

    // The elements of the tuple.
    std::vector<std::shared_ptr<Expr>> elements;
    // The token representing the opening parenthesis.
    Token paren;
};

/**
 * @brief A class representing an object expression.
 * An object expression is a collection of key-value pairs used to initialize a struct.
 * E.g. Point { x: 1, y: 2 }
 *
 */
class Expr::Object : public Expr {
public:
    Object(Token colon, std::shared_ptr<Annotation::Segmented> struct_annotation, Dictionary<std::string, std::shared_ptr<Expr>>& fields)
        : colon(colon), struct_annotation(struct_annotation), fields(fields) {
        location = colon.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_object_expr(this);
    }

    // The token representing the colon.
    Token colon;
    // The annotation representing the struct name.
    std::shared_ptr<Annotation::Segmented> struct_annotation;
    // The key-value pairs of the object.
    Dictionary<std::string, std::shared_ptr<Expr>> fields;
};

#endif // EXPR_H
