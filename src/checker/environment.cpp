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

std::pair<std::shared_ptr<Node::Variable>, ErrorCode> Environment::declare_variable(const std::string& name, TokenType declarer, std::shared_ptr<Annotation> annotation, bool allow_deferral) {
    if (HAS_KEY(current_scope->children, name)) {
        return {nullptr, E_SYMBOL_ALREADY_DECLARED};
    } else {
        std::shared_ptr<Type> type = get_type(annotation);
        if (type == nullptr && allow_deferral) {
            deferred_variables.push_back(DeferredVariable{
                annotation,
                current_scope,
                name,
                declarer,
            });
            return {nullptr, (ErrorCode)0};
        } else if (type == nullptr) {
            return {nullptr, E_UNKNOWN_TYPE};
        } else {
            // If the type is a pointer, set the declarer to the token type that declared the variable.
            auto ptr_type = std::dynamic_pointer_cast<Type::Pointer>(type);
            if (ptr_type != nullptr) {
                // This is to prevent users from mutating the object that the pointer points to.
                ptr_type->declarer = declarer;
            }
            auto new_variable = std::make_shared<Node::Variable>(current_scope, declarer, type, name);
            current_scope->children[name] = new_variable;
            return {new_variable, (ErrorCode)0};
        }
    }
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

std::shared_ptr<Type> Environment::get_type(const std::shared_ptr<Annotation>& annotation, std::shared_ptr<Node::Scope> from_scope) {
    if (from_scope == nullptr) {
        from_scope = current_scope;
    }
    // If the annotation is "auto", return a blank type.
    if (annotation->to_string() == "auto") {
        return std::make_shared<Type::Blank>();
    }

    if (IS_TYPE(annotation, Annotation::Segmented)) {
        // Annotations of the form `t<t>::t<t>`
        auto segmented_annotation = std::dynamic_pointer_cast<Annotation::Segmented>(annotation);
        // Start building the path to the type.
        std::vector<std::string> path;
        for (auto& class_ : segmented_annotation->classes) {
            path.push_back(class_->name);
            // Also verify each type argument in the segmented type.
            for (auto& type_arg : class_->type_args) {
                auto ret = get_type(type_arg, from_scope);
                // If any of the type arguments are invalid, return nullptr.
                if (ret == nullptr) {
                    return nullptr;
                }
            }
        }
        // Lookup the type in the global tree.
        // Note: only downward lookup is performed here. We may change this later.
        auto found_node = from_scope->downward_lookup(path);
        auto found_struct = std::dynamic_pointer_cast<Node::StructScope>(found_node);
        return found_struct != nullptr ? std::make_shared<Type::Struct>(found_struct) : nullptr;
    } else if (IS_TYPE(annotation, Annotation::Function)) {
        // Annotations of the form `fun(t, t) => t`
        auto fun_annotation = std::dynamic_pointer_cast<Annotation::Function>(annotation);
        std::vector<std::pair<TokenType, std::shared_ptr<Type>>> params;
        // Verify each parameter type in the function type.
        for (auto& param : fun_annotation->params) {
            auto param_type = get_type(param.second, from_scope);
            // If any of the parameter types are invalid, return nullptr.
            if (param_type == nullptr) {
                return nullptr;
            }
            params.push_back({param.first ? KW_VAR : KW_CONST, param_type});
        }
        // Verify the return type of the function.
        auto ret_type = get_type(fun_annotation->return_annotation, from_scope);
        if (ret_type == nullptr) {
            return nullptr;
        }
        return std::make_shared<Type::Function>(
            params,
            fun_annotation->return_declarer,
            ret_type
        );
    } else if (IS_TYPE(annotation, Annotation::Tuple)) {
        // Annotations of the form `(t, t, t)`
        auto tuple_annotation = std::dynamic_pointer_cast<Annotation::Tuple>(annotation);
        std::vector<std::shared_ptr<Type>> elements;
        for (auto& elem : tuple_annotation->elements) {
            auto ret = get_type(elem, from_scope);
            if (ret == nullptr) {
                return nullptr;
            }
            elements.push_back(ret);
        }
        return std::make_shared<Type::Tuple>(elements);
    } else if (IS_TYPE(annotation, Annotation::Array)) {
        // Annotations of the form `t[]`
        auto array_annotation = std::dynamic_pointer_cast<Annotation::Array>(annotation);
        auto ret = get_type(array_annotation->name, from_scope);
        if (ret == nullptr) {
            return nullptr;
        }
        return std::make_shared<Type::Array>(ret);
    } else if (IS_TYPE(annotation, Annotation::Pointer)) {
        // Annotations of the form `t*`
        auto ptr_annotation = std::dynamic_pointer_cast<Annotation::Pointer>(annotation);
        auto ret = get_type(ptr_annotation->name, from_scope);
        if (ret == nullptr) {
            return nullptr;
        }
        return std::make_shared<Type::Pointer>(ret);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Type> Environment::get_type(const std::string& name) {
    auto annotation = std::make_shared<Annotation::Segmented>(name);
    return get_type(annotation);
}

bool Environment::verify_deferred_types() {
    for (auto& deferred_variable : deferred_variables) {
        auto [variable, error] = declare_variable(deferred_variable.name, deferred_variable.declarer, deferred_variable.annotation, false);
        if (error != 0) {
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
