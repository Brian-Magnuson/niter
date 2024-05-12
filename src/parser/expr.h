#ifndef EXPR_H
#define EXPR_H

#include "../scanner/token.h"
#include "annotation.h"
#include <any>
#include <memory>
#include <vector>

/**
 * @brief An abstract base class for all expressions.
 *
 */
class Expr {
public:
    class Assign;
    class Logical;
    class Binary;
    class Unary;
    class Call;
    class Access;
    class Grouping;
    class Identifier;
    class Literal;
    class Array;
    class Tuple;

    // An annotation representing the type of the expression. Set to nullptr, to be filled in by the type checker.
    std::shared_ptr<Annotation> type_annotation = nullptr;
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
    Unary(Token op, std::shared_ptr<Expr> right) : op(op), right(right) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_unary_expr(this);
    }

    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing an access expression.
 * An access expression is an expression where a member of an object is accessed.
 * This could be using the dot, single arrow, or subscript operators.
 * (. or -> or [])
 *
 */
class Expr::Access : public Expr {
public:
    Access(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {
        location = op.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_access_expr(this);
    }

    // The expression on the left side.
    std::shared_ptr<Expr> left;
    // The token representing the operator.
    Token op;
    // The expression on the right side.
    std::shared_ptr<Expr> right;
};

/**
 * @brief A class representing a call expression.
 * Usually, this means a function call, indicated by parentheses.
 * E.g. add(1, 2)
 *
 */
class Expr::Call : public Expr {
public:
    Call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
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
class Expr::Identifier : public Expr {
public:
    Identifier(std::vector<Token> tokens) : tokens(tokens) {
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
    Array(std::vector<std::shared_ptr<Expr>> elements, Token bracket) : bracket(bracket), elements(elements) {
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
    Tuple(std::vector<std::shared_ptr<Expr>> elements, Token paren) : elements(elements), paren(paren) {
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

#endif // EXPR_H
