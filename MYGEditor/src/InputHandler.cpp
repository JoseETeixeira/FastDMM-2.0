#include "myg/InputHandler.h"

namespace myg {

InputHandler::InputHandler()
    : mouse_x_(0)
    , mouse_y_(0)
    , mouse_dx_(0)
    , mouse_dy_(0)
    , mouse_wheel_delta_(0)
    , middle_mouse_dragging_(false)
    , drag_start_x_(0)
    , drag_start_y_(0)
{
}

InputHandler::~InputHandler() {
}

void InputHandler::ProcessEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            HandleKeyPress(event.key.key);
            break;

        case SDL_EVENT_KEY_UP:
            HandleKeyRelease(event.key.key);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            HandleMouseButtonPress(event.button.button, 
                                  static_cast<int>(event.button.x), 
                                  static_cast<int>(event.button.y));
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            HandleMouseButtonRelease(event.button.button);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            HandleMouseMotion(static_cast<int>(event.motion.x), 
                            static_cast<int>(event.motion.y));
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            HandleMouseWheel(static_cast<int>(event.wheel.y));
            break;
    }
}

void InputHandler::Update() {
    // Update previous frame state
    prev_key_states_ = key_states_;
    prev_mouse_button_states_ = mouse_button_states_;

    // Clear just pressed states
    key_just_pressed_.clear();
    mouse_button_just_pressed_.clear();

    // Detect just pressed keys
    for (const auto& [key, pressed] : key_states_) {
        if (pressed && !prev_key_states_[key]) {
            key_just_pressed_[key] = true;
        }
    }

    // Detect just pressed mouse buttons
    for (const auto& [button, pressed] : mouse_button_states_) {
        if (pressed && !prev_mouse_button_states_[button]) {
            mouse_button_just_pressed_[button] = true;
        }
    }

    // Reset mouse delta and wheel delta
    mouse_dx_ = 0;
    mouse_dy_ = 0;
    mouse_wheel_delta_ = 0;
}

void InputHandler::HandleKeyPress(SDL_Keycode key) {
    key_states_[key] = true;
}

void InputHandler::HandleKeyRelease(SDL_Keycode key) {
    key_states_[key] = false;
}

void InputHandler::HandleMouseButtonPress(int button, int x, int y) {
    mouse_button_states_[button] = true;

    // Start middle mouse drag
    if (button == SDL_BUTTON_MIDDLE) {
        middle_mouse_dragging_ = true;
        drag_start_x_ = x;
        drag_start_y_ = y;
    }
}

void InputHandler::HandleMouseButtonRelease(int button) {
    mouse_button_states_[button] = false;

    // End middle mouse drag
    if (button == SDL_BUTTON_MIDDLE) {
        middle_mouse_dragging_ = false;
    }
}

void InputHandler::HandleMouseMotion(int x, int y) {
    mouse_dx_ = x - mouse_x_;
    mouse_dy_ = y - mouse_y_;
    mouse_x_ = x;
    mouse_y_ = y;
}

void InputHandler::HandleMouseWheel(int delta) {
    mouse_wheel_delta_ = delta;
}

bool InputHandler::IsKeyPressed(SDL_Keycode key) const {
    auto it = key_states_.find(key);
    return it != key_states_.end() && it->second;
}

bool InputHandler::IsKeyJustPressed(SDL_Keycode key) const {
    auto it = key_just_pressed_.find(key);
    return it != key_just_pressed_.end() && it->second;
}

bool InputHandler::IsMouseButtonPressed(int button) const {
    auto it = mouse_button_states_.find(button);
    return it != mouse_button_states_.end() && it->second;
}

bool InputHandler::IsMouseButtonJustPressed(int button) const {
    auto it = mouse_button_just_pressed_.find(button);
    return it != mouse_button_just_pressed_.end() && it->second;
}

void InputHandler::GetMousePosition(int& x, int& y) const {
    x = mouse_x_;
    y = mouse_y_;
}

void InputHandler::GetMouseDelta(int& dx, int& dy) const {
    dx = mouse_dx_;
    dy = mouse_dy_;
}

int InputHandler::GetMouseWheelDelta() const {
    return mouse_wheel_delta_;
}

void InputHandler::GetDragStartPosition(int& x, int& y) const {
    x = drag_start_x_;
    y = drag_start_y_;
}

bool InputHandler::IsCtrlPressed() const {
    return IsKeyPressed(SDLK_LCTRL) || IsKeyPressed(SDLK_RCTRL);
}

bool InputHandler::IsShiftPressed() const {
    return IsKeyPressed(SDLK_LSHIFT) || IsKeyPressed(SDLK_RSHIFT);
}

bool InputHandler::IsAltPressed() const {
    return IsKeyPressed(SDLK_LALT) || IsKeyPressed(SDLK_RALT);
}

bool InputHandler::IsUndoPressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_Z);
}

bool InputHandler::IsRedoPressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_Y);
}

bool InputHandler::IsSavePressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_S);
}

bool InputHandler::IsOpenPressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_O);
}

bool InputHandler::IsCopyPressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_C);
}

bool InputHandler::IsPastePressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_V);
}

bool InputHandler::IsCutPressed() const {
    return IsCtrlPressed() && IsKeyJustPressed(SDLK_X);
}

} // namespace myg
