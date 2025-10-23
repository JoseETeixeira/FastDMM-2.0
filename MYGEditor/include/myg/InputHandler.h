#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include <functional>

namespace myg {

/**
 * Handles user input from keyboard and mouse
 * Tracks input state and provides input mapping for common operations
 */
class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    /**
     * Process an SDL event
     * @param event The SDL event to process
     */
    void ProcessEvent(const SDL_Event& event);

    /**
     * Update input state (call once per frame)
     */
    void Update();

    /**
     * Check if a key is currently pressed
     * @param key The SDL keycode to check
     * @return true if the key is pressed
     */
    bool IsKeyPressed(SDL_Keycode key) const;

    /**
     * Check if a key was just pressed this frame
     * @param key The SDL keycode to check
     * @return true if the key was just pressed
     */
    bool IsKeyJustPressed(SDL_Keycode key) const;

    /**
     * Check if a mouse button is currently pressed
     * @param button The mouse button (SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT)
     * @return true if the button is pressed
     */
    bool IsMouseButtonPressed(int button) const;

    /**
     * Check if a mouse button was just pressed this frame
     * @param button The mouse button to check
     * @return true if the button was just pressed
     */
    bool IsMouseButtonJustPressed(int button) const;

    /**
     * Get current mouse position
     * @param x Output parameter for X coordinate
     * @param y Output parameter for Y coordinate
     */
    void GetMousePosition(int& x, int& y) const;

    /**
     * Get mouse movement delta since last frame
     * @param dx Output parameter for X delta
     * @param dy Output parameter for Y delta
     */
    void GetMouseDelta(int& dx, int& dy) const;

    /**
     * Get mouse wheel delta
     * @return Mouse wheel delta (positive = scroll up, negative = scroll down)
     */
    int GetMouseWheelDelta() const;

    /**
     * Check if middle mouse button is being dragged
     * @return true if middle mouse is being dragged
     */
    bool IsMiddleMouseDragging() const { return middle_mouse_dragging_; }

    /**
     * Get the starting position of the current middle mouse drag
     * @param x Output parameter for X coordinate
     * @param y Output parameter for Y coordinate
     */
    void GetDragStartPosition(int& x, int& y) const;

    /**
     * Check if Ctrl key is pressed
     */
    bool IsCtrlPressed() const;

    /**
     * Check if Shift key is pressed
     */
    bool IsShiftPressed() const;

    /**
     * Check if Alt key is pressed
     */
    bool IsAltPressed() const;

    /**
     * Check for common keyboard shortcuts
     */
    bool IsUndoPressed() const;      // Ctrl+Z
    bool IsRedoPressed() const;      // Ctrl+Y
    bool IsSavePressed() const;      // Ctrl+S
    bool IsOpenPressed() const;      // Ctrl+O
    bool IsCopyPressed() const;      // Ctrl+C
    bool IsPastePressed() const;     // Ctrl+V
    bool IsCutPressed() const;       // Ctrl+X

private:
    // Keyboard state
    std::unordered_map<SDL_Keycode, bool> key_states_;
    std::unordered_map<SDL_Keycode, bool> key_just_pressed_;

    // Mouse state
    std::unordered_map<int, bool> mouse_button_states_;
    std::unordered_map<int, bool> mouse_button_just_pressed_;
    int mouse_x_;
    int mouse_y_;
    int mouse_dx_;
    int mouse_dy_;
    int mouse_wheel_delta_;

    // Middle mouse dragging
    bool middle_mouse_dragging_;
    int drag_start_x_;
    int drag_start_y_;

    // Previous frame state for detecting "just pressed"
    std::unordered_map<SDL_Keycode, bool> prev_key_states_;
    std::unordered_map<int, bool> prev_mouse_button_states_;

    /**
     * Handle key press event
     */
    void HandleKeyPress(SDL_Keycode key);

    /**
     * Handle key release event
     */
    void HandleKeyRelease(SDL_Keycode key);

    /**
     * Handle mouse button press event
     */
    void HandleMouseButtonPress(int button, int x, int y);

    /**
     * Handle mouse button release event
     */
    void HandleMouseButtonRelease(int button);

    /**
     * Handle mouse motion event
     */
    void HandleMouseMotion(int x, int y);

    /**
     * Handle mouse wheel event
     */
    void HandleMouseWheel(int delta);
};

} // namespace myg
