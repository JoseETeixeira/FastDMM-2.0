#pragma once

#include <string>
#include <memory>
#include <functional>

namespace DMCompiler {
    class DMCompiler;
    class DMObjectTree;
}

namespace myg {

class ObjectTreeCache;

/**
 * Callback for compilation progress updates
 * @param message Status message
 * @param progress Progress value (0.0 to 1.0)
 */
using CompilationCallback = std::function<void(const std::string& message, float progress)>;

/**
 * ProjectManager - Manages DME project loading, compilation, and cache persistence
 * 
 * Responsibilities:
 * - Load .dme project files
 * - Invoke DMCompiler for compilation
 * - Manage object tree lifecycle
 * - Cache compiled object trees to disk
 * - Validate cache freshness against source files
 */
class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    /**
     * Load a DME project file
     * @param dme_path Path to the .dme file
     * @return true if project loaded successfully, false otherwise
     */
    bool LoadProject(const std::string& dme_path);

    /**
     * Compile the loaded project
     * @param callback Optional callback for progress updates
     * @return true if compilation succeeded, false otherwise
     */
    bool CompileProject(CompilationCallback callback = nullptr);

    /**
     * Check if a valid object tree is loaded
     * @return true if object tree is available, false otherwise
     */
    bool HasValidObjectTree() const;

    /**
     * Get the project file path
     * @return Path to the .dme file
     */
    const std::string& GetProjectPath() const { return dme_path_; }

    /**
     * Get the project directory
     * @return Directory containing the .dme file
     */
    const std::string& GetProjectDirectory() const { return project_directory_; }

    /**
     * Get the project name (filename without extension)
     * @return Project name
     */
    std::string GetProjectName() const;

    /**
     * Get the object tree
     * @return Pointer to DMObjectTree, or nullptr if not loaded
     */
    DMCompiler::DMObjectTree* GetObjectTree();

    /**
     * Get the last error message
     * @return Error message from last failed operation
     */
    const std::string& GetLastError() const { return last_error_; }

    /**
     * Get compilation messages (errors and warnings)
     * @return Vector of compiler messages
     */
    const std::vector<std::string>& GetCompilationMessages() const;

private:
    std::string dme_path_;
    std::string project_directory_;
    std::unique_ptr<DMCompiler::DMCompiler> compiler_;
    std::unique_ptr<ObjectTreeCache> cache_;
    bool object_tree_loaded_;
    std::string last_error_;

    /**
     * Load cached object tree if available and valid
     * @return true if cache loaded successfully, false otherwise
     */
    bool LoadCachedObjectTree();

    /**
     * Save object tree to cache
     * @return true if cache saved successfully, false otherwise
     */
    bool SaveObjectTreeCache();

    /**
     * Check if cache is valid (exists and newer than .dme file)
     * @return true if cache is valid, false otherwise
     */
    bool IsCacheValid() const;

    /**
     * Extract directory from file path
     * @param file_path Full path to file
     * @return Directory containing the file
     */
    std::string ExtractDirectory(const std::string& file_path) const;

    /**
     * Check if file exists
     * @param path Path to file
     * @return true if file exists, false otherwise
     */
    bool FileExists(const std::string& path) const;

    /**
     * Get file modification timestamp
     * @param path Path to file
     * @return Timestamp in seconds since epoch, or 0 if file doesn't exist
     */
    uint64_t GetFileTimestamp(const std::string& path) const;
};

} // namespace myg
