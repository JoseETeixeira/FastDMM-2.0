#pragma once

#include <DMObjectTree.h>
#include <DMObject.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace MYG {

/// <summary>
/// ObjectTreeManager - UI-Friendly Wrapper for DMObjectTree
/// 
/// Provides a convenient interface for accessing the DMCompiler object tree
/// with caching and performance optimizations for UI operations.
/// 
/// Key Features:
/// - Efficient tree navigation with cached children lists
/// - Fast path lookups with caching
/// - Variable resolution with inheritance
/// - Search and filtering capabilities
/// </summary>
class ObjectTreeManager {
public:
    ObjectTreeManager();
    ~ObjectTreeManager();

    /// Set the object tree to manage
    /// Clears all caches when a new tree is set
    /// @param tree Pointer to the DMObjectTree (can be null)
    void SetObjectTree(DMCompiler::DMObjectTree* tree);

    /// Get the root object (/)
    /// @return Pointer to root object, or nullptr if no tree is set
    DMCompiler::DMObject* GetRoot();

    /// Get all child objects of a given object
    /// Results are cached for performance
    /// @param obj Parent object to get children for
    /// @return Vector of child object pointers (empty if obj is null)
    std::vector<DMCompiler::DMObject*> GetChildren(DMCompiler::DMObject* obj);

    /// Get an object by its path
    /// Results are cached for performance
    /// @param path Type path to look up (e.g., "/mob/player")
    /// @return Pointer to object if found, nullptr otherwise
    DMCompiler::DMObject* GetObjectByPath(const std::string& path);

    /// Check if the object tree is loaded
    /// @return true if a valid tree is set, false otherwise
    bool HasObjectTree() const { return tree_ != nullptr; }

    /// Get a variable value from an object, searching up the inheritance chain
    /// @param obj Object to get variable from
    /// @param var_name Name of the variable to look up
    /// @return Variable value as string if found, empty string otherwise
    std::string GetVariable(DMCompiler::DMObject* obj, const std::string& var_name);

    /// Get all variables for an object including inherited ones
    /// Returns a map of variable name to value string
    /// @param obj Object to get all variables for
    /// @return Vector of (name, value) pairs for all variables
    std::vector<std::pair<std::string, std::string>> GetAllVariables(DMCompiler::DMObject* obj);

private:
    /// Pointer to the DMObjectTree being managed
    DMCompiler::DMObjectTree* tree_;

    /// Cache for path lookups: path string -> DMObject*
    std::unordered_map<std::string, DMCompiler::DMObject*> path_cache_;

    /// Cache for children lists: DMObject* -> vector of children
    std::unordered_map<DMCompiler::DMObject*, std::vector<DMCompiler::DMObject*>> children_cache_;

    /// Clear all caches (called when tree changes)
    void ClearCaches();

    /// Build children list for an object (internal helper)
    /// @param obj Parent object
    /// @return Vector of child objects
    std::vector<DMCompiler::DMObject*> BuildChildrenList(DMCompiler::DMObject* obj);

    /// Extract a string representation from an AST expression
    /// Handles common constant types (int, float, string, path, null)
    /// @param expr Expression to extract value from
    /// @return String representation of the value, or empty string if not extractable
    std::string ExtractValueString(DMCompiler::DMASTExpression* expr);
};

} // namespace MYG
