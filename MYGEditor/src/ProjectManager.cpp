#include "myg/ProjectManager.h"
#include "myg/ObjectTreeCache.h"
#include <DMCompiler.h>
#include <DMObjectTree.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace myg {

ProjectManager::ProjectManager()
    : object_tree_loaded_(false)
    , cache_(std::make_unique<ObjectTreeCache>())
{
}

ProjectManager::~ProjectManager() = default;

bool ProjectManager::LoadProject(const std::string& dme_path) {
    // Reset state
    dme_path_.clear();
    project_directory_.clear();
    compiler_.reset();
    object_tree_loaded_ = false;
    last_error_.clear();

    try {
        // Validate file path
        if (dme_path.empty()) {
            last_error_ = "Project path is empty";
            std::cerr << "Error: " << last_error_ << std::endl;
            return false;
        }

        // Check if file exists
        if (!FileExists(dme_path)) {
            last_error_ = "DME file not found: " + dme_path;
            std::cerr << "Error: " << last_error_ << std::endl;
            return false;
        }

        // Check file extension
        fs::path path(dme_path);
        if (path.extension() != ".dme") {
            last_error_ = "File is not a .dme file: " + dme_path;
            std::cerr << "Error: " << last_error_ << std::endl;
            return false;
        }

        // Store paths
        dme_path_ = dme_path;
        project_directory_ = ExtractDirectory(dme_path);

        std::cout << "Loaded project: " << GetProjectName() << std::endl;
        std::cout << "Project directory: " << project_directory_ << std::endl;

        // Try to load cached object tree
        if (LoadCachedObjectTree()) {
            std::cout << "Loaded object tree from cache" << std::endl;
            object_tree_loaded_ = true;
            return true;
        }

        std::cout << "No valid cache found, compilation required" << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        last_error_ = "Filesystem error: " + std::string(e.what());
        std::cerr << "Error loading project: " << last_error_ << std::endl;
        return false;
    } catch (const std::exception& e) {
        last_error_ = "Unexpected error: " + std::string(e.what());
        std::cerr << "Error loading project: " << last_error_ << std::endl;
        return false;
    }
}

bool ProjectManager::CompileProject(CompilationCallback callback) {
    try {
        if (dme_path_.empty()) {
            last_error_ = "No project loaded";
            std::cerr << "Error: " << last_error_ << std::endl;
            return false;
        }

        if (callback) {
            callback("Starting compilation...", 0.0f);
        }

        // Create compiler instance
        compiler_ = std::make_unique<DMCompiler::DMCompiler>();

        // Set up compiler settings
        DMCompiler::DMCompilerSettings settings;
        settings.Files.push_back(dme_path_);
        settings.Verbose = true;

        if (callback) {
            callback("Compiling project...", 0.3f);
        }

        // Compile the project
        std::cout << "Compiling project: " << dme_path_ << std::endl;
        bool success = compiler_->Compile(settings);

        if (callback) {
            callback(success ? "Compilation complete" : "Compilation failed", 1.0f);
        }

        if (!success) {
            // Collect error messages
            const auto& messages = compiler_->GetCompilerMessages();
            if (!messages.empty()) {
                last_error_ = "Compilation failed with errors:\n";
                for (const auto& msg : messages) {
                    last_error_ += "  " + msg + "\n";
                    std::cerr << "  " << msg << std::endl;
                }
            } else {
                last_error_ = "Compilation failed with unknown error";
            }
            
            std::cerr << "Compilation failed" << std::endl;
            return false;
        }

        // Check if object tree was generated
        if (!compiler_->GetObjectTree()) {
            last_error_ = "Object tree not generated after compilation";
            std::cerr << "Error: " << last_error_ << std::endl;
            return false;
        }

        object_tree_loaded_ = true;
        std::cout << "Compilation successful!" << std::endl;

        // Save to cache
        if (SaveObjectTreeCache()) {
            std::cout << "Object tree cached successfully" << std::endl;
        } else {
            std::cerr << "Warning: Failed to cache object tree" << std::endl;
        }

        return true;
    } catch (const std::exception& e) {
        last_error_ = "Compilation error: " + std::string(e.what());
        std::cerr << "Error during compilation: " << last_error_ << std::endl;
        if (callback) {
            callback("Compilation error", 1.0f);
        }
        return false;
    }
}

bool ProjectManager::HasValidObjectTree() const {
    return object_tree_loaded_ && compiler_ && compiler_->GetObjectTree();
}

std::string ProjectManager::GetProjectName() const {
    if (dme_path_.empty()) {
        return "";
    }

    fs::path path(dme_path_);
    return path.stem().string();
}

DMCompiler::DMObjectTree* ProjectManager::GetObjectTree() {
    if (!HasValidObjectTree()) {
        return nullptr;
    }

    return compiler_->GetObjectTree();
}

const std::vector<std::string>& ProjectManager::GetCompilationMessages() const {
    static const std::vector<std::string> empty;
    
    if (!compiler_) {
        return empty;
    }

    return compiler_->GetCompilerMessages();
}

bool ProjectManager::LoadCachedObjectTree() {
    try {
        if (!IsCacheValid()) {
            return false;
        }

        // Create a temporary compiler to hold the object tree
        compiler_ = std::make_unique<DMCompiler::DMCompiler>();

        std::string cache_path = project_directory_ + "/.myg_cache.bin";
        
        std::cout << "Loading cache from: " << cache_path << std::endl;
        
        if (!cache_->Load(cache_path, compiler_->GetObjectTree())) {
            std::cerr << "Failed to load cache" << std::endl;
            compiler_.reset();
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading cache: " << e.what() << std::endl;
        compiler_.reset();
        return false;
    }
}

bool ProjectManager::SaveObjectTreeCache() {
    try {
        if (!HasValidObjectTree()) {
            std::cerr << "Cannot save cache: no valid object tree" << std::endl;
            return false;
        }

        std::string cache_path = project_directory_ + "/.myg_cache.bin";
        
        std::cout << "Saving cache to: " << cache_path << std::endl;
        
        return cache_->Save(cache_path, compiler_->GetObjectTree());
    } catch (const std::exception& e) {
        std::cerr << "Error saving cache: " << e.what() << std::endl;
        return false;
    }
}

bool ProjectManager::IsCacheValid() const {
    if (dme_path_.empty() || project_directory_.empty()) {
        return false;
    }

    std::string cache_path = project_directory_ + "/.myg_cache.bin";
    
    // Check if cache file exists
    if (!FileExists(cache_path)) {
        return false;
    }

    // Check if cache is newer than .dme file
    uint64_t dme_timestamp = GetFileTimestamp(dme_path_);
    uint64_t cache_timestamp = GetFileTimestamp(cache_path);

    if (cache_timestamp < dme_timestamp) {
        std::cout << "Cache is older than .dme file" << std::endl;
        return false;
    }

    return true;
}

std::string ProjectManager::ExtractDirectory(const std::string& file_path) const {
    fs::path path(file_path);
    return path.parent_path().string();
}

bool ProjectManager::FileExists(const std::string& path) const {
    return fs::exists(path);
}

uint64_t ProjectManager::GetFileTimestamp(const std::string& path) const {
    try {
        if (!fs::exists(path)) {
            return 0;
        }

        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        
        return std::chrono::duration_cast<std::chrono::seconds>(
            sctp.time_since_epoch()
        ).count();
    } catch (const std::exception& e) {
        std::cerr << "Error getting file timestamp: " << e.what() << std::endl;
        return 0;
    }
}

} // namespace myg
