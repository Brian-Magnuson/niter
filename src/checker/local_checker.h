#ifndef LOCAL_CHECKER_H
#define LOCAL_CHECKER_H

#include "../parser/decl.h"
#include "../parser/expr.h"
#include "environment.h"
#include <memory>

/**
 * @brief A class for the local type checker.
 * The local type checker is the second part of a two-stage type checker.
 * This type checker specifically checks all the declarations made in local space.
 * Note: local space includes anything declared within a function body.
 * Theoretically, this type checker should only have to make one pass over the code since all types are made known from the global checker.
 *
 */
class LocalChecker {
    std::shared_ptr<Environment> environment;

public:
    LocalChecker(std::shared_ptr<Environment> environment) : environment(environment) {}
};

#endif // LOCAL_CHECKER_H
