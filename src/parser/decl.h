#ifndef DECL_H
#define DECL_H

#include "../scanner/token.h"
#include "expr.h"
#include "stmt.h"
#include <any>
#include <memory>

class Expr;
class Stmt;

/**
 * @brief An abstract base class for all declarations.
 *
 */
class Decl {
public:
    class Var;
    class Fun;
    class Struct;

    virtual ~Decl() {}

    /**
     * @brief A visitor class for declarations.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_var_decl(Var* decl) = 0;
        virtual std::any visit_fun_decl(Fun* decl) = 0;
        virtual std::any visit_struct_decl(Struct* decl) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

/**
 * @brief A class representing a variable declaration.
 * E.g. var a = 1; const msg = "Hello, world!";
 *
 */
class Decl::Var : public Decl {
public:
    Var(Token declarer, Token name, std::shared_ptr<Expr::Identifier> type_annotation, std::shared_ptr<Expr> initializer) : declarer(declarer), name(name), type_annotation(type_annotation), initializer(initializer) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_var_decl(this);
    }

    // The token that signifies the declaration type.
    Token declarer;
    // The name of the variable.
    Token name;
    // The type annotation of the variable. nullptr if no type was specified.
    std::shared_ptr<Expr::Identifier> type_annotation;
    // The initializer expression. Note: if the variable is explicitly initialized to nil, this will still point to an expression that represents nil.
    std::shared_ptr<Expr> initializer;
};

/**
 * @brief A class representing a function declaration.
 * E.g. fun add(a: int, b: int): int { return a + b; }
 *
 */
class Decl::Fun : public Decl {
public:
    Fun(
        Token name,
        std::vector<Decl::Var> parameters,
        std::shared_ptr<Expr::Identifier> return_type,
        std::vector<std::shared_ptr<Stmt>> body
    )
        : name(name), parameters(parameters), return_type(return_type), body(body) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_fun_decl(this);
    }

    // The name of the function.
    Token name;
    // The parameters of the function.
    std::vector<Decl::Var> parameters;
    // The return type of the function. If no type was provided, an identifier with the name "void" will be inserted.
    std::shared_ptr<Expr::Identifier> return_type;
    // The body of the function.
    std::vector<std::shared_ptr<Stmt>> body;
};

#endif // DECL_H
