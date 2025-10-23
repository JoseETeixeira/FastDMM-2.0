#pragma once

#include "myg/DMI.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace myg {

/**
 * Cache manager for DMI (Dream Maker Icon) files.
 * Loads and caches DMI files to avoid redundant disk I/O and parsing.
 */
class DMICache {
public:
    DMICache();
    ~DMICache();

    /**
     * Set the project directory path.
     * This is used as the base path for resolving relative icon paths.
     * @param project_dir Path to the project directory
     */
    void SetProjectDirectory(const std::string& project_dir);

    /**
     * Get a DMI file, loading it if not already cached.
     * @param icon_path Path to the icon file (relative to project directory)
     * @return Pointer to DMI object, or placeholder DMI if file not found
     */
    DMI* GetDMI(const std::string& icon_path);

    /**
     * Clear all cached DMI files.
     * This will free all loaded textures and memory.
     */
    void Clear();

    /**
     * Get the placeholder DMI used for missing files.
     * @return Pointer to placeholder DMI
     */
    DMI* GetPlaceholderDMI();

private:
    std::string project_dir_;
    std::unordered_map<std::string, std::unique_ptr<DMI>> cache_;
    std::unique_ptr<DMI> placeholder_dmi_;

    /**
     * Build the full file path from project directory and icon path.
     * @param icon_path Relative icon path
     * @return Full file system path
     */
    std::string BuildFilePath(const std::string& icon_path) const;

    /**
     * Create a placeholder DMI for missing files.
     * @return Unique pointer to placeholder DMI
     */
    std::unique_ptr<DMI> CreatePlaceholderDMI();
};

} // namespace myg
