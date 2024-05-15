#include "environment.h"
#include "../utility/utils.h"

ErrorCode Environment::add_namespace(const std::string& name) {

    auto namespace_scope = std::dynamic_pointer_cast<Node::NamespaceScope>(current_scope);

    if (namespace_scope != nullptr) {
        auto new_scope = std::make_shared<Node::NamespaceScope>(namespace_scope, name);
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
            auto new_scope = std::make_shared<Node::StructScope>(current_scope, name);
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
        global_tree->children[type] = std::make_shared<Node::StructScope>(global_tree, type);
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

void Environment::exit_all_local_scopes() {
    while (!IS_TYPE(current_scope, Node::RootScope)) {
        exit_scope();
    }
}

bool Environment::in_global_scope() {
    return !IS_TYPE(current_scope, Node::LocalScope);
}

ErrorCode Environment::declare_variable(const std::string& name, TokenType declarer, std::shared_ptr<Annotation> type) {
    if (HAS_KEY(current_scope->children, name)) {
        return E_SYMBOL_ALREADY_DECLARED;
    } else {
        current_scope->children[name] = std::make_shared<Node::Variable>(current_scope, declarer, type, name);
        return (ErrorCode)0;
    }
}

bool Environment::verify_type(const std::shared_ptr<Annotation>& type, bool allow_deferral, std::shared_ptr<Node::Scope> from_scope) {
    if (from_scope == nullptr) {
        from_scope = current_scope;
    }

    if (IS_TYPE(type, Annotation::Function)) {
        // Annotations of the form `fun(t, t) -> t`
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
        // Annotations of the form `(t, t, t)`
        auto tuple_type = std::dynamic_pointer_cast<Annotation::Tuple>(type);
        for (auto& elem : tuple_type->elements) {
            auto ret = verify_type(elem, allow_deferral, from_scope);
            if (!ret) {
                return false;
            }
        }
        return true;
    } else if (IS_TYPE(type, Annotation::Array)) {
        // Annotations of the form `t[]`
        auto array_type = std::dynamic_pointer_cast<Annotation::Array>(type);
        return verify_type(array_type->name, allow_deferral, from_scope);
    } else if (IS_TYPE(type, Annotation::Pointer)) {
        // Annotations of the form `t*`
        auto pointer_type = std::dynamic_pointer_cast<Annotation::Pointer>(type);
        return verify_type(pointer_type->name, allow_deferral, from_scope);
    } else if (IS_TYPE(type, Annotation::Segmented)) {
        // Annotations of the form `t<t>::t<t>`
        auto segmented_type = std::dynamic_pointer_cast<Annotation::Segmented>(type);
        std::vector<std::string> path;
        // Verify each type argument in the segmented type.
        for (auto& class_ : segmented_type->classes) {
            path.push_back(class_->name);
            // If any of the type arguments are invalid, return false.
            for (auto& type_arg : class_->type_args) {
                if (!verify_type(type_arg, allow_deferral, from_scope)) {
                    return false;
                }
            }
        }
        // If there is only one class in the segmented type, we can perform an upward lookup.
        std::shared_ptr<Node> node = nullptr;
        if (path.size() == 1) {
            node = from_scope->upward_lookup(path[0]);
        }
        if (node == nullptr) {
            node = from_scope->downward_lookup(path);
        }
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

    // Unreachable
    return false;
}

std::shared_ptr<Node::Variable> Environment::get_variable(const Expr::Identifier* identifier) {
    std::shared_ptr<Node> found_node = nullptr;
    // If the identifier is a single token, we can look up the variable in the global scope.
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

    return std::dynamic_pointer_cast<Node::Variable>(found_node);
}

std::shared_ptr<Node::Variable> Environment::get_instance_variable(std::shared_ptr<Type::Struct> instance_type, const std::string& member_name) {
    auto struct_node = instance_type->struct_scope;
    auto found_node_iter = struct_node->children.find(member_name);
    if (found_node_iter != struct_node->children.end()) {
        return std::dynamic_pointer_cast<Node::Variable>(found_node_iter->second);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Node::StructScope> Environment::get_struct(std::shared_ptr<Annotation::Segmented> type) {
    std::vector<std::string> path;
    for (auto& class_ : type->classes) {
        path.push_back(class_->name);
    }
    auto found_node = current_scope->downward_lookup(path);
    return std::dynamic_pointer_cast<Node::StructScope>(found_node);
}

std::shared_ptr<Node::StructScope> Environment::get_struct(const std::string& name) {
    auto found_node = current_scope->upward_lookup(name);
    return std::dynamic_pointer_cast<Node::StructScope>(found_node);
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
