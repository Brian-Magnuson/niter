#include "environment.h"
#include "../utility/utils.h"

ErrorCode Environment::add_namespace(const std::string& name) {

    auto namespace_scope = std::dynamic_pointer_cast<Scope::Namespace>(current_scope);

    if (namespace_scope != nullptr) {
        namespace_scope->children[name] = std::make_shared<Scope::Namespace>(namespace_scope);
        current_scope = namespace_scope->children[name];
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
            return (ErrorCode)0;
        }
    } else {
        return E_GLOBAL_TYPE;
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
        // If the current scope is a local scope, the local scope will be cleaned up since there will be no references to it.
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

void Environment::reset() {
    global_tree = std::make_shared<Scope::Root>();
    current_scope = global_tree;
}
