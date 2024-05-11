#ifndef SCOPE_H
#define SCOPE_H

#include "../parser/annotation.h"
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief A node class representing a scope in the namespace tree.
 *
 */
class Scope {
public:
    class Global;
    class Root;
    class Namespace;
    class Struct;
    class Local;

    virtual ~Scope() = default;

    // The parent scope of the scope. If the scope is the root, the parent is nullptr.
    std::shared_ptr<Scope> parent = nullptr;

    // The symbol table of the scope.
    std::unordered_map<std::string, std::shared_ptr<Annotation>> symbol_table;
};

/**
 * @brief A base class representing a scope in the namespace tree.
 * Can be a global scope, a namespace scope, or a struct scope.
 * All global scopes have children.
 * This class is not meant to be instantiated directly.
 *
 */
class Scope::Global : public Scope {
protected:
    Global() = default;

public:
    // The children of the scope.
    std::unordered_map<std::string, std::shared_ptr<Scope::Global>> children;
};

/**
 * @brief The root scope of the namespace tree.
 * It is similar to a namespace scope, but it has no parent.
 * There should only be one root scope in the namespace tree at the root.
 *
 */
class Scope::Root : public Scope::Global {
public:
    Root() = default;
};

/**
 * @brief A namespace scope in the namespace tree.
 * Its children may be namespaces, structs, or local scopes.
 *
 */
class Scope::Namespace : public Scope::Global {
public:
    Namespace(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

/**
 * @brief A struct scope in the namespace tree.
 * Its children may be structs or local scopes, but not namespaces.
 * Its symbol table contains the static members of the struct.
 * It has an additional symbol table for the instance members.
 *
 */
class Scope::Struct : public Scope::Global {
public:
    // The parent scope of the struct.
    std::shared_ptr<Scope> parent;
    // The non-static members of the struct.
    std::unordered_map<std::string, std::shared_ptr<Annotation>> instance_members;

    Struct(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

/**
 * @brief A local scope.
 * Local scopes may not have children and are not permanent nodes in the namespace tree.
 * Instead, they are created when a local scope is entered and destroyed when it is exited.
 * They keep a reference to their parent scope.
 *
 */
class Scope::Local : public Scope {
public:
    // The parent scope of the local scope.
    std::shared_ptr<Scope> parent;

    Local(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

#endif // SCOPE_H
