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
    class Root;
    class Namespace;
    class Struct;
    class Local;

    virtual ~Scope() = default;

    // The symbol table of the scope.
    std::unordered_map<std::string, std::shared_ptr<Annotation>> symbol_table;
};

/**
 * @brief The root scope of the namespace tree.
 * It is similar to a namespace scope, but it has no parent.
 * There should only be one root scope in the namespace tree at the root.
 *
 */
class Scope::Root : public Scope {
public:
    // The global namespaces contained in the root.
    std::unordered_map<std::string, std::shared_ptr<Scope>> children;

    Root() = default;
};

/**
 * @brief A namespace scope in the namespace tree.
 * Its children may be namespaces, structs, or local scopes.
 *
 */
class Scope::Namespace : public Scope {
public:
    // The parent scope of the namespace.
    std::shared_ptr<Scope> parent;
    // The global scopes contained in the namespace.
    std::unordered_map<std::string, std::shared_ptr<Scope>> children;

    Namespace(std::shared_ptr<Scope> parent)
        : parent(parent) {}
};

/**
 * @brief A struct scope in the namespace tree.
 * Its children may be structs or local scopes, but not namespaces.
 * Its symbol table contains the static members of the struct.
 * It has an additional symbol table for the instance members.
 *
 */
class Scope::Struct : public Scope {
public:
    // The parent scope of the struct.
    std::shared_ptr<Scope> parent;
    // The structs contained in the struct.
    std::unordered_map<std::string, std::shared_ptr<Scope>> children;
    // The non-static members of the struct.
    std::unordered_map<std::string, std::shared_ptr<Annotation>> instance_members;

    Struct(std::shared_ptr<Scope> parent)
        : parent(parent) {}
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

    Local(std::shared_ptr<Scope> parent)
        : parent(parent) {}
};

#endif // SCOPE_H
