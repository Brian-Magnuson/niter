#ifndef NODE_H
#define NODE_H

#include "../utility/core.h"

#include "../scanner/token.h"
#include "../utility/dictionary.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief A base class for a scope in the namespace tree.
 * A scope is a node that can contain other nodes.
 *
 */
class Node::Scope : public virtual Node {
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
    std::shared_ptr<Node> upward_lookup(const std::string& name);

    /**
     * @brief Perform a downward lookup for a node with the given path.
     * A downward lookup starts at the current scope and attempts to follow a path to the desired node.
     * If lookup fails at any point along the path, the lookup is restarted from the parent scope.
     *
     * @param path The path to the node to look for. A path is a series of namespaces followed by a name.
     * E.g. for A::B::c, the path is {"A", "B", "c"}.
     * @return std::shared_ptr<Node> A shared pointer to the node if it is found, or nullptr if it is not found.
     */
    std::shared_ptr<Node> downward_lookup(const std::vector<std::string>& path);
};

/**
 * @brief A base class for a node that has a location in the source code.
 *
 */
class Node::Locatable : public virtual Node {
protected:
    Locatable() = default;

public:
    Location location;
};

/**
 * @brief A special scope that is the root of the namespace tree.
 * Its parent is always nullptr.
 *
 */
class Node::RootScope : public Node::Scope {
public:
    RootScope() {
        unique_name = "";
    }
};

/**
 * @brief A scope that represents a namespace.
 * Namespaces can contain variables, structs, local scopes, and other namespaces.
 *
 */
class Node::NamespaceScope : public Node::Scope, public Node::Locatable {
public:
    NamespaceScope(const Location& location, std::shared_ptr<Scope> parent, const std::string& name) {
        this->location = location;
        this->parent = parent;
        unique_name = parent->unique_name + "::" + name;
    }
};

/**
 * @brief A scope that represents a struct.
 * Has an extra map for instance members.
 * Its children represent static members.
 * Structs can contain variables, local scopes, and other structs, but not namespaces.
 * Note: Instance members are not stored as Nodes;
 * This is because instance members are not part of the namespace tree.
 *
 */
class Node::StructScope : public Node::Scope, public Node::Locatable {
public:
    // The instance members of the struct
    Dictionary<std::string, Decl::VarDeclarable*> instance_members;

    llvm::Type* ir_type = nullptr;
    bool is_primitive = false;

    StructScope(const Location& location, std::shared_ptr<Scope> parent, const std::string& name, llvm::Type* llvm_type = nullptr);
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
        unique_name = parent->unique_name + "::" + std::to_string(local_scope_count++);
    }
};

/**
 * @brief A node that represents a variable.
 * Despite the name "Variable", this class is also used for function pointers.
 * Variables may not have children, though they still track their parent scope.
 * Variables store a raw pointer to the AST declaration node that created them.
 * The variable's name, type, and declarer are stored in the declaration node.
 *
 */
class Node::Variable : public Node::Locatable {
public:
    // The declaration node that created this variable
    Decl::VarDeclarable* decl;
    // The LLVM value that represents the memory location of the variable; typically, this is either an llvm::GlobalVariable or an llvm::AllocaInst; call CreateLoad to get the actual value
    llvm::Value* llvm_allocation;

    Variable(
        std::shared_ptr<Scope> parent,
        Decl::VarDeclarable* declaration
    );
};

#endif // NODE_H
