#ifndef EXPR_H
#define EXPR_H

#include <any>

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
        virtual std::any visit_binary_expr(Binary* expr) = 0;
        virtual std::any visit_grouping_expr(Grouping* expr) = 0;
        virtual std::any visit_literal_expr(Literal* expr) = 0;
        virtual std::any visit_logical_expr(Logical* expr) = 0;
        virtual std::any visit_unary_expr(Unary* expr) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

#endif // EXPR_H
