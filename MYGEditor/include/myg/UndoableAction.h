#pragma once

namespace myg {

// Base class for undoable actions
// Will be fully implemented in task 10
class UndoableAction {
public:
    virtual ~UndoableAction() = default;
    virtual void Undo() = 0;
    virtual void Redo() = 0;
};

} // namespace myg
