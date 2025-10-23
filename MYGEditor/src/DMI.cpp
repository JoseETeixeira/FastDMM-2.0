#include "myg/DMI.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace myg {

DMI::DMI()
    : icon_width_(32)
    , icon_height_(32)
    , sheet_width_(0)
    , sheet_height_(0)
    , texture_id_(0)
    , default_state_(nullptr)
{
}

DMI::~DMI() {
    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
    }
}

bool DMI::Load(const std::string& path) {
    // Load PNG file
    int width, height, channels;
    unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    
    if (!image_data) {
        std::cerr << "Failed to load DMI file: " << path << std::endl;
        std::cerr << "stbi error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    sheet_width_ = width;
    sheet_height_ = height;

    // Read the file again to extract PNG metadata
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open DMI file for metadata: " << path << std::endl;
        stbi_image_free(image_data);
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<unsigned char> png_data(file_size);
    fread(png_data.data(), 1, file_size, file);
    fclose(file);

    // Extract DMI metadata from PNG tEXt chunk
    std::string description = ExtractPNGDescription(png_data.data(), png_data.size());
    
    if (description.empty()) {

        // Use the full image as a single icon
        icon_width_ = width;
        icon_height_ = height;
        // Create a default state for the entire image
        IconState default_state;
        default_state.name = "";
        default_state.dir_count = 1;
        default_state.frame_count = 1;
        default_state.delay = 0.0f;
        default_state.loop = false;
        default_state.rewind = false;
        default_state.movement = false;
        
        // For pure PNG, just use the top-left icon (or entire image)
        IconState::IconSubstate substate;
        substate.dir = 2; // SOUTH
        substate.frame = 0;
        substate.x = 0;
        substate.y = 0;
        default_state.substates.push_back(substate);
        
        states_[""] = default_state;
        default_state_ = &states_[""];
        
        std::cout << "Loaded pure PNG file: " << path 
                  << " (treating as " << icon_width_ << "x" << icon_height_ << " icon)" << std::endl;
    } else {
        // Parse metadata
        if (!ParseMetadata(description)) {
            std::cerr << "Failed to parse DMI metadata: " << path << std::endl;
            stbi_image_free(image_data);
            return false;
        }
    }

    // Load texture into OpenGL
    bool texture_loaded = LoadTexture(image_data, width, height);
    stbi_image_free(image_data);

    return texture_loaded;
}

std::string DMI::ExtractPNGDescription(const unsigned char* png_data, size_t png_size) {
    // PNG signature: 89 50 4E 47 0D 0A 1A 0A
    if (png_size < 8 || png_data[0] != 0x89 || png_data[1] != 0x50 ||
        png_data[2] != 0x4E || png_data[3] != 0x47) {
        return "";
    }

    size_t pos = 8; // Skip PNG signature

    while (pos + 12 <= png_size) {
        // Read chunk length (big-endian)
        uint32_t length = (png_data[pos] << 24) | (png_data[pos + 1] << 16) |
                         (png_data[pos + 2] << 8) | png_data[pos + 3];
        pos += 4;

        // Read chunk type
        char chunk_type[5] = {0};
        memcpy(chunk_type, &png_data[pos], 4);
        pos += 4;

        // Check if this is a tEXt chunk
        if (strcmp(chunk_type, "tEXt") == 0 && pos + length <= png_size) {
            // Find the null separator between keyword and text
            const char* chunk_data = reinterpret_cast<const char*>(&png_data[pos]);
            const char* null_pos = static_cast<const char*>(memchr(chunk_data, '\0', length));
            
            if (null_pos) {
                std::string keyword(chunk_data, null_pos - chunk_data);
                
                // Check if this is the "Description" chunk
                if (keyword == "Description") {
                    size_t text_start = (null_pos - chunk_data) + 1;
                    size_t text_length = length - text_start;
                    return std::string(null_pos + 1, text_length);
                }
            }
        }

        // Skip chunk data and CRC
        pos += length + 4;

        // Stop at IEND chunk
        if (strcmp(chunk_type, "IEND") == 0) {
            break;
        }
    }

    return "";
}

bool DMI::ParseMetadata(const std::string& description) {
    std::istringstream iss(description);
    std::string line;
    
    // First line should be "# BEGIN DMM"
    if (!std::getline(iss, line) || line.find("# BEGIN DMI") == std::string::npos) {
        return false;
    }

    // Second line should be "version = 4.0"
    if (!std::getline(iss, line)) {
        return false;
    }

    // Third line should contain width and height
    if (!std::getline(iss, line)) {
        return false;
    }

    // Parse: width = X, height = Y
    size_t width_pos = line.find("width = ");
    size_t height_pos = line.find("height = ");
    
    if (width_pos != std::string::npos && height_pos != std::string::npos) {
        icon_width_ = std::stoi(line.substr(width_pos + 8));
        icon_height_ = std::stoi(line.substr(height_pos + 9));
    } else {
        return false;
    }

    // Calculate icons per row
    int icons_per_row = sheet_width_ / icon_width_;
    int current_icon_index = 0;

    // Parse icon states
    IconState* current_state = nullptr;
    
    while (std::getline(iss, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        if (line.find("state = ") == 0) {
            // New state
            std::string state_name = line.substr(8);
            // Remove quotes if present
            if (state_name.size() >= 2 && state_name.front() == '"' && state_name.back() == '"') {
                state_name = state_name.substr(1, state_name.size() - 2);
            }

            states_[state_name] = IconState();
            current_state = &states_[state_name];
            current_state->name = state_name;
            current_state->dir_count = 1;
            current_state->frame_count = 1;
            current_state->delay = 0.0f;
            current_state->loop = false;
            current_state->rewind = false;
            current_state->movement = false;

            if (default_state_ == nullptr) {
                default_state_ = current_state;
            }
        } else if (current_state != nullptr) {
            if (line.find("dirs = ") == 0) {
                current_state->dir_count = std::stoi(line.substr(7));
            } else if (line.find("frames = ") == 0) {
                current_state->frame_count = std::stoi(line.substr(9));
            } else if (line.find("delay = ") == 0) {
                // Parse delay values (can be comma-separated)
                std::string delay_str = line.substr(8);
                size_t comma_pos = delay_str.find(',');
                if (comma_pos != std::string::npos) {
                    current_state->delay = std::stof(delay_str.substr(0, comma_pos));
                } else {
                    current_state->delay = std::stof(delay_str);
                }
            } else if (line.find("loop = ") == 0) {
                current_state->loop = (std::stoi(line.substr(7)) != 0);
            } else if (line.find("rewind = ") == 0) {
                current_state->rewind = (std::stoi(line.substr(9)) != 0);
            } else if (line.find("movement = ") == 0) {
                current_state->movement = (std::stoi(line.substr(11)) != 0);
            }
        }
    }

    // Calculate substate positions for all states
    current_icon_index = 0;
    for (auto& state_pair : states_) {
        IconState& state = state_pair.second;
        
        // Direction order: SOUTH, NORTH, EAST, WEST, SOUTHEAST, SOUTHWEST, NORTHEAST, NORTHWEST
        static const int dir_order[] = {2, 1, 4, 8, 6, 10, 5, 9};
        
        for (int frame = 0; frame < state.frame_count; ++frame) {
            for (int dir_idx = 0; dir_idx < state.dir_count; ++dir_idx) {
                IconState::IconSubstate substate;
                substate.dir = dir_order[dir_idx];
                substate.frame = frame;
                
                // Calculate position in sprite sheet
                int row = current_icon_index / icons_per_row;
                int col = current_icon_index % icons_per_row;
                substate.x = col * icon_width_;
                substate.y = row * icon_height_;
                
                state.substates.push_back(substate);
                current_icon_index++;
            }
        }
    }

    return true;
}

bool DMI::LoadTexture(const unsigned char* image_data, int width, int height) {
    glGenTextures(1, &texture_id_);
    if (texture_id_ == 0) {
        std::cerr << "Failed to generate OpenGL texture" << std::endl;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id_);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

IconState* DMI::GetState(const std::string& state_name) {
    auto it = states_.find(state_name);
    if (it != states_.end()) {
        return &it->second;
    }
    return nullptr;
}

IconState* DMI::GetDefaultState() {
    return default_state_;
}

} // namespace myg
