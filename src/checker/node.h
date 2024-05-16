#ifndef NODE_H
#define NODE_H

#include "../parser/annotation.h"
#include "../scanner/token.h"
#include "type.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief A base class for nodes in the namespace tree.
 *
 */
class Node {
public:
    class Scope;
    class RootScope;
    class NamespaceScope;
    class StructScope;
    class LocalScope;
    class Variable;

    virtual ~Node() = default;

    // static int local_scope_count;

    // A unique name for this node. Used for type comparison and LLVM IR generation.
    std::string unique_name;
    // The parent scope of this node. This is never a variable since variables do not have children.
    std::shared_ptr<Scope> parent = nullptr;
};

/**
 * @brief A base class for a scope in the namespace tree.
 * A scope is a node that can contain other nodes.
 *
 */
class Node::Scope : public Node {
public:
    virtual ~Scope() = default;

    // The children of this scope. The key is the name of the child.
    std::unordered_map<std::string, std::shared_ptr<Node>> children;

    /**
     * @brief Perform an upward lookup for a node with the given name.
     * Should only be called with a name that is not a path.
     * An upward lookup starts at the current scope and goes up the tree until it finds a node with the given name.
     *
     * @param name The name of the node to look for.
     * @return std::shared_ptr<Node> A shared pointer to the node if it is found, or nullptr if it is not found.
     */
    std::shared_ptr<Node> upward_lookup(const std::string& name) {
        if (children.find(name) != children.end()) {
            return children[name];
        } else if (parent != nullptr) {
            return parent->upward_lookup(name);
        } else {
            return nullptr;
        }
    }

    /**
     * @brief Perform a downward lookup for a node with the given path.
     * A downward lookup starts at the current scope and attempts to follow a path to the desired node.
     * If lookup fails at any point along the path, the lookup is restarted from the parent scope.
     *
     * @param path The path to the node to look for. A path is a series of namespaces followed by a name.
     * E.g. for A::B::c, the path is {"A", "B", "c"}.
     * @return std::shared_ptr<Node> A shared pointer to the node if it is found, or nullptr if it is not found.
     */
    std::shared_ptr<Node> downward_lookup(const std::vector<std::string>& path) {
        if (path.empty()) {
            return nullptr;
        }

        bool found = true;
        Scope* current = this;
        // E.g. if the identifier is A::B::c, attempt to enter scope A, then B, then find c.
        // Iterate through all but the last element in the path. E.g. for A::B::c, iterate through A and B.
        for (unsigned i = 0; i < path.size() - 1; i++) {
            // See if 'A' exists in the current scope.
            if (current->children.find(path[i]) == current->children.end()) {
                found = false;
                break;
            }
            // If 'A' exists, enter the scope.
            current = dynamic_cast<Scope*>(current->children[path[i]].get());
            // If 'A' turns out not to be a scope, then the path is invalid.
            if (current == nullptr) {
                found = false;
                break;
            }
            // Else, continue and start looking for 'B'.
        }
        // If we didn't find 'A' or 'B', or 'A::B' does not contain 'c', then try again from the parent scope.
        if (!found || current->children.find(path.back()) == current->children.end()) {
            return parent->downward_lookup(path);
        }
        // If we found 'A::B::c', return it.
        return current->children[path.back()];
    };
};

/**
 * @brief A special scope that is the root of the namespace tree.
 * Its parent is always nullptr.
 *
 */
class Node::RootScope : public Node::Scope {
public:
    RootScope() {
        parent = nullptr;
        unique_name = "";
    }
};

/**
 * @brief A scope that represents a namespace.
 * Namespaces can contain variables, structs, local scopes, and other namespaces.
 *
 */
class Node::NamespaceScope : public Node::Scope {
public:
    NamespaceScope(std::shared_ptr<Scope> parent, const std::string& name) {
        this->parent = parent;
        unique_name = parent->unique_name + "::" + name;
    }
};

/**
 * @brief A scope that represents a struct.
 * Has an extra map for instance members.
 * Its children represent static members.
 * Structs can contain variables, local scopes, and other structs, but not namespaces.
 *
 */
class Node::StructScope : public Node::Scope {
public:
    std::unordered_map<std::string, std::shared_ptr<Node>> instance_members;

    StructScope(std::shared_ptr<Scope> parent, const std::string& name) {
        this->parent = parent;
        unique_name = parent->unique_name + "::" + name;
    }
};

/**
 * @brief A scope that represents a local scope.
 * Local scopes are used for blocks of code, such as function bodies.
 * They are not permanent members of the namespace tree.
 * They can contain variables and other local scopes.
 * Local scopes do not have names and global scopes do not track local scopes as children.
 * As a result, when a current scope moves out of a local scope, the local scope becomes inaccessible.
 *
 */
class Node::LocalScope : public Node::Scope {
public:
    LocalScope(std::shared_ptr<Scope> parent) {
        this->parent = parent;
        // unique_name = parent->unique_name + std::to_string(local_scope_count++) + "__";
        unique_name = parent->unique_name + "::local";
    }
};

/**
 * @brief A node that represents a variable.
 * Despite the name "Variable", this class is also used for function pointers.
 * Variables may not have children, though they still track their parent scope.
 * Variables may have an annotation, which is used to store type information.
 *
 */
class Node::Variable : public Node {
public:
    TokenType declarer;
    std::shared_ptr<Type> type;

    Variable(
        std::shared_ptr<Scope> parent,
        TokenType declarer,
        std::shared_ptr<Type> type,
        const std::string& name
    ) : declarer(declarer), type(type) {
        this->parent = parent;
        unique_name = parent->unique_name + "::" + name;
    }
};

#endif // NODE_H
