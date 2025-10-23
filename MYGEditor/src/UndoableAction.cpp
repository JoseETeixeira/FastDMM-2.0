#include "myg/UndoableAction.h"
#include "myg/Map.h"
#include "myg/TileInstance.h"
#include <iostream>

namespace myg {

// PlaceObjectAction implementation

PlaceObjectAction::PlaceObjectAction(int x, int y, int z, const std::string& type_path)
    : x_(x), y_(y), z_(z), type_path_(type_path), replaced_existing_(false)
{
}

void PlaceObjectAction::Undo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) return;

    // Determine object type category
    bool is_turf = type_path_.find("/turf") == 0;
    bool is_area = type_path_.find("/area") == 0;

    if (is_turf || is_area) {
        // If we replaced an existing object, restore it
        if (replaced_existing_) {
            for (auto& obj : tile->objects) {
                if ((is_turf && obj.IsType("/turf")) || (is_area && obj.IsType("/area"))) {
                    obj = previous_object_;
                    break;
                }
            }
        } else {
            // Remove the object we added
            for (auto it = tile->objects.begin(); it != tile->objects.end(); ++it) {
                if ((is_turf && it->IsType("/turf") && it->type_path == type_path_) ||
                    (is_area && it->IsType("/area") && it->type_path == type_path_)) {
                    tile->objects.erase(it);
                    break;
                }
            }
        }
    } else {
        // For obj/mob, remove the last instance of this type that was added
        for (auto it = tile->objects.rbegin(); it != tile->objects.rend(); ++it) {
            if (it->type_path == type_path_) {
                // Convert reverse iterator to forward iterator for erase
                tile->objects.erase(std::next(it).base());
                break;
            }
        }
    }

    // Invalidate tile cache
    tile->cached_sorted_.clear();
    tile->cache_valid_ = false;
    tile->cached_area_ = nullptr;
}

void PlaceObjectAction::Redo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) {
        // Tile doesn't exist, use PlaceObject to create it
        map->PlaceObject(x_, y_, z_, type_path_);
        return;
    }

    // Create the new object instance
    ObjectInstance new_obj;
    new_obj.type_path = type_path_;

    // Determine object type category
    bool is_turf = type_path_.find("/turf") == 0;
    bool is_area = type_path_.find("/area") == 0;

    // Handle placement based on type
    if (is_turf) {
        // Replace existing turf
        bool found_turf = false;
        for (auto& obj : tile->objects) {
            if (obj.IsType("/turf")) {
                previous_object_ = obj;  // Store for undo
                replaced_existing_ = true;
                obj = new_obj;
                found_turf = true;
                break;
            }
        }

        if (!found_turf) {
            tile->objects.push_back(new_obj);
            replaced_existing_ = false;
        }
    } else if (is_area) {
        // Replace existing area
        bool found_area = false;
        for (auto& obj : tile->objects) {
            if (obj.IsType("/area")) {
                previous_object_ = obj;  // Store for undo
                replaced_existing_ = true;
                obj = new_obj;
                found_area = true;
                break;
            }
        }

        if (!found_area) {
            tile->objects.push_back(new_obj);
            replaced_existing_ = false;
        }
    } else {
        // Append to object list
        tile->objects.push_back(new_obj);
        replaced_existing_ = false;
    }

    // Invalidate tile cache
    tile->cached_sorted_.clear();
    tile->cache_valid_ = false;
    tile->cached_area_ = nullptr;
}

// DeleteObjectAction implementation

DeleteObjectAction::DeleteObjectAction(int x, int y, int z, size_t object_index, const ObjectInstance& object_to_delete)
    : x_(x), y_(y), z_(z), object_index_(object_index), deleted_object_(object_to_delete)
{
}

void DeleteObjectAction::Undo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) return;

    // Restore the deleted object at the original index
    if (object_index_ <= tile->objects.size()) {
        tile->objects.insert(tile->objects.begin() + object_index_, deleted_object_);
    } else {
        // If index is out of bounds, append to end
        tile->objects.push_back(deleted_object_);
    }

    // Invalidate tile cache
    tile->cached_sorted_.clear();
    tile->cache_valid_ = false;
    tile->cached_area_ = nullptr;
}

void DeleteObjectAction::Redo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) return;

    // Delete the object at the specified index
    if (object_index_ < tile->objects.size()) {
        tile->objects.erase(tile->objects.begin() + object_index_);
    }

    // Invalidate tile cache
    tile->cached_sorted_.clear();
    tile->cache_valid_ = false;
    tile->cached_area_ = nullptr;
}

// ModifyVariableAction implementation

ModifyVariableAction::ModifyVariableAction(int x, int y, int z, size_t object_index,
                                          const std::string& var_name,
                                          const std::string& old_value,
                                          const std::string& new_value)
    : x_(x), y_(y), z_(z), object_index_(object_index),
      var_name_(var_name), old_value_(old_value), new_value_(new_value)
{
}

void ModifyVariableAction::Undo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) return;

    if (object_index_ >= tile->objects.size()) return;

    ObjectInstance& obj = tile->objects[object_index_];

    // Restore old value
    if (old_value_.empty()) {
        // Variable didn't exist before, remove it
        obj.vars.erase(var_name_);
    } else {
        obj.vars[var_name_] = old_value_;
    }

    // Invalidate tile cache if visual properties changed
    if (var_name_ == "icon" || var_name_ == "icon_state" || var_name_ == "dir" ||
        var_name_ == "plane" || var_name_ == "layer" || var_name_ == "alpha" ||
        var_name_ == "color" || var_name_ == "pixel_x" || var_name_ == "pixel_y") {
        tile->cached_sorted_.clear();
        tile->cache_valid_ = false;
        tile->cached_area_ = nullptr;
    }
}

void ModifyVariableAction::Redo(Map* map) {
    if (!map) return;

    TileInstance* tile = map->GetTile(x_, y_, z_);
    if (!tile) return;

    if (object_index_ >= tile->objects.size()) return;

    ObjectInstance& obj = tile->objects[object_index_];

    // Apply new value
    obj.vars[var_name_] = new_value_;

    // Invalidate tile cache if visual properties changed
    if (var_name_ == "icon" || var_name_ == "icon_state" || var_name_ == "dir" ||
        var_name_ == "plane" || var_name_ == "layer" || var_name_ == "alpha" ||
        var_name_ == "color" || var_name_ == "pixel_x" || var_name_ == "pixel_y") {
        tile->cached_sorted_.clear();
        tile->cache_valid_ = false;
        tile->cached_area_ = nullptr;
    }
}

} // namespace myg
