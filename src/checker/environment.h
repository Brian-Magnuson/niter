#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include <map>
#include <memory>
#include <string>
#include <tuple>

enum class ScopeType {
    // The root scope of the environment.
    ROOT,
    // A namespace scope.
    NAMESPACE,
    // A struct scope.
    STRUCT,
    // A local block scope.
    LOCAL,
};

/**
 * @brief A singleton class to store environment information for the type checkers.
 * Includes a registry of all structs and functions.
 * Also tracks namespaces in a tree.
 *
 */
class Environment {
    struct Scope {
        ScopeType kind;
        std::shared_ptr<Scope> parent;
        std::map<std::string, std::shared_ptr<Annotation>> table;
        std::map<std::string, std::shared_ptr<Scope>> children;

        Scope(ScopeType kind, std::shared_ptr<Scope> parent)
            : kind(kind), parent(parent) {}
    };

    std::shared_ptr<Scope> global_tree;
    std::shared_ptr<Scope> current_scope;

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
     * @brief Adds a global scope to the enivironment.
     * Adding a ROOT scope is not allowed.
     * NAMESPACES may only be added to ROOT scopes or other NAMESPACES.
     * STRUCTS may be added to ROOT, NAMESPACES, or other STRUCTS.
     * Adding a LOCAL scope is not allowed. Use increase_local_scope() instead.
     *
     * @param kind The kind of scope to add.
     * @param name The name of the scope to add.
     * @return true If the scope was added successfully.
     * @return false If the scope cannot be added.
     */
    bool add_global_scope(ScopeType kind, const std::string& name);

    /**
     * @brief Adds a local scope to the enivironment.
     *
     * @return true If the scope was added successfully.
     * @return false If the scope cannot be added.
     */
    bool increase_local_scope();

    /**
     * @brief Removes the current local scope from the environment.
     *
     * @return true If the scope was removed successfully.
     * @return false If the scope cannot be removed.
     */
    bool decrease_local_scope();

    /**
     * @brief Declares a local variable.
     *
     * @param name The name of the variable to declare.
     * @param annotation The type of the variable. Should not be auto.
     * @return true If the variable was declared successfully.
     * @return false If the variable cannot be declared.
     */
    bool declare_local(const std::string& name, const std::shared_ptr<Annotation>& annotation);

    std::shared_ptr<Annotation> get_type(const std::string& name);
};

#endif // ENVIRONMENT_H
