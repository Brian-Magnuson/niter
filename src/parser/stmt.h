#ifndef STMT_H
#define STMT_H

#include <any>

/**
 * @brief An abstract base class for all statements.
 * Includes statements for expressions and declarations.
 *
 */
class Stmt {
public:
    class Expression;
    class Block;
    class Declaration;
    class Conditional;
    class Loop;
    class Return;
    class Break;
    class Continue;
    class Print;
    class EndOfFile;

    virtual ~Stmt() {}

    /**
     * @brief A visitor class for statements.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_expression_stmt(Expression* stmt) = 0;
        virtual std::any visit_block_stmt(Block* stmt) = 0;
        virtual std::any visit_declaration_stmt(Declaration* stmt) = 0;
        virtual std::any visit_conditional_stmt(Conditional* stmt) = 0;
        virtual std::any visit_loop_stmt(Loop* stmt) = 0;
        virtual std::any visit_return_stmt(Return* stmt) = 0;
        virtual std::any visit_break_stmt(Break* stmt) = 0;
        virtual std::any visit_continue_stmt(Continue* stmt) = 0;
        virtual std::any visit_print_stmt(Print* stmt) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief A class representing an expression statement.
 * Expression statements are statements that consist of an expression.
 *
 */
class Stmt::Expression : public Stmt {
public:
    Expression(std::shared_ptr<Expr> expression) : expression(expression) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_expression_stmt(this);
    }

    // The expression in the statement
    std::shared_ptr<Expr> expression;
};

/**
 * @brief A class representing a print statement.
 * Print statements are statements that consist of the "puts" keyword followed by an expression.
 * Designed to be temporary until a proper print function is implemented.
 *
 */
class Stmt::Print : public Stmt {
public:
    Print(std::shared_ptr<Expr> value) : value(value) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_print_stmt(this);
    }

    // The expression to print
    std::shared_ptr<Expr> value;
};

/**
 * @brief A class representing an end-of-file statement.
 * EOF statements are used to signal the end of the file.
 * They have no member variables.
 *
 */
class Stmt::EndOfFile : public Stmt {
public:
    EndOfFile() {}

    std::any accept(Visitor* visitor) override {
        return std::any();
    }
};

#endif // STMT_H
