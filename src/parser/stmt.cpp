#include "stmt.h"

Stmt::Declaration::Declaration(std::shared_ptr<Decl> declaration) : declaration(declaration) {
    location = declaration->location;
}

Stmt::Expression::Expression(std::shared_ptr<Expr> expression) : expression(expression) {
    location = expression->location;
}

Stmt::Print::Print(std::shared_ptr<Expr> value) : value(value) {
    location = value->location;
}
