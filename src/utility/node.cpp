#include "node.h"

#include "../checker/environment.h"

int Node::local_scope_count = 0;

std::shared_ptr<Node> Node::Scope::upward_lookup(const std::string& name) {
    if (children.find(name) != children.end()) {
        return children[name];
    } else if (parent != nullptr) {
        return parent->upward_lookup(name);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Node> Node::Scope::downward_lookup(const std::vector<std::string>& path) {
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
        return parent != nullptr ? parent->downward_lookup(path) : nullptr;
    }
    // If we found 'A::B::c', return it.
    return current->children[path.back()];
}

Node::StructScope::StructScope(const Location& location, std::shared_ptr<Scope> parent, const std::string& name, llvm::Type* llvm_type) {
    this->location = location;
    this->parent = parent;
    unique_name = parent->unique_name + "::" + name;
    // The struct type will be provided for primitive types.
    ir_type = llvm_type;

    // If the struct type is not provided, create a new one.
    if (ir_type == nullptr) {
        auto llvm_safe_name = unique_name;
        std::replace(llvm_safe_name.begin(), llvm_safe_name.end(), ':', '_');

        ir_type = llvm::StructType::create(*Environment::inst().get_llvm_context(), llvm_safe_name);
    }
}

Node::Variable::Variable(
    std::shared_ptr<Scope> parent,
    Decl::VarDeclarable* declaration
) : decl(declaration) {
    this->location = declaration->location;
    this->parent = parent;
    unique_name = parent->unique_name + "::" + declaration->name.lexeme;
}
