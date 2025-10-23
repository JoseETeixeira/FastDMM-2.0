#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

namespace myg {

/**
 * Represents a single icon state within a DMI file.
 * Icon states can have multiple directions and animation frames.
 */
struct IconState {
    std::string name;
    int dir_count;      // 1, 4, or 8 directions
    int frame_count;    // Number of animation frames
    float delay;        // Animation delay per frame (in deciseconds)
    bool loop;          // Whether animation loops
    bool rewind;        // Whether animation reverses
    bool movement;      // Whether this is a movement state

    /**
     * Represents a single substate (specific direction + frame combination)
     */
    struct IconSubstate {
        int dir;        // Direction: NORTH=1, SOUTH=2, EAST=4, WEST=8
        int frame;      // Frame index
        int x, y;       // Position in sprite sheet (in pixels)
    };

    std::vector<IconSubstate> substates;
};

/**
 * DMI (Dream Maker Icon) file parser and container.
 * Handles loading PNG files with embedded DMI metadata and extracting sprite information.
 */
class DMI {
public:
    DMI();
    ~DMI();

    /**
     * Load a DMI file from disk.
     * @param path Path to the .dmi file (PNG with metadata)
     * @return true if loaded successfully, false otherwise
     */
    bool Load(const std::string& path);

    /**
     * Get an icon state by name.
     * @param state_name Name of the state to retrieve
     * @return Pointer to IconState, or nullptr if not found
     */
    IconState* GetState(const std::string& state_name);

    /**
     * Get the default icon state (first state or empty state).
     * @return Pointer to default IconState
     */
    IconState* GetDefaultState();

    /**
     * Get the OpenGL texture ID for this DMI.
     * @return OpenGL texture ID
     */
    GLuint GetTextureID() const { return texture_id_; }

    /**
     * Get the width of individual icons in this DMI.
     * @return Icon width in pixels
     */
    int GetIconWidth() const { return icon_width_; }

    /**
     * Get the height of individual icons in this DMI.
     * @return Icon height in pixels
     */
    int GetIconHeight() const { return icon_height_; }

    /**
     * Get the full sprite sheet width.
     * @return Sheet width in pixels
     */
    int GetSheetWidth() const { return sheet_width_; }

    /**
     * Get the full sprite sheet height.
     * @return Sheet height in pixels
     */
    int GetSheetHeight() const { return sheet_height_; }

    /**
     * Check if this DMI has been successfully loaded.
     * @return true if loaded, false otherwise
     */
    bool IsLoaded() const { return texture_id_ != 0; }

private:
    int icon_width_;
    int icon_height_;
    int sheet_width_;
    int sheet_height_;

    GLuint texture_id_;
    std::unordered_map<std::string, IconState> states_;
    IconState* default_state_;

    /**
     * Parse DMI metadata from the PNG tEXt chunk.
     * @param description The description text from the PNG
     * @return true if parsed successfully
     */
    bool ParseMetadata(const std::string& description);

    /**
     * Load the PNG image data into an OpenGL texture.
     * @param image_data Raw RGBA image data
     * @param width Image width
     * @param height Image height
     * @return true if texture created successfully
     */
    bool LoadTexture(const unsigned char* image_data, int width, int height);

    /**
     * Extract PNG tEXt chunk containing DMI metadata.
     * @param png_data Raw PNG file data
     * @param png_size Size of PNG data
     * @return Extracted description string, or empty if not found
     */
    std::string ExtractPNGDescription(const unsigned char* png_data, size_t png_size);
};

} // namespace myg
