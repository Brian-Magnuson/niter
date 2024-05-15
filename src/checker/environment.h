#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../logger/error_code.h"
#include "../parser/annotation.h"
#include "../parser/expr.h"
#include "../scanner/token.h"
#include "node.h"
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/**
 * @brief A singleton class to store environment information for the type checkers.
 * Uses a namespace tree to store scopes.
 *
 */
class Environment {

    // The root of the namespace tree.
    std::shared_ptr<Node::RootScope> global_tree;
    // The current scope in the namespace tree.
    std::shared_ptr<Node::Scope> current_scope;

    // A list of deferred types to be resolved later.
    std::vector<std::pair<std::shared_ptr<Annotation>, std::shared_ptr<Node::Scope>>> deferred_types;

    Environment() {
        reset();
    }

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
     * @brief Adds a namespace to the current scope and enters it.
     * A namespace can only be added if the current scope is a namespace or the root.
     * Namespaces can actually be opened with the same name in the same scope.
     * In this case, new symbols will be added to the existing namespace.
     *
     * @param name The name of the namespace to add.
     * @return ErrorCode 0 if the namespace was added successfully.
     * E_NAMESPACE_IN_LOCAL_SCOPE if the namespace was added in a local scope.
     * E_NAMESPACE_IN_STRUCT if the namespace was added in a struct.
     */
    ErrorCode add_namespace(const std::string& name);

    /**
     * @brief Adds a struct to the current scope and enters it.
     * A struct can only be added if the current scope is the root, a namespace, or another struct.
     *
     * @param name The name of the struct to add.
     * @return ErrorCode 0 if the struct was added successfully.
     * E_STRUCT_ALREADY_DECLARED if the struct already exists in the current scope.
     * E_STRUCT_IN_LOCAL_SCOPE if the struct was added in a local scope.
     */
    ErrorCode add_struct(const std::string& name);

    /**
     * @brief Adds the primitive types to the global scope.
     *
     */
    void install_primitive_types();

    /**
     * @brief Adds a local scope to the enivironment.
     * A local scope is added when a function is encountered.
     * Unlike global scopes, local scopes are removed when exited.
     * This function shouldn't throw any errors.
     *
     */
    void increase_local_scope();

    /**
     * @brief Exits the current scope.
     * If the current scope is the root, it will not exit.
     * If the current scope is a local scope, the local scope will be removed.
     * If the current scope is a namespace or struct, it will exit to the parent scope, but the current scope will not be removed.
     *
     * @return ErrorCode 0 if the scope was exited successfully.
     * E_EXITED_ROOT_SCOPE if the root scope was exited (should never happen).
     */
    ErrorCode exit_scope();

    /**
     * @brief Exits all local scopes.
     * Useful when an error occurs and the local scopes need to be removed.
     *
     */
    void exit_all_local_scopes();

    /**
     * @brief Checks if the current scope is a global scope.
     * Any scope that is not a local scope is considered a global scope.
     *
     * @return true If the current scope is a global scope.
     * @return false If the current scope is a local scope.
     */
    bool in_global_scope();

    /**
     * @brief Declares a new variable in the current scope.
     * If the symbol already exists in the current scope, it will not be declared.
     * If the current scope is a local scope, this information will be removed when the local scope is exited.
     * If the current scope is a global scope, this information will be kept until the program ends or the environment is reset.
     *
     * @param name The name of the symbol to declare.
     * @param declarer The type of the token that declared the variable.
     * @param type The type of the variable to declare.
     * @return ErrorCode 0 if the variable was declared successfully.
     * E_SYMBOL_ALREADY_DECLARED if the symbol already exists in the current scope.
     */
    ErrorCode declare_variable(const std::string& name, TokenType declarer, std::shared_ptr<Annotation> type);

    /**
     * @brief Verifies that a type is valid.
     * That is, the type is either a primitive or references a struct that has been declared.
     * Can be set to allow types to be deferred.
     * When a type is deferred, a pointer to the type and scope will be saved for later resolution.
     * Only the global checker should be deferring types.
     *
     * @param type The type to verify.
     * @param allow_deferral Whether or not to allow the type to be deferred. Default is false.
     * @param from_scope The scope from which the type is being verified. If nullptr, the current global scope will be used. Default is nullptr.
     * @return true If the type is valid.
     * @return false If the type is invalid.
     */
    bool verify_type(
        const std::shared_ptr<Annotation>& type,
        bool allow_deferral = false,
        std::shared_ptr<Node::Scope> from_scope = nullptr
    );

    /**
     * @brief Retrieves the variable node from the current scope.
     * If the identifier has namespaces, downward lookup will be used.
     * If the identifier is only one token long, upward lookup will be used, then downward lookup if the symbol is not found.
     * If the symbol is not found, nullptr will be returned.
     *
     * @param identifier The identifier of the variable to retrieve.
     * @return std::shared_ptr<Node::Variable> A pointer to the variable node.
     */
    std::shared_ptr<Node::Variable> get_variable(const Expr::Identifier* identifier);

    /**
     * @brief Get the instance variable object for a given instance type and member name.
     * The instance type must be a segmented annotation.
     * If it is a pointer, it must be dereferenced first.
     * If the instance_type references a valid struct, but the member is not found, the static members of the struct will be checked.
     * If the member is not found in the struct, nullptr will be returned.
     *
     * @param instance_type The type of the instance. Must reference a struct scope in the global tree.
     * @param member_name The name of the member to retrieve.
     * @return std::shared_ptr<Node::Variable> A pointer to the variable node.
     */
    std::shared_ptr<Node::Variable> get_instance_variable(std::shared_ptr<Type::Struct> instance_type, const std::string& member_name);

    /**
     * @brief Get the struct object for a given segmented annotation.
     *
     * @param type The segmented annotation to get the struct for.
     * @return std::shared_ptr<Node::StructScope> A pointer to the struct scope node in the global tree.
     */
    std::shared_ptr<Node::StructScope> get_struct(std::shared_ptr<Annotation::Segmented> type);

    /**
     * @brief Get the struct object for a given name.
     * Use this function when the struct name is a single string segment.
     *
     * @param name The short name of the struct to get.
     * @return std::shared_ptr<Node::StructScope> A pointer to the struct scope node in the global tree.
     */
    std::shared_ptr<Node::StructScope> get_struct(const std::string& name);

    /**
     * @brief Iterates through the list of deferred types and verifies them.
     * Should be called by the global checker after all statements have been visited.
     *
     * @return true If the list is empty or all deferred types are valid.
     * @return false If any deferred type is found to be invalid.
     */
    bool verify_deferred_types();

    /**
     * @brief Resets the environment to its initial state.
     * Useful for testing purposes.
     *
     */
    void reset();
};

#endif // ENVIRONMENT_H
