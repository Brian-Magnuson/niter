#ifndef DECL_H
#define DECL_H

#include "../utility/core.h"

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include "expr.h"
#include "stmt.h"
#include <any>
#include <memory>
#include <vector>

/**
 * @brief A base class for all variable-declarable declarations.
 * We say that a declaration is variable-declarable if the Environment class can declare the declaration as a variable.
 * Includes normal variable declarations and function declarations.
 * This class allows the namespace tree to store a reference to the AST node that represents the declaration.
 * Unlike some Nodes in the namespace tree, the AST node lives for the entire duration of the program, making it a reliable source of type information.
 *
 */
class Decl::VarDeclarable : public Decl {
protected:
    VarDeclarable(TokenType declarer, Token name, std::shared_ptr<Annotation> type_annotation) : declarer(declarer), name(name), type_annotation(type_annotation) {
        location = name.location;
    }

public:
    virtual ~VarDeclarable() {}

    // The token type that signifies the declaration type.
    TokenType declarer;
    // The name of the variable.
    Token name;
    // The type annotation of the variable. nullptr if no type was specified.
    std::shared_ptr<Annotation> type_annotation;
    // The type of the variable. nullptr if the type was not resolved.
    // To be set by the type checker.
    std::shared_ptr<Type> type = nullptr;
};

class Expr;
// Forward declaration Expr for Decl::Var

/**
 * @brief A class representing a variable declaration.
 * E.g. var a = 1; const msg = "Hello, world!";
 *
 */
class Decl::Var : public Decl::VarDeclarable {
public:
    // Var(TokenType declarer, Token name, std::shared_ptr<Annotation> type_annotation, std::shared_ptr<Expr> initializer) : declarer(declarer), name(name), type_annotation(type_annotation), initializer(initializer) {
    //     location = name.location;
    // }

    Var(TokenType declarer, Token name, std::shared_ptr<Annotation> type_annotation, std::shared_ptr<Expr> initializer) : VarDeclarable(declarer, name, type_annotation), initializer(initializer) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_var_decl(this);
    }

    // The initializer expression. Note: if the variable is explicitly initialized to nil, this will still point to an expression that represents nil.
    std::shared_ptr<Expr> initializer;
};

/**
 * @brief A class representing a function declaration.
 * E.g. fun add(a: int, b: int): int { return a + b; }
 *
 */
class Decl::Fun : public Decl::VarDeclarable {
public:
    Fun(
        TokenType declarer,
        Token name,
        std::vector<std::shared_ptr<Decl::Var>> parameters,
        std::shared_ptr<Decl::Var> return_var,
        std::shared_ptr<Annotation> type_annotation,
        std::vector<std::shared_ptr<Stmt>> body
    ) : VarDeclarable(declarer, name, type_annotation), parameters(parameters), return_var(return_var), body(body) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_fun_decl(this);
    }

    // The parameters of the function.
    std::vector<std::shared_ptr<Decl::Var>> parameters;
    // The return variable of the function. nullptr if the function does not return a value.
    std::shared_ptr<Decl::Var> return_var;
    // The body of the function.
    std::vector<std::shared_ptr<Stmt>> body;
};

/**
 * @brief A class representing an external function declaration.
 * E.g. extern fun printf(format: string, ...): int;
 *
 */
class Decl::ExternFun : public Decl::VarDeclarable {
public:
    ExternFun(
        TokenType declarer,
        Token name,
        std::shared_ptr<Annotation> type_annotation
    ) : VarDeclarable(declarer, name, type_annotation) {
        location = name.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_extern_fun_decl(this);
    }
};

/**
 * @brief A class representing a struct declaration.
 * E.g. struct Point { x: int; y: int; }
 * Note: A struct contains a list of declarations, which can be VarDeclarables or other Structs.
 *
 */
class Decl::Struct : public Decl {
public:
    Struct(TokenType declarer, Token name, std::vector<std::shared_ptr<Decl>> declarations)
        : declarer(declarer), name(name), declarations(declarations) {
        location = name.location;
    }

    std::any accept(Visitor* visitor) override {
        return visitor->visit_struct_decl(this);
    }

    // The token type that signifies the declaration type.
    TokenType declarer;
    // The name of the struct.
    Token name;
    // The declarations of the struct.
    std::vector<std::shared_ptr<Decl>> declarations;
};

#endif // DECL_H
