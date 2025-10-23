#include "myg/MapManager.h"
#include "myg/Map.h"
#include <iostream>

namespace myg {

MapManager::MapManager()
    : active_map_index_(-1)
{
}

MapManager::~MapManager() = default;

bool MapManager::LoadMap(const std::string& dmm_path, DMCompiler::DMObjectTree* tree) {
    // Create a new Map object
    auto map = std::make_unique<Map>();

    // Load the map file
    if (!map->Load(dmm_path, tree)) {
        std::cerr << "Failed to load map: " << dmm_path << std::endl;
        return false;
    }

    // Add to open maps
    open_maps_.push_back(std::move(map));

    // Set as active map
    active_map_index_ = static_cast<int>(open_maps_.size()) - 1;

    return true;
}

bool MapManager::SaveMap(const std::string& dmm_path) {
    if (active_map_index_ < 0 || active_map_index_ >= static_cast<int>(open_maps_.size())) {
        std::cerr << "No active map to save" << std::endl;
        return false;
    }

    return open_maps_[active_map_index_]->Save(dmm_path);
}

void MapManager::CloseMap(int index) {
    if (index < 0 || index >= static_cast<int>(open_maps_.size())) {
        std::cerr << "Invalid map index: " << index << std::endl;
        return;
    }

    // Remove the map from the list
    open_maps_.erase(open_maps_.begin() + index);

    // Update active map index
    if (open_maps_.empty()) {
        active_map_index_ = -1;
    } else if (active_map_index_ >= static_cast<int>(open_maps_.size())) {
        active_map_index_ = static_cast<int>(open_maps_.size()) - 1;
    } else if (active_map_index_ > index) {
        active_map_index_--;
    }
}

Map* MapManager::GetCurrentMap() {
    if (active_map_index_ < 0 || active_map_index_ >= static_cast<int>(open_maps_.size())) {
        return nullptr;
    }

    return open_maps_[active_map_index_].get();
}

void MapManager::SetActiveMap(int index) {
    if (index < 0 || index >= static_cast<int>(open_maps_.size())) {
        std::cerr << "Invalid map index: " << index << std::endl;
        return;
    }

    active_map_index_ = index;
}

} // namespace myg
