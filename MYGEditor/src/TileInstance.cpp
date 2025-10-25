#include "myg/TileInstance.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace myg {

// ObjectInstance implementation

std::string ObjectInstance::GetVar(const std::string& name, ::DMCompiler::DMObjectTree* tree) const {
    auto it = vars.find(name);
    if (it != vars.end()) {
        return it->second;
    }

    // TODO: Look up in object tree for inherited values
    return "";
}

void ObjectInstance::SetVar(const std::string& name, const std::string& value) {
    vars[name] = value;
}

bool ObjectInstance::IsType(const std::string& path) const {
    // Simple prefix check - could be more sophisticated with object tree
    return type_path.find(path) == 0;
}

std::string ObjectInstance::ToString() const {
    std::string result = type_path;

    if (!vars.empty()) {
        result += "{";
        bool first = true;
        for (const auto& [key, value] : vars) {
            if (!first) result += ";";
            first = false;
            result += key + "=" + value;
        }
        result += "}";
    }

    return result;
}

std::string ObjectInstance::ToTGMString() const {
    std::string result = type_path;

    if (!vars.empty()) {
        result += "{";
        bool first = true;
        for (const auto& [key, value] : vars) {
            if (!first) result += "; ";
            first = false;
            result += key + " = " + value;
        }
        result += "}";
    }

    return result;
}

// TileInstance implementation

std::string TileInstance::ToString() const {
    std::string result;
    for (size_t i = 0; i < objects.size(); i++) {
        if (i > 0) result += ",";
        result += objects[i].ToString();
    }
    return result;
}

std::string TileInstance::ToTGMString() const {
    std::string result;
    for (size_t i = 0; i < objects.size(); i++) {
        result += objects[i].ToTGMString();
        if (i < objects.size() - 1) {
            result += ",\n";
        }
    }
    return result;
}

std::unique_ptr<TileInstance> TileInstance::FromString(
    const std::string& str,
    ::DMCompiler::DMObjectTree* tree
) {
    try {
        auto tile = std::make_unique<TileInstance>();
        tile->cache_valid_ = false;
        tile->cached_area_ = nullptr;

        size_t pos = 0;

        while (pos < str.size()) {
            // Skip whitespace and commas
            while (pos < str.size() && (std::isspace(str[pos]) || str[pos] == ',')) {
                pos++;
            }

            if (pos >= str.size()) break;

            // Parse object path
            ObjectInstance obj;
            size_t path_start = pos;

            // Find end of path (either '{' for vars, ',' for next object, or end of string)
            while (pos < str.size() && str[pos] != '{' && str[pos] != ',' && 
                   str[pos] != ')' && str[pos] != '\n') {
                pos++;
            }

            obj.type_path = str.substr(path_start, pos - path_start);

            // Trim whitespace from path
            size_t path_end = obj.type_path.find_last_not_of(" \t\r\n");
            if (path_end != std::string::npos) {
                obj.type_path = obj.type_path.substr(0, path_end + 1);
            }

            // Validate object path
            if (!obj.type_path.empty() && obj.type_path[0] != '/') {
                std::cerr << "Warning: Invalid object path (missing leading slash): " << obj.type_path << std::endl;
                // Continue parsing but log the warning
            }

        // Parse variables if present
        if (pos < str.size() && str[pos] == '{') {
            pos++; // Skip opening brace

            while (pos < str.size() && str[pos] != '}') {
                // Skip whitespace
                while (pos < str.size() && std::isspace(str[pos])) {
                    pos++;
                }

                if (pos >= str.size() || str[pos] == '}') break;

                // Parse variable name
                size_t var_start = pos;
                while (pos < str.size() && str[pos] != '=' && str[pos] != ';' && str[pos] != '}') {
                    pos++;
                }

                std::string var_name = str.substr(var_start, pos - var_start);
                
                // Trim whitespace
                size_t name_end = var_name.find_last_not_of(" \t\r\n");
                if (name_end != std::string::npos) {
                    var_name = var_name.substr(0, name_end + 1);
                }

                // Skip '='
                if (pos < str.size() && str[pos] == '=') {
                    pos++;
                }

                // Skip whitespace
                while (pos < str.size() && std::isspace(str[pos])) {
                    pos++;
                }

                // Parse variable value
                std::string var_value;
                bool in_quotes = false;

                if (pos < str.size() && str[pos] == '"') {
                    in_quotes = true;
                    pos++; // Skip opening quote
                    size_t value_start = pos;

                    while (pos < str.size() && str[pos] != '"') {
                        if (str[pos] == '\\' && pos + 1 < str.size()) {
                            pos++; // Skip escape character
                        }
                        pos++;
                    }

                    var_value = str.substr(value_start, pos - value_start);
                    if (pos < str.size()) pos++; // Skip closing quote
                } else {
                    size_t value_start = pos;
                    while (pos < str.size() && str[pos] != ';' && str[pos] != '}') {
                        pos++;
                    }

                    var_value = str.substr(value_start, pos - value_start);
                    
                    // Trim whitespace
                    size_t value_end = var_value.find_last_not_of(" \t\r\n");
                    if (value_end != std::string::npos) {
                        var_value = var_value.substr(0, value_end + 1);
                    }
                }

                if (!var_name.empty()) {
                    obj.vars[var_name] = var_value;
                }

                // Skip semicolon
                if (pos < str.size() && str[pos] == ';') {
                    pos++;
                }
            }

            if (pos < str.size() && str[pos] == '}') {
                pos++; // Skip closing brace
            }
        }

            if (!obj.type_path.empty()) {
                tile->objects.push_back(obj);
            }
        }

        return tile;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing tile instance: " << e.what() << std::endl;
        // Return an empty tile instance rather than nullptr
        auto tile = std::make_unique<TileInstance>();
        tile->cache_valid_ = false;
        tile->cached_area_ = nullptr;
        return tile;
    }
}

