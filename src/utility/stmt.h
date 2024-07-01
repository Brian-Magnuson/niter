#ifndef STMT_H
#define STMT_H

#include "../utility/core.h"

#include "../scanner/token.h"
#include "decl.h"
#include "expr.h"
#include <any>
#include <memory>

/**
 * @brief A class representing a declaration statement.
 * Declaration statements are statements that consist of a declaration.
 *
 */
class Stmt::Declaration : public Stmt {
public:
    Declaration(std::shared_ptr<Decl> declaration) : declaration(declaration) {
        location = declaration->location;
    }

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
    Expression(std::shared_ptr<Expr> expression) : expression(expression) {
        location = expression->location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_expression_stmt(this);
    }

    // The expression in the statement
    std::shared_ptr<Expr> expression;
};

/**
 * @brief A class representing a conditional statement.
 * Conditional statements contain an expression, a list of statements for the "then" branch, and optionally a list of statements for the "else" branch.
 *
 */
class Stmt::Conditional : public Stmt {
public:
    Conditional(Token keyword, std::shared_ptr<Expr> condition, std::vector<std::shared_ptr<Stmt>> then_branch, std::vector<std::shared_ptr<Stmt>> else_branch)
        : keyword(keyword), condition(condition), then_branch(then_branch), else_branch(else_branch) {
        location = condition->location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_conditional_stmt(this);
    }

    // The keyword that signifies the conditional statement.
    Token keyword;
    // The condition of the conditional statement
    std::shared_ptr<Expr> condition;
    // The statements in the "then" branch
    std::vector<std::shared_ptr<Stmt>> then_branch;
    // The statements in the "else" branch
    std::vector<std::shared_ptr<Stmt>> else_branch;
};

/**
 * @brief A class representing a loop statement.
 * Loop statements contain an expression for the condition and a list of statements for the loop body.
 *
 */
class Stmt::Loop : public Stmt {
public:
    Loop(Token keyword, std::shared_ptr<Expr> condition, std::vector<std::shared_ptr<Stmt>> body)
        : keyword(keyword), condition(condition), body(body) {
        location = condition->location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_loop_stmt(this);
    }

    // The keyword that signifies the loop statement.
    Token keyword;
    // The condition of the loop statement
    std::shared_ptr<Expr> condition;
    // The statements in the loop body
    std::vector<std::shared_ptr<Stmt>> body;
};

/**
 * @brief A class representing a return statement.
 * Return statements consist of the "return" keyword and optionally an expression.
 *
 */
class Stmt::Return : public Stmt {
public:
    Return(Token keyword, std::shared_ptr<Expr> value) : keyword(keyword), value(value) {
        location = keyword.location;
    }

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
