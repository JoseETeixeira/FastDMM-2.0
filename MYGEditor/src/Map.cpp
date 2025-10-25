#include "myg/Map.h"
#include "myg/TileInstance.h"
#include "myg/UndoableAction.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <map>

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
    try {
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

        // Check if file was empty
        if (content.empty()) {
            std::cerr << "Map file is empty: " << path << std::endl;
            return false;
        }

        // Parse DMM format
        if (!ParseDMM(content)) {
            std::cerr << "Failed to parse DMM file: " << path << std::endl;
            return false;
        }

        modified_ = false;
        std::cout << "Successfully loaded map: " << path << std::endl;
        std::cout << "  Bounds: (" << bounds_.min_x << "," << bounds_.min_y << "," << bounds_.min_z 
                  << ") to (" << bounds_.max_x << "," << bounds_.max_y << "," << bounds_.max_z << ")" << std::endl;
        std::cout << "  Tiles: " << tiles_.size() << ", Instances: " << instances_.size() << std::endl;
        
        return true;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "I/O error loading map file: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading map: " << e.what() << std::endl;
        return false;
    }
}

bool Map::Save(const std::string& path) {
    try {
        // Open file for writing
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open map file for writing: " << path << std::endl;
            return false;
        }

        // Write tile definitions first
        if (!WriteTileDefinitions(file)) {
            std::cerr << "Failed to write tile definitions" << std::endl;
            file.close();
            return false;
        }

        // Write map grid sections
        if (!WriteMapGrid(file)) {
            std::cerr << "Failed to write map grid" << std::endl;
            file.close();
            return false;
        }

        file.close();
        
        // Check if write was successful
        if (file.fail()) {
            std::cerr << "Error occurred while writing map file: " << path << std::endl;
            return false;
        }
        
        // Update file path and clear modified flag
        file_path_ = path;
        modified_ = false;
        
        std::cout << "Successfully saved map: " << path << std::endl;
        return true;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "I/O error saving map file: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error saving map: " << e.what() << std::endl;
        return false;
    }
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
    try {
        if (type_path.empty()) {
            std::cerr << "Cannot place object: type path is empty" << std::endl;
            return false;
        }

        // Validate coordinates are within reasonable bounds
        if (x < -1000 || x > 1000 || y < -1000 || y > 1000 || z < 1 || z > 100) {
            std::cerr << "Cannot place object: coordinates out of bounds (" << x << "," << y << "," << z << ")" << std::endl;
            return false;
        }

        // Create a PlaceObjectAction and push it to the undo stack
        // This will execute the action and add it to the undo stack
        auto action = std::make_unique<PlaceObjectAction>(x, y, z, type_path);
        
        // Ensure tile exists before placing
        Location loc{x, y, z};
        TileInstance* tile = GetTile(x, y, z);
        
        if (!tile) {
            // Create a new tile instance
            auto new_tile = std::make_unique<TileInstance>();
            
            // Allocate a key for this tile
            std::string key = AllocateKey();
            if (key.empty()) {
                std::cerr << "Failed to allocate key for new tile at (" << x << "," << y << "," << z << ")" << std::endl;
                return false;
            }
            
            // Store the tile instance
            tile = new_tile.get();
            instances_[key] = std::move(new_tile);
            tiles_[loc] = key;
        }
        
        // Push the action to undo stack (this will execute it)
        PushUndoState(std::move(action));
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error placing object: " << e.what() << std::endl;
        return false;
    }
}

void Map::PushUndoState(std::unique_ptr<UndoableAction> action) {
    if (!action) return;

    // Execute the action (Redo)
    action->Redo(this);

    // Add to undo stack
    undo_stack_.push_back(std::move(action));

    // Clear redo stack when new action is performed
    redo_stack_.clear();

    modified_ = true;
}

bool Map::Undo() {
    if (undo_stack_.empty()) {
        return false;
    }

    // Get the last action from undo stack
    std::unique_ptr<UndoableAction> action = std::move(undo_stack_.back());
    undo_stack_.pop_back();

    // Undo the action
    action->Undo(this);

    // Move to redo stack
    redo_stack_.push_back(std::move(action));

    modified_ = true;
    return true;
}

bool Map::Redo() {
    if (redo_stack_.empty()) {
        return false;
    }

    // Get the last action from redo stack
    std::unique_ptr<UndoableAction> action = std::move(redo_stack_.back());
    redo_stack_.pop_back();

    // Redo the action
    action->Redo(this);

    // Move to undo stack
    undo_stack_.push_back(std::move(action));

    modified_ = true;
    return true;
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

bool Map::WriteTileDefinitions(std::ofstream& file) {
    // Write all tile instance definitions
    // Sort keys for consistent output
    std::vector<std::string> sorted_keys;
    for (const auto& [key, instance] : instances_) {
        sorted_keys.push_back(key);
    }
    std::sort(sorted_keys.begin(), sorted_keys.end());

    for (const auto& key : sorted_keys) {
        const auto& instance = instances_[key];
        if (!instance) continue;

        // Write key
        file << "\"" << key << "\" = (";

        if (is_tgm_format_) {
            // TGM format: multi-line with proper formatting
            file << "\n";
            std::string tile_str = instance->ToTGMString();
            file << tile_str;
            file << ")\n";
        } else {
            // Standard format: single line
            std::string tile_str = instance->ToString();
            file << tile_str;
            file << ")\n";
        }
    }

    return true;
}

bool Map::WriteMapGrid(std::ofstream& file) {
    // Group tiles by Z-level
    std::map<int, std::vector<Location>> tiles_by_z;
    
    for (const auto& [loc, key] : tiles_) {
        tiles_by_z[loc.z].push_back(loc);
    }

    // Write each Z-level
    for (const auto& [z, locations] : tiles_by_z) {
        // Find bounds for this Z-level
        int min_x = bounds_.max_x;
        int max_x = bounds_.min_x;
        int min_y = bounds_.max_y;
        int max_y = bounds_.min_y;

        for (const auto& loc : locations) {
            min_x = std::min(min_x, loc.x);
            max_x = std::max(max_x, loc.x);
            min_y = std::min(min_y, loc.y);
            max_y = std::max(max_y, loc.y);
        }

        // Write grid section header
        file << "\n(" << min_x << "," << min_y << "," << z << ") = {\"\n";

        // Write grid data row by row
        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                Location loc{x, y, z};
                auto it = tiles_.find(loc);
                
                if (it != tiles_.end()) {
                    file << it->second;
                } else {
                    // Write empty key (should not happen in well-formed maps)
                    file << std::string(key_length_, 'a');
                }
            }
            file << "\n";
        }

        file << "\"}\n";
    }

    return true;
}

} // namespace myg