std::vector<ObjectInstance*> TileInstance::GetLayerSorted() {
    // Return cached result if valid
    if (cache_valid_ && !cached_sorted_.empty()) {
        return cached_sorted_;
    }

    // Build list of object pointers
    cached_sorted_.clear();
    for (auto& obj : objects) {
        cached_sorted_.push_back(&obj);
    }

    // Sort by plane (primary), layer (secondary), and type category (tiebreaker)
    std::sort(cached_sorted_.begin(), cached_sorted_.end(), 
        [](const ObjectInstance* a, const ObjectInstance* b) {
            // Get plane values (default to 0)
            int plane_a = 0;
            int plane_b = 0;
            
            auto plane_a_it = a->vars.find("plane");
            if (plane_a_it != a->vars.end()) {
                try {
                    plane_a = std::stoi(plane_a_it->second);
                } catch (...) {}
            }
            
            auto plane_b_it = b->vars.find("plane");
            if (plane_b_it != b->vars.end()) {
                try {
                    plane_b = std::stoi(plane_b_it->second);
                } catch (...) {}
            }

            // Compare by plane first
            if (plane_a != plane_b) {
                return plane_a < plane_b;
            }

            // Get layer values (default to 0)
            int layer_a = 0;
            int layer_b = 0;
            
            auto layer_a_it = a->vars.find("layer");
            if (layer_a_it != a->vars.end()) {
                try {
                    layer_a = std::stoi(layer_a_it->second);
                } catch (...) {}
            }
            
            auto layer_b_it = b->vars.find("layer");
            if (layer_b_it != b->vars.end()) {
                try {
                    layer_b = std::stoi(layer_b_it->second);
                } catch (...) {}
            }

            // Compare by layer second
            if (layer_a != layer_b) {
                return layer_a < layer_b;
            }

            // Tiebreaker: sort by type category (area < turf < obj < mob)
            auto get_type_priority = [](const ObjectInstance* obj) {
                if (obj->type_path.find("/area") == 0) return 0;
                if (obj->type_path.find("/turf") == 0) return 1;
                if (obj->type_path.find("/obj") == 0) return 2;
                if (obj->type_path.find("/mob") == 0) return 3;
                return 4; // Unknown types last
            };

            int priority_a = get_type_priority(a);
            int priority_b = get_type_priority(b);

            return priority_a < priority_b;
        }
    );

    cache_valid_ = true;
    return cached_sorted_;
}

ObjectInstance* TileInstance::GetArea() {
    if (cached_area_ != nullptr) {
        return cached_area_;
    }

    for (auto& obj : objects) {
        if (obj.IsType("/area")) {
            cached_area_ = &obj;
            return cached_area_;
        }
    }

    return nullptr;
}

} // namespace myg
