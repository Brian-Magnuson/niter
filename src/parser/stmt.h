#ifndef STMT_H
#define STMT_H

#include "decl.h"
#include "expr.h"
#include <any>

class Decl;
class Expr;

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
    class Print;
    class EndOfFile;

    virtual ~Stmt() {}

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
        virtual std::any visit_print_stmt(Print* stmt) = 0;
        virtual std::any visit_eof_stmt(EndOfFile* stmt) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief A class representing a declaration statement.
 * Declaration statements are statements that consist of a declaration.
 *
 */
class Stmt::Declaration : public Stmt {
public:
    Declaration(std::shared_ptr<Decl> declaration) : declaration(declaration) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_declaration_stmt(this);
    }

    // The declaration in the statement
    std::shared_ptr<Decl> declaration;
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
 * @brief A class representing a return statement.
 * Return statements consist of the "return" keyword and optionally an expression.
 *
 */
class Stmt::Return : public Stmt {
public:
    Return(Token keyword, std::shared_ptr<Expr> value) : keyword(keyword), value(value) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_return_stmt(this);
    }

    // The keyword that signifies the return statement.
    Token keyword;
    // The expression to return.
    // If the shared_ptr is nullptr, no expression was specified.
    // If the shared_ptr is pointing to an expression that represents nil, the return statement was explicitly specified to return nil.
    std::shared_ptr<Expr> value;
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
        return visitor->visit_eof_stmt(this);
    }
};

#endif // STMT_H
