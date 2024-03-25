#ifndef DECL_H
#define DECL_H

#include <any>

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

#endif // DECL_H
