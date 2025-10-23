#include "myg/Map.h"
#include "myg/TileInstance.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace myg {

Map::Map()
    : is_tgm_format_(false)
    , key_length_(3)
    , modified_(false)
    , object_tree_(nullptr)
{
    bounds_.min_x = bounds_.min_y = bounds_.min_z = 0;
    bounds_.max_x = bounds_.max_y = bounds_.max_z = 0;
}

Map::~Map() = default;

bool Map::Load(const std::string& path, DMCompiler::DMObjectTree* tree) {
    file_path_ = path;
    object_tree_ = tree;

    // Read file content
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Parse DMM format
    if (!ParseDMM(content)) {
        std::cerr << "Failed to parse DMM file: " << path << std::endl;
        return false;
    }

    modified_ = false;
    return true;
}

bool Map::Save(const std::string& path) {
    // TODO: Implement in task 11
    return false;
}

TileInstance* Map::GetTile(int x, int y, int z) {
    Location loc{x, y, z};
    auto it = tiles_.find(loc);
    if (it == tiles_.end()) {
        return nullptr;
    }

    const std::string& key = it->second;
    auto inst_it = instances_.find(key);
    if (inst_it == instances_.end()) {
        return nullptr;
    }

    return inst_it->second.get();
}

void Map::SetTile(int x, int y, int z, TileInstance* tile) {
    // TODO: Implement in editing tasks
    modified_ = true;
}

bool Map::PlaceObject(int x, int y, int z, const std::string& type_path) {
    if (type_path.empty()) {
        return false;
    }

    Location loc{x, y, z};
    
    // Get or create tile at this location
    TileInstance* tile = GetTile(x, y, z);
    
    if (!tile) {
        // Create a new tile instance
        auto new_tile = std::make_unique<TileInstance>();
        
        // Allocate a key for this tile
        std::string key = AllocateKey();
        if (key.empty()) {
            std::cerr << "Failed to allocate key for new tile" << std::endl;
            return false;
        }
        
        // Store the tile instance
        tile = new_tile.get();
        instances_[key] = std::move(new_tile);
        tiles_[loc] = key;
    }
    
    // Create the new object instance
    ObjectInstance new_obj;
    new_obj.type_path = type_path;
    
    // Determine object type category
    bool is_turf = type_path.find("/turf") == 0;
    bool is_area = type_path.find("/area") == 0;
    bool is_obj = type_path.find("/obj") == 0;
    bool is_mob = type_path.find("/mob") == 0;
    
    // Handle placement based on type
    if (is_turf) {
        // Replace existing turf
        bool found_turf = false;
        for (auto& obj : tile->objects) {
            if (obj.IsType("/turf")) {
                obj = new_obj;
                found_turf = true;
                break;
            }
        }
        
        if (!found_turf) {
            // No existing turf, add it
            tile->objects.push_back(new_obj);
        }
    }
    else if (is_area) {
        // Replace existing area
        bool found_area = false;
        for (auto& obj : tile->objects) {
            if (obj.IsType("/area")) {
                obj = new_obj;
                found_area = true;
                break;
            }
        }
        
        if (!found_area) {
            // No existing area, add it
            tile->objects.push_back(new_obj);
        }
    }
    else if (is_obj || is_mob) {
        // Append to object list
        tile->objects.push_back(new_obj);
    }
    else {
        // Unknown type, just append
        tile->objects.push_back(new_obj);
    }
    
    // Invalidate tile cache
    tile->cached_sorted_.clear();
    tile->cache_valid_ = false;
    tile->cached_area_ = nullptr;
    
    modified_ = true;
    return true;
}

void Map::PushUndoState(std::unique_ptr<UndoableAction> action) {
    // TODO: Implement in task 10
}

bool Map::Undo() {
    // TODO: Implement in task 10
    return false;
}

bool Map::Redo() {
    // TODO: Implement in task 10
    return false;
}

std::string Map::AllocateKey() {
    if (!unused_keys_.empty()) {
        std::string key = unused_keys_.back();
        unused_keys_.pop_back();
        return key;
    }

    // Generate more keys if needed
    GenerateKeys(key_length_);
    if (!unused_keys_.empty()) {
        std::string key = unused_keys_.back();
        unused_keys_.pop_back();
        return key;
    }

    return "";
}

void Map::ReleaseKey(const std::string& key) {
    unused_keys_.push_back(key);
}

void Map::GenerateKeys(int length) {
    // Generate all possible keys of given length using a-z
    // This is a simplified version - real implementation would be more sophisticated
    const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int char_count = 52;

    // For now, just generate some keys
    for (int i = 0; i < 100; i++) {
        std::string key;
        int val = i;
        for (int j = 0; j < length; j++) {
            key += chars[val % char_count];
            val /= char_count;
        }
        if (instances_.find(key) == instances_.end()) {
            unused_keys_.push_back(key);
        }
    }
}

