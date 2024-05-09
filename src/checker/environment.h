#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include <map>
#include <memory>
#include <string>
#include <tuple>

/**
 * @brief A singleton class to store environment information for the type checkers.
 * Includes a registry of all structs and functions.
 * Also tracks namespaces in a tree.
 *
 */
class Environment {

public:
    /**
     * @brief Get the singleton instance of the Environment. Will create the instance if it does not exist.
     *
     * @return Environment& A reference to the Environment singleton instance.
     */
    static Environment&
    inst() {
        static Environment instance;
        return instance;
    }

    /**
     * @brief Looks up the type of a binary expression.
     *
     * @param op The operator of the binary expression.
     * @param lhs The type of the left-hand side of the binary expression.
     * @param rhs The type of the right-hand side of the binary expression.
     * @return std::pair<std::shared_ptr<Annotation>, bool> A pair of the resulting type and whether the operation is trivial.
     */
    std::pair<std::shared_ptr<Annotation>, bool> lookup_op_type(TokenType op, std::shared_ptr<Annotation> lhs, std::shared_ptr<Annotation> rhs);
};

#endif // ENVIRONMENT_H
