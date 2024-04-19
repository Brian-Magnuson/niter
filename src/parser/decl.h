#ifndef DECL_H
#define DECL_H

#include "../scanner/token.h"
#include "expr.h"
#include <any>
#include <memory>

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
    Var(Token declarer, Token name, std::shared_ptr<Expr> initializer) : declarer(declarer), name(name), initializer(initializer) {}

    std::any accept(Visitor* visitor) override {
        return visitor->visit_var_decl(this);
    }

    // The token that signifies the declaration type.
    Token declarer;
    // The name of the variable.
    Token name;
    // The initializer expression. Note: if the variable is explicitly initialized to nil, this will still point to an expression that represents nil.
    std::shared_ptr<Expr> initializer;
};

#endif // DECL_H
