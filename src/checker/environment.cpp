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
            global_scope->children[name] = std::make_shared<Scope::Struct>();
            current_scope = global_scope->children[name];
            return (ErrorCode)0;
        }
    } else {
        return E_GLOBAL_TYPE;
    }
}
