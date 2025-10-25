#include "myg/DMICache.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

namespace myg {

DMICache::DMICache()
    : project_dir_("")
{
    placeholder_dmi_ = CreatePlaceholderDMI();
}

DMICache::~DMICache() {
    Clear();
}

void DMICache::SetProjectDirectory(const std::string& project_dir) {
    project_dir_ = project_dir;
}

std::string DMICache::BuildFilePath(const std::string& icon_path) const {
    if (project_dir_.empty()) {
        return icon_path;
    }

    // Handle absolute paths
    std::filesystem::path icon_fs_path(icon_path);
    if (icon_fs_path.is_absolute()) {
        return icon_path;
    }

    // Build relative path from project directory
    std::filesystem::path full_path = std::filesystem::path(project_dir_) / icon_path;
    return full_path.string();
}

DMI* DMICache::GetDMI(const std::string& icon_path) {
    try {
        // Check if already cached
        auto it = cache_.find(icon_path);
        if (it != cache_.end()) {
            return it->second.get();
        }

        // Validate icon path
        if (icon_path.empty()) {
            std::cerr << "Warning: Empty icon path, using placeholder" << std::endl;
            return placeholder_dmi_.get();
        }

        // Build full file path
        std::string full_path = BuildFilePath(icon_path);

        // Check if file exists before trying to load
        if (!std::filesystem::exists(full_path)) {
            std::cerr << "Warning: DMI file not found: " << full_path << " (using placeholder)" << std::endl;
            // Cache the placeholder for this path to avoid repeated warnings
            cache_[icon_path] = nullptr;
            return placeholder_dmi_.get();
        }

        // Try to load the DMI
        auto dmi = std::make_unique<DMI>();
        if (dmi->Load(full_path)) {
            DMI* dmi_ptr = dmi.get();
            cache_[icon_path] = std::move(dmi);
            std::cout << "Loaded DMI: " << icon_path << std::endl;
            return dmi_ptr;
        }

        // File exists but failed to load - return placeholder
        std::cerr << "Warning: Failed to parse DMI: " << full_path << " (using placeholder)" << std::endl;
        cache_[icon_path] = nullptr;
        return placeholder_dmi_.get();
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error loading DMI '" << icon_path << "': " << e.what() << " (using placeholder)" << std::endl;
        return placeholder_dmi_.get();
    } catch (const std::exception& e) {
        std::cerr << "Error loading DMI '" << icon_path << "': " << e.what() << " (using placeholder)" << std::endl;
        return placeholder_dmi_.get();
    }
}

void DMICache::Clear() {
    cache_.clear();
}

DMI* DMICache::GetPlaceholderDMI() {
    return placeholder_dmi_.get();
}

std::unique_ptr<DMI> DMICache::CreatePlaceholderDMI() {
    auto dmi = std::make_unique<DMI>();

    // Create a simple 32x32 magenta/black checkerboard texture
    const int size = 32;
    unsigned char pixels[size * size * 4];

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int idx = (y * size + x) * 4;
            
            // Checkerboard pattern
            bool is_magenta = ((x / 8) + (y / 8)) % 2 == 0;
            
            if (is_magenta) {
                pixels[idx + 0] = 255; // R
                pixels[idx + 1] = 0;   // G
                pixels[idx + 2] = 255; // B
                pixels[idx + 3] = 255; // A
            } else {
                pixels[idx + 0] = 0;   // R
                pixels[idx + 1] = 0;   // G
                pixels[idx + 2] = 0;   // B
                pixels[idx + 3] = 255; // A
            }
        }
    }

    // Create OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Note: We're creating a placeholder but the DMI class manages its own texture.
    // For a proper placeholder, we'd need to expose a way to set the texture directly.
    // For now, this creates the texture but the DMI won't know about it.
    // This is a simplified implementation - in production, you'd want to handle this better.

    return dmi;
}

} // namespace myg
