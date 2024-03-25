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
    class Block;
    class Declaration;
    class Expression;
    class Conditional;
    class Loop;
    class Return;
    class Break;
    class Continue;

    virtual ~Stmt() {}

    /**
     * @brief A visitor class for statements.
     *
     */
    class Visitor {
    public:
        virtual std::any visit_block_stmt(Block* stmt) = 0;
        virtual std::any visit_declaration_stmt(Declaration* stmt) = 0;
        virtual std::any visit_expression_stmt(Expression* stmt) = 0;
        virtual std::any visit_conditional_stmt(Conditional* stmt) = 0;
        virtual std::any visit_loop_stmt(Loop* stmt) = 0;
        virtual std::any visit_return_stmt(Return* stmt) = 0;
        virtual std::any visit_break_stmt(Break* stmt) = 0;
        virtual std::any visit_continue_stmt(Continue* stmt) = 0;
    };

    virtual std::any accept(Visitor* visitor) = 0;
};

#endif // STMT_H
