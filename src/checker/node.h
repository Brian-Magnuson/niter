#ifndef NODE_H
#define NODE_H

#include "../parser/annotation.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Node {
public:
    class Scope;
    class RootScope;
    class NamespaceScope;
    class StructScope;
    class LocalScope;
    class Variable;

    virtual ~Node() = default;

    std::shared_ptr<Scope> parent = nullptr;
};

class Node::Scope : public Node {
public:
    virtual ~Scope() = default;

    std::unordered_map<std::string, std::shared_ptr<Node>> children;

    std::shared_ptr<Node> upward_lookup(const std::string& name) {
        if (children.find(name) != children.end()) {
            return children[name];
        } else if (parent != nullptr) {
            return parent->upward_lookup(name);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Node> downward_lookup(const std::vector<std::string>& path) {
        if (path.empty()) {
            return nullptr;
        }

        bool found = true;
        Scope* current = this;
        // E.g. if the identifier is A::B::c, attempt to enter scope A, then B, then find c.
        // Iterate through all but the last element in the path. E.g. for A::B::c, iterate through A and B.
        for (int i = 0; i < path.size() - 1; i++) {
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

class Node::RootScope : public Node::Scope {
public:
    RootScope() = default;
};

class Node::NamespaceScope : public Node::Scope {
public:
    NamespaceScope(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

class Node::StructScope : public Node::Scope {
public:
    std::unordered_map<std::string, std::shared_ptr<Annotation>> instance_members;

    StructScope(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

class Node::LocalScope : public Node::Scope {
public:
    LocalScope(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

class Node::Variable : public Node {
public:
    std::shared_ptr<Annotation> annotation;

    Variable(std::shared_ptr<Scope> parent) {
        this->parent = parent;
    }
};

#endif // NODE_H
