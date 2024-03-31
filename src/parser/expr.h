#ifndef EXPR_H
#define EXPR_H

#include "../scanner/token.h"
#include <any>
#include <memory>

/**
 * @brief An abstract base class for all expressions.
 *
 */
class Expr {
public:
    class Variable;
    class Call;
    class Assignment;
    class Binary;
    class Grouping;
    class Literal;
    class Logical;
    class Unary;

    virtual ~Expr() {}

    /**
     * @brief A visitor class for expressions.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_variable_expr(Variable* expr) = 0;
        virtual std::any visit_call_expr(Call* expr) = 0;
        virtual std::any visit_assignment_expr(Assignment* expr) = 0;
        virtual std::any visit_logical_expr(Logical* expr) = 0;
        virtual std::any visit_binary_expr(Binary* expr) = 0;
        virtual std::any visit_unary_expr(Unary* expr) = 0;
        virtual std::any visit_grouping_expr(Grouping* expr) = 0;
        virtual std::any visit_literal_expr(Literal* expr) = 0;
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
 * @brief An expression representing a logical expression.
 * E.g. a AND b, a OR b.
 *
 */
class Expr::Logical : public Expr {
public:
    Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}

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
 * @brief An expression representing a binary expression.
 * Includes arithmetic and comparison expressions, i.e. +, -, *, /, <, >, ==, !=, etc.
 *
 */
class Expr::Binary : public Expr {
public:
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}

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

#endif // EXPR_H
