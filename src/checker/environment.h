#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include "scope.h"
#include <map>
#include <memory>
#include <string>
#include <tuple>

/**
 * @brief A singleton class to store environment information for the type checkers.
 * Uses a namespace tree to store scopes.
 *
 */
class Environment {

    // The root of the namespace tree.
    std::shared_ptr<Scope> global_tree;
    // The current scope in the namespace tree.
    std::shared_ptr<Scope> current_scope;

public:
    /**
     * @brief Get the singleton instance of the Environment. Will create the instance if it does not exist.
     *
     * @return Environment& A reference to the Environment singleton instance.
     */
    static Environment& inst() {
        static Environment instance;
        return instance;
    }

    /**
     * @brief Adds a namespace to the current scope and enters it.
     * A namespace can only be added if the current scope is a namespace or the root.
     *
     * @param name The name of the namespace to add.
     * @return true If the namespace was added successfully.
     * @return false If the namespace cannot be added.
     */
    bool add_namespace(const std::string& name);

    /**
     * @brief Adds a struct to the current scope and enters it.
     * A struct can only be added if the current scope is the root, a namespace, or another struct.
     *
     * @param name The name of the struct to add.
     * @return true If the struct was added successfully.
     * @return false If the struct cannot be added.
     */
    bool add_struct(const std::string& name);

    /**
     * @brief Adds a local scope to the enivironment.
     * A local scope is added when a function is encountered.
     * Unlike global scopes, local scopes are removed when exited.
     *
     * @return true If the scope was added successfully.
     * @return false If the scope cannot be added.
     */
    bool increase_local_scope();

    /**
     * @brief Exits the current scope.
     * If the current scope is the root, it will not exit.
     * If the current scope is a local scope, the local scope will be removed.
     * If the current scope is a namespace or struct, it will exit to the parent scope, but the current scope will not be removed.
     *
     * @return true If the scope was exited successfully.
     * @return false If the scope cannot be exited.
     */
    bool exit_scope();

    /**
     * @brief Declares a new symbol in the current scope.
     * If the symbol already exists in the current scope, it will not be declared.
     * If the current scope is a local scope, this information will be removed when the local scope is exited.
     * If the current scope is a global scope, this information will be kept until the program ends or the environment is reset.
     *
     * @param name The name of the symbol to declare.
     * @param type The type of the symbol to declare.
     * @return true If the symbol was declared successfully.
     * @return false If the symbol cannot be declared.
     */
    bool declare_symbol(const std::string& name, std::shared_ptr<Annotation> type);

    /**
     * @brief Retrieves the type of a symbol in the current scope.
     * If the symbol does not exist in the current scope, the parent scopes will be searched from the current scope to the root.
     * If the parent scopes do not contain the symbol, nullptr will be returned.
     * If the symbol happens to be of type `void` or `nil`, an annotation representing the type will be returned (not nullptr).
     *
     * @param name The name of the symbol to retrieve.
     * @return std::shared_ptr<Annotation> The type of the symbol.
     */
    std::shared_ptr<Annotation> get_type(const std::string& name);
};

#endif // ENVIRONMENT_H
