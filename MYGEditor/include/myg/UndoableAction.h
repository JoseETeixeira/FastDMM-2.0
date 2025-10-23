#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "TileInstance.h"

namespace myg {

class Map;

// Base class for undoable actions
class UndoableAction {
public:
    virtual ~UndoableAction() = default;
    virtual void Undo(Map* map) = 0;
    virtual void Redo(Map* map) = 0;
};

// Action for placing an object on a tile
class PlaceObjectAction : public UndoableAction {
public:
    PlaceObjectAction(int x, int y, int z, const std::string& type_path);
    
    void Undo(Map* map) override;
    void Redo(Map* map) override;

private:
    int x_, y_, z_;
    std::string type_path_;
    bool replaced_existing_;
    ObjectInstance previous_object_;  // Store the object that was replaced (for turfs/areas)
};

// Action for deleting an object from a tile
class DeleteObjectAction : public UndoableAction {
public:
    DeleteObjectAction(int x, int y, int z, size_t object_index, const ObjectInstance& object_to_delete);
    
    void Undo(Map* map) override;
    void Redo(Map* map) override;

private:
    int x_, y_, z_;
    size_t object_index_;
    ObjectInstance deleted_object_;  // Store the deleted object for undo
};

// Action for modifying a variable on an object
class ModifyVariableAction : public UndoableAction {
public:
    ModifyVariableAction(int x, int y, int z, size_t object_index,
                        const std::string& var_name,
                        const std::string& old_value,
                        const std::string& new_value);
    
    void Undo(Map* map) override;
    void Redo(Map* map) override;

private:
    int x_, y_, z_;
    size_t object_index_;
    std::string var_name_;
    std::string old_value_;
    std::string new_value_;
};

} // namespace myg
