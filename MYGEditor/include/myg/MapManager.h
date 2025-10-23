#pragma once

#include <string>
#include <vector>
#include <memory>

namespace DMCompiler {
    class DMObjectTree;
}

namespace myg {

class Map;

class MapManager {
public:
    MapManager();
    ~MapManager();

    bool LoadMap(const std::string& dmm_path, DMCompiler::DMObjectTree* tree);
    bool SaveMap(const std::string& dmm_path);
    void CloseMap(int index);

    Map* GetCurrentMap();
    const std::vector<std::unique_ptr<Map>>& GetOpenMaps() const { return open_maps_; }
    void SetActiveMap(int index);
    int GetActiveMapIndex() const { return active_map_index_; }

private:
    std::vector<std::unique_ptr<Map>> open_maps_;
    int active_map_index_;
};

} // namespace myg
