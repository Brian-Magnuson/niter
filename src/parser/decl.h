#ifndef DECL_H
#define DECL_H

#include "../scanner/token.h"
#include "annotation.h"
#include "expr.h"
#include "stmt.h"
#include <any>
#include <memory>
#include <vector>

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
        virtual std::any visit_struct_decl(Struct* decl) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

class Expr;
// Forward declaration Expr for Decl::Var

/**
 * @brief A class representing a variable declaration.
 * E.g. var a = 1; const msg = "Hello, world!";
 *
 */
class Decl::Var : public Decl {
public:
    Var(TokenType declarer, Token name, std::shared_ptr<Annotation> type_annotation, std::shared_ptr<Expr> initializer) : declarer(declarer), name(name), type_annotation(type_annotation), initializer(initializer) {
        location = name.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_var_decl(this);
    }

    // The token type that signifies the declaration type.
    TokenType declarer;
    // The name of the variable.
    Token name;
    // The type annotation of the variable. nullptr if no type was specified.
    std::shared_ptr<Annotation> type_annotation;
    // The initializer expression. Note: if the variable is explicitly initialized to nil, this will still point to an expression that represents nil.
    std::shared_ptr<Expr> initializer;
};

class Stmt;
// Forward declaration Stmt for Decl::Fun

/**
 * @brief A class representing a function declaration.
 * E.g. fun add(a: int, b: int): int { return a + b; }
 *
 */
class Decl::Fun : public Decl {
public:
    Fun(
        TokenType declarer,
        Token name,
        std::vector<std::shared_ptr<Decl::Var>> parameters,
        std::shared_ptr<Decl::Var> return_var,
        std::shared_ptr<Annotation> type_annotation,
        std::vector<std::shared_ptr<Stmt>> body
    ) : declarer(declarer), name(name), parameters(parameters), return_var(return_var), type_annotation(type_annotation), body(body) {
        location = name.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_fun_decl(this);
    }

    // The token type that signifies the declaration type.
    TokenType declarer;
    // The name of the function.
    Token name;
    // The parameters of the function.
    std::vector<std::shared_ptr<Decl::Var>> parameters;
    // The return variable of the function. nullptr if the function does not return a value.
    std::shared_ptr<Decl::Var> return_var;
    // The type of the function. Includes the return type and the type arguments.
    std::shared_ptr<Annotation> type_annotation;
    // The body of the function.
    std::vector<std::shared_ptr<Stmt>> body;
};

/**
 * @brief A class representing a struct declaration.
 * E.g. struct Point { x: int; y: int; }
 *
 */
class Decl::Struct : public Decl {
public:
    Struct(TokenType declarer, Token name, std::vector<std::shared_ptr<Decl::Var>> fields) : declarer(declarer), name(name), fields(fields) {
        location = name.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_struct_decl(this);
    }

    // The token type that signifies the declaration type.
    TokenType declarer;
    // The name of the struct.
    Token name;
    // The fields of the struct.
    std::vector<std::shared_ptr<Decl::Var>> fields;
};

#endif // DECL_H
