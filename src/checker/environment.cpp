#include "environment.h"
#include "../utility/utils.h"

ErrorCode Environment::add_namespace(const std::string& name) {

    auto namespace_scope = std::dynamic_pointer_cast<Node::NamespaceScope>(current_scope);

    if (namespace_scope != nullptr) {
        auto new_scope = std::make_shared<Node::NamespaceScope>(namespace_scope);
        namespace_scope->children[name] = new_scope;
        current_scope = new_scope;
        return (ErrorCode)0;
    } else if (IS_TYPE(current_scope, Node::StructScope)) {
        return E_NAMESPACE_IN_STRUCT;
    } else {
        return E_NAMESPACE_IN_LOCAL_SCOPE;
    }
}

ErrorCode Environment::add_struct(const std::string& name) {
    if (IS_TYPE(current_scope, Node::LocalScope)) {
        return E_STRUCT_IN_LOCAL_SCOPE;
    } else {
        if (HAS_KEY(current_scope->children, name)) {
            return E_STRUCT_ALREADY_DECLARED;
        } else {
            auto new_scope = std::make_shared<Node::StructScope>(current_scope);
            current_scope->children[name] = new_scope;
            current_scope = new_scope;
            return (ErrorCode)0;
        }
    }
}

void Environment::install_primitive_types() {
    std::vector<std::string> primitive_types = {
        "i32",
        "f64",
        "bool",
        "char",
        "void",
    };
    for (auto& type : primitive_types) {
        global_tree->children[type] = std::make_shared<Node::StructScope>(global_tree);
    }
}

void Environment::increase_local_scope() {
    auto local_scope = std::make_shared<Node::LocalScope>(current_scope);
    current_scope = local_scope;
}

ErrorCode Environment::exit_scope() {
    if (current_scope->parent == nullptr) {
        return E_EXITED_ROOT_SCOPE;
    } else {
        current_scope = current_scope->parent;
        return (ErrorCode)0;
    }
}

ErrorCode Environment::declare_symbol(const std::string& name, std::shared_ptr<Annotation> type) {
    if (HAS_KEY(current_scope->children, name)) {
        return E_SYMBOL_ALREADY_DECLARED;
    } else {
        current_scope->children[name] = std::make_shared<Node::Variable>(type);
        return (ErrorCode)0;
    }
}

bool Environment::verify_type(const std::shared_ptr<Annotation>& type, bool allow_deferral, std::shared_ptr<Node::Scope> from_scope) {
    if (from_scope == nullptr) {
        from_scope = current_scope;
    }

    if (IS_TYPE(type, Annotation::Function)) {
        auto fun_type = std::dynamic_pointer_cast<Annotation::Function>(type);
        auto ret = verify_type(fun_type->ret, allow_deferral, from_scope);
        if (!ret) {
            return false;
        }
        for (auto& param : fun_type->params) {
            ret = verify_type(param.second, allow_deferral, from_scope);
            if (!ret) {
                return false;
            }
        }
        return true;
    } else if (IS_TYPE(type, Annotation::Tuple)) {
        auto tuple_type = std::dynamic_pointer_cast<Annotation::Tuple>(type);
        for (auto& elem : tuple_type->elements) {
            auto ret = verify_type(elem, allow_deferral, from_scope);
            if (!ret) {
                return false;
            }
        }
        return true;
    } else if (IS_TYPE(type, Annotation::Array)) {
        auto array_type = std::dynamic_pointer_cast<Annotation::Array>(type);
        return verify_type(array_type->name, allow_deferral, from_scope);
    } else if (IS_TYPE(type, Annotation::Pointer)) {
        auto pointer_type = std::dynamic_pointer_cast<Annotation::Pointer>(type);
        return verify_type(pointer_type->name, allow_deferral, from_scope);
    } else if (IS_TYPE(type, Annotation::Segmented)) {
        auto segmented_type = std::dynamic_pointer_cast<Annotation::Segmented>(type);
        std::vector<std::string> path;
        for (auto& class_ : segmented_type->classes) {
            path.push_back(class_->name);
            // If any of the type arguments are invalid, return false.
            for (auto& type_arg : class_->type_args) {
                if (!verify_type(type_arg, allow_deferral, from_scope)) {
                    return false;
                }
            }
        }
        auto node = from_scope->downward_lookup(path);
        // If allow_deferral is true, we can defer the type if it is not found.
        if (node != nullptr) {
            return true;
        } else if (allow_deferral) {
            deferred_types.push_back({type, from_scope});
            return true;
        } else {
            return false;
        }
    }
}

std::shared_ptr<Annotation> Environment::get_type(std::shared_ptr<Expr::Identifier> identifier) {
    std::shared_ptr<Node> found_node = nullptr;
    // If the identifier is a single token, we can look up the type in the global scope.
    if (identifier->tokens.size() == 1) {
        found_node = current_scope->upward_lookup(identifier->tokens[0].lexeme);
    }
    // If the first lookup failed or was not attempted, we perform a downward lookup.
    if (found_node == nullptr) {
        // TODO: Check later to see if this works. If it doesn't, this function will return nullptr and it'll appear as if the symbol was not found.
        std::vector<std::string> path;
        for (auto& token : identifier->tokens) {
            path.push_back(token.lexeme);
        }
        found_node = current_scope->downward_lookup(path);
    }

    if (found_node == nullptr) {
        return nullptr;
    }

    auto found_var = std::dynamic_pointer_cast<Node::Variable>(found_node);

    if (found_var != nullptr) {
        return found_var->annotation;
    } else {
        return nullptr;
    }
}

bool Environment::verify_deferred_types() {
    for (auto& deferred_type : deferred_types) {
        if (!verify_type(deferred_type.first, false, deferred_type.second)) {
            return false;
        }
    }
    return true;
}

void Environment::reset() {
    global_tree = std::make_shared<Node::RootScope>();
    current_scope = global_tree;
    install_primitive_types();
}