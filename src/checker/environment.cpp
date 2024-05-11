#include "environment.h"
#include "../utility/utils.h"

std::shared_ptr<Annotation> Environment::downward_lookup(std::shared_ptr<Expr::Identifier> identifier) {
    auto current = current_global_scope;

    std::shared_ptr<Annotation> found_type = nullptr;

    bool traversed = true;
    for (unsigned i = 0; i < identifier->tokens.size() - 1; i++) {
        // Look for the namespace in the children of the current scope.
        auto it = current->children.find(identifier->tokens[i].lexeme);
        if (it == current->children.end()) {
            // If the namespace is not found, return null.
            traversed = false;
            break;
        }
        // If the namespace is found, move to that scope.
        current = it->second;
    }

    if (traversed) {
        // Look for the symbol in the symbol table of the current scope.
        auto it = current->symbol_table.find(identifier->tokens.back().lexeme);
        if (it != current->symbol_table.end()) {
            found_type = it->second;
        }
    }

    if (found_type != nullptr) {
        return found_type;
    }

    // If the symbol was not found, we need to look from the root.
    current = global_tree;
    for (unsigned i = 0; i < identifier->tokens.size() - 1; i++) {
        auto it = current->children.find(identifier->tokens[i].lexeme);
        if (it == current->children.end()) {
            return nullptr;
        }
        current = it->second;
    }

    auto it = current->symbol_table.find(identifier->tokens.back().lexeme);
    if (it != current->symbol_table.end()) {
        found_type = it->second;
    }

    return found_type;
}

std::shared_ptr<Annotation> Environment::upward_lookup(std::shared_ptr<Expr::Identifier> identifier) {
    auto current = current_scope;

    std::shared_ptr<Annotation> found_type = nullptr;

    while (current != nullptr) {
        auto it = current->symbol_table.find(identifier->tokens[0].lexeme);
        if (it != current->symbol_table.end()) {
            found_type = it->second;
            break;
        }
        current = current->parent;
    }

    return found_type;
}

ErrorCode Environment::add_namespace(const std::string& name) {

    auto namespace_scope = std::dynamic_pointer_cast<Scope::Namespace>(current_scope);

    if (namespace_scope != nullptr) {
        namespace_scope->children[name] = std::make_shared<Scope::Namespace>(namespace_scope);
        current_scope = namespace_scope->children[name];
        current_global_scope = std::dynamic_pointer_cast<Scope::Global>(current_scope);
        return (ErrorCode)0;
    } else if (IS_TYPE(current_scope, Scope::Struct)) {
        return E_NAMESPACE_IN_STRUCT;
    } else {
        return E_NAMESPACE_IN_LOCAL_SCOPE;
    }
}

ErrorCode Environment::add_struct(const std::string& name) {
    if (IS_TYPE(current_scope, Scope::Local)) {
        return E_STRUCT_IN_LOCAL_SCOPE;
    } else if (IS_TYPE(current_scope, Scope::Global)) {
        auto global_scope = std::dynamic_pointer_cast<Scope::Global>(current_scope);
        if (HAS_KEY(global_scope->children, name)) {
            return E_STRUCT_ALREADY_DECLARED;
        } else {
            global_scope->children[name] = std::make_shared<Scope::Struct>(current_scope);
            current_scope = global_scope->children[name];
            current_global_scope = std::dynamic_pointer_cast<Scope::Global>(current_scope);
            return (ErrorCode)0;
        }
    } else {
        return E_GLOBAL_TYPE;
    }
}

void Environment::install_primitive_types() {
    auto global_scope = std::dynamic_pointer_cast<Scope::Global>(global_tree);
    std::vector<std::string> primitive_types = {
        "i32",
        "f64",
        "bool",
        "char",
        "void",
    };
    for (auto& type : primitive_types) {
        global_scope->children[type] = std::make_shared<Scope::Struct>(global_scope);
    }
}

void Environment::increase_local_scope() {
    auto local_scope = std::make_shared<Scope::Local>(current_scope);
    current_scope = local_scope;
}

ErrorCode Environment::exit_scope() {
    if (current_scope->parent == nullptr) {
        return E_EXITED_ROOT_SCOPE;
    } else {
        // If we are exiting a global scope, we need to remove the last element from the scope chain names.
        if (IS_TYPE(current_scope, Scope::Global)) {
            current_global_scope = std::dynamic_pointer_cast<Scope::Global>(current_scope->parent);
        }
        current_scope = current_scope->parent;
        return (ErrorCode)0;
    }
}

ErrorCode Environment::declare_symbol(const std::string& name, std::shared_ptr<Annotation> type) {
    if (HAS_KEY(current_scope->symbol_table, name)) {
        return E_SYMBOL_ALREADY_DECLARED;
    } else {
        current_scope->symbol_table[name] = type;
        return (ErrorCode)0;
    }
}

std::shared_ptr<Annotation> Environment::get_type(std::shared_ptr<Expr::Identifier> identifier) {
    std::shared_ptr<Annotation> found_type = nullptr;
    // If the identifier is a single token, we can look up the type in the global scope.
    if (identifier->tokens.size() == 1) {
        found_type = upward_lookup(identifier);
    }
    // If the first lookup failed or was not attempted, we perform a downward lookup.
    if (found_type == nullptr) {
        // TODO: Check later to see if this works. If it doesn't, this function will return nullptr and it'll appear as if the symbol was not found.
        found_type = downward_lookup(identifier);
    }
    return found_type;
}

void Environment::reset() {
    global_tree = std::make_shared<Scope::Root>();
    current_scope = global_tree;
    install_primitive_types();
}