bool Map::ParseDMM(const std::string& content) {
    size_t pos = 0;
    bool parsing_definitions = true;

    // Detect format by looking for TGM-style definitions
    is_tgm_format_ = (content.find("\"\n/") != std::string::npos);

    while (pos < content.size()) {
        // Skip whitespace
        while (pos < content.size() && std::isspace(content[pos])) {
            pos++;
        }

        if (pos >= content.size()) break;

        // Check if we're starting a map grid section
        if (content[pos] == '(') {
            parsing_definitions = false;
            // Find the end of this grid section
            size_t section_start = pos;
            int paren_depth = 0;
            bool in_quotes = false;

            while (pos < content.size()) {
                if (content[pos] == '"' && (pos == 0 || content[pos-1] != '\\')) {
                    in_quotes = !in_quotes;
                }
                if (!in_quotes) {
                    if (content[pos] == '(') paren_depth++;
                    if (content[pos] == ')') {
                        paren_depth--;
                        if (paren_depth == 0) {
                            pos++;
                            break;
                        }
                    }
                }
                pos++;
            }

            std::string section = content.substr(section_start, pos - section_start);
            if (!ParseMapGrid(section)) {
                return false;
            }
        }
        // Parse tile definition
        else if (content[pos] == '"') {
            size_t line_start = pos;
            size_t line_end = content.find('\n', pos);
            if (line_end == std::string::npos) {
                line_end = content.size();
            }

            std::string line = content.substr(line_start, line_end - line_start);
            std::string key, definition;

            if (ParseTileDefinition(line, key, definition)) {
                // Create TileInstance from definition
                auto tile = TileInstance::FromString(definition, object_tree_);
                if (tile) {
                    instances_[key] = std::move(tile);
                }
            }

            pos = line_end + 1;
        }
        else {
            pos++;
        }
    }

    return true;
}

bool Map::ParseTileDefinition(const std::string& line, std::string& key, std::string& definition) {
    // Format: "key" = (definition)
    // or TGM: "key" = (\ndefinition)

    size_t pos = 0;

    // Skip whitespace
    while (pos < line.size() && std::isspace(line[pos])) {
        pos++;
    }

    // Parse key
    if (pos >= line.size() || line[pos] != '"') {
        return false;
    }
    pos++; // Skip opening quote

    size_t key_start = pos;
    while (pos < line.size() && line[pos] != '"') {
        pos++;
    }
    if (pos >= line.size()) {
        return false;
    }

    key = line.substr(key_start, pos - key_start);
    pos++; // Skip closing quote

    // Update key length
    if (key.length() > 0 && (int)key.length() > key_length_) {
        key_length_ = key.length();
    }

    // Skip whitespace and '='
    while (pos < line.size() && (std::isspace(line[pos]) || line[pos] == '=')) {
        pos++;
    }

    // Parse definition
    if (pos >= line.size() || line[pos] != '(') {
        return false;
    }
    pos++; // Skip opening paren

    size_t def_start = pos;
    int paren_depth = 1;
    while (pos < line.size() && paren_depth > 0) {
        if (line[pos] == '(') paren_depth++;
        if (line[pos] == ')') paren_depth--;
        if (paren_depth > 0) pos++;
    }

    definition = line.substr(def_start, pos - def_start);

    return true;
}

bool Map::ParseMapGrid(const std::string& section) {
    // Format: (x,y,z) = {"keys"}
    
    size_t pos = 0;

    // Skip opening paren
    if (section[pos] != '(') return false;
    pos++;

    // Parse coordinates
    int x, y, z;
    size_t comma1 = section.find(',', pos);
    size_t comma2 = section.find(',', comma1 + 1);
    size_t close_paren = section.find(')', comma2 + 1);

    if (comma1 == std::string::npos || comma2 == std::string::npos || close_paren == std::string::npos) {
        return false;
    }

    try {
        x = std::stoi(section.substr(pos, comma1 - pos));
        y = std::stoi(section.substr(comma1 + 1, comma2 - comma1 - 1));
        z = std::stoi(section.substr(comma2 + 1, close_paren - comma2 - 1));
    } catch (...) {
        return false;
    }

    // Update bounds
    if (tiles_.empty()) {
        bounds_.min_x = bounds_.max_x = x;
        bounds_.min_y = bounds_.max_y = y;
        bounds_.min_z = bounds_.max_z = z;
    } else {
        bounds_.min_x = std::min(bounds_.min_x, x);
        bounds_.max_x = std::max(bounds_.max_x, x);
        bounds_.min_y = std::min(bounds_.min_y, y);
        bounds_.max_y = std::max(bounds_.max_y, y);
        bounds_.min_z = std::min(bounds_.min_z, z);
        bounds_.max_z = std::max(bounds_.max_z, z);
    }

    // Find the grid data
    pos = section.find('{', close_paren);
    if (pos == std::string::npos) return false;
    pos++; // Skip opening brace

    // Skip opening quote
    while (pos < section.size() && (std::isspace(section[pos]) || section[pos] == '"')) {
        pos++;
    }

    // Parse grid keys
    int current_x = x;
    int current_y = y;

    while (pos < section.size()) {
        char c = section[pos];

        if (c == '"' || c == '}') {
            break;
        }

        if (c == '\n' || c == '\r') {
            current_y++;
            current_x = x;
            pos++;
            continue;
        }

        if (std::isspace(c)) {
            pos++;
            continue;
        }

        // Read key
        std::string key;
        while (pos < section.size() && !std::isspace(section[pos]) && 
               section[pos] != '"' && section[pos] != '}') {
            key += section[pos];
            pos++;
        }

        if (!key.empty()) {
            Location loc{current_x, current_y, z};
            tiles_[loc] = key;
            current_x++;
        }
    }

    return true;
}

} // namespace myg
