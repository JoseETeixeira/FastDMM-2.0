#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "UndoableAction.h"

namespace DMCompiler {
    class DMObjectTree;
}

namespace myg {

class TileInstance;

struct Location {
    int x, y, z;

    bool operator==(const Location& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct LocationHash {
    size_t operator()(const Location& loc) const {
        return std::hash<int>()(loc.x) ^
               (std::hash<int>()(loc.y) << 1) ^
               (std::hash<int>()(loc.z) << 2);
    }
};

class Map {
public:
    struct Bounds {
        int min_x, min_y, min_z;
        int max_x, max_y, max_z;
    };

    Map();
    ~Map();

    bool Load(const std::string& path, DMCompiler::DMObjectTree* tree);
    bool Save(const std::string& path);

    TileInstance* GetTile(int x, int y, int z);
    void SetTile(int x, int y, int z, TileInstance* tile);

    /**
     * Place an object on a tile at the specified location
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @param type_path Object type path to place
     * @return true if placement succeeded
     */
    bool PlaceObject(int x, int y, int z, const std::string& type_path);

    const Bounds& GetBounds() const { return bounds_; }
    const std::string& GetFilePath() const { return file_path_; }
    bool IsModified() const { return modified_; }

    // Undo/Redo
    void PushUndoState(std::unique_ptr<UndoableAction> action);
    bool Undo();
    bool Redo();
    bool CanUndo() const { return !undo_stack_.empty(); }
    bool CanRedo() const { return !redo_stack_.empty(); }

private:
    std::string file_path_;
    Bounds bounds_;
    bool is_tgm_format_;
    int key_length_;

    // Map data: location -> tile key
    std::unordered_map<Location, std::string, LocationHash> tiles_;

    // Tile instances: key -> tile data
    std::unordered_map<std::string, std::unique_ptr<TileInstance>> instances_;

    // Available keys for new instances
    std::vector<std::string> unused_keys_;

    // Undo/Redo stacks
    std::vector<std::unique_ptr<UndoableAction>> undo_stack_;
    std::vector<std::unique_ptr<UndoableAction>> redo_stack_;

    bool modified_;
    DMCompiler::DMObjectTree* object_tree_;

    std::string AllocateKey();
    void ReleaseKey(const std::string& key);
    void GenerateKeys(int length);

    bool ParseDMM(const std::string& content);
    bool ParseTileDefinition(const std::string& line, std::string& key, std::string& definition);
    bool ParseMapGrid(const std::string& section);
};

} // namespace myg
