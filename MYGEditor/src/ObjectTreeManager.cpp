#include <myg/ObjectTreeManager.h>
#include <DreamPath.h>
#include <DMASTExpression.h>
#include <sstream>

namespace MYG {

ObjectTreeManager::ObjectTreeManager()
    : tree_(nullptr) {
}

ObjectTreeManager::~ObjectTreeManager() {
    // No cleanup needed - we don't own the tree
}

void ObjectTreeManager::SetObjectTree(DMCompiler::DMObjectTree* tree) {
    tree_ = tree;
    ClearCaches();
}

DMCompiler::DMObject* ObjectTreeManager::GetRoot() {
    if (!tree_) {
        return nullptr;
    }
    return tree_->GetRoot();
}

std::vector<DMCompiler::DMObject*> ObjectTreeManager::GetChildren(DMCompiler::DMObject* obj) {
    if (!obj || !tree_) {
        return {};
    }

    // Check cache first
    auto it = children_cache_.find(obj);
    if (it != children_cache_.end()) {
        return it->second;
    }

    // Build and cache the children list
    auto children = BuildChildrenList(obj);
    children_cache_[obj] = children;
    return children;
}

DMCompiler::DMObject* ObjectTreeManager::GetObjectByPath(const std::string& path) {
    if (!tree_) {
        return nullptr;
    }

    // Check cache first
    auto it = path_cache_.find(path);
    if (it != path_cache_.end()) {
        return it->second;
    }

    // Look up in the tree
    DMCompiler::DreamPath dream_path(path);
    DMCompiler::DMObject* obj = nullptr;
    
    if (tree_->TryGetDMObject(dream_path, &obj)) {
        // Cache the result
        path_cache_[path] = obj;
        return obj;
    }

    // Cache null result to avoid repeated lookups
    path_cache_[path] = nullptr;
    return nullptr;
}

void ObjectTreeManager::ClearCaches() {
    path_cache_.clear();
    children_cache_.clear();
}

std::vector<DMCompiler::DMObject*> ObjectTreeManager::BuildChildrenList(DMCompiler::DMObject* obj) {
    std::vector<DMCompiler::DMObject*> children;
    
    if (!obj || !tree_) {
        return children;
    }

    // Iterate through all objects in the tree and find children
    for (const auto& obj_ptr : tree_->AllObjects) {
        if (obj_ptr && obj_ptr->Parent == obj) {
            children.push_back(obj_ptr.get());
        }
    }

    return children;
}

std::string ObjectTreeManager::GetVariable(DMCompiler::DMObject* obj, const std::string& var_name) {
    if (!obj || !tree_) {
        return "";
    }

    // Get the variable from the object (searches up inheritance chain)
    const DMCompiler::DMVariable* var = obj->GetVariable(var_name);
    if (!var) {
        return "";
    }

    // Try to extract a string representation of the default value
    if (var->Value) {
        return ExtractValueString(var->Value);
    }

    // No default value specified
    return "";
}

std::string ObjectTreeManager::ExtractValueString(DMCompiler::DMASTExpression* expr) {
    if (!expr) {
        return "";
    }

    // Try to cast to known constant types
    if (auto* int_const = dynamic_cast<DMCompiler::DMASTConstantInteger*>(expr)) {
        return std::to_string(int_const->Value);
    }
    
    if (auto* float_const = dynamic_cast<DMCompiler::DMASTConstantFloat*>(expr)) {
        return std::to_string(float_const->Value);
    }
    
    if (auto* string_const = dynamic_cast<DMCompiler::DMASTConstantString*>(expr)) {
        return string_const->Value;
    }
    
    if (auto* resource_const = dynamic_cast<DMCompiler::DMASTConstantResource*>(expr)) {
        return "'" + resource_const->Path + "'";
    }
    
    if (auto* path_const = dynamic_cast<DMCompiler::DMASTConstantPath*>(expr)) {
        return path_const->Path.Path.ToString();
    }
    
    if (dynamic_cast<DMCompiler::DMASTConstantNull*>(expr)) {
        return "null";
    }

    // For other expression types, return empty string
    // A full implementation would evaluate more complex expressions
    return "";
}

std::vector<std::pair<std::string, std::string>> ObjectTreeManager::GetAllVariables(DMCompiler::DMObject* obj) {
    std::vector<std::pair<std::string, std::string>> result;
    
    if (!obj || !tree_) {
        return result;
    }

    // Use the DMObjectTree's GetAllVariables method which handles inheritance
    auto all_vars = tree_->GetAllVariables(obj);
    
    // Convert to vector of pairs with extracted values
    for (const auto& [name, var_ptr] : all_vars) {
        if (var_ptr) {
            std::string value = "";
            if (var_ptr->Value) {
                value = ExtractValueString(var_ptr->Value);
            }
            result.push_back({name, value});
        }
    }
    
    return result;
}

} // namespace MYG
