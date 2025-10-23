#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <memory>
#include <string>

namespace myg {

class InputHandler;
class ProjectManager;

/**
 * Main application class for MYG Editor
 * Manages SDL3 window, OpenGL context, and application lifecycle
 */
class MYGEditor {
public:
    MYGEditor();
    ~MYGEditor();

    /**
     * Initialize SDL3, create window and OpenGL context, set up ImGui
     * @return true if initialization succeeded, false otherwise
     */
    bool Initialize();

    /**
     * Run the main event loop
     */
    void Run();

    /**
     * Clean up resources and shutdown
     */
    void Shutdown();

    /**
     * Check if the application is running
     */
    bool IsRunning() const { return running_; }

    /**
     * Request application shutdown
     */
    void RequestShutdown() { running_ = false; }

    /**
     * Get the input handler
     */
    InputHandler* GetInputHandler() { return input_handler_.get(); }

    /**
     * Get the project manager
     */
    ProjectManager* GetProjectManager() { return project_manager_.get(); }

    /**
     * Set the window title
     */
    void SetWindowTitle(const std::string& title);

private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    bool running_;
    bool initialized_;

    // Window properties
    int window_width_;
    int window_height_;

    // Input handling
    std::unique_ptr<InputHandler> input_handler_;

    // Project management
    std::unique_ptr<ProjectManager> project_manager_;

    /**
     * Process SDL events
     */
    void ProcessEvents();

    /**
     * Update application state
     * @param delta_time Time elapsed since last frame in seconds
     */
    void Update(float delta_time);

    /**
     * Render the frame
     */
    void Render();

    /**
     * Initialize ImGui with SDL3 and OpenGL backends
     */
    bool InitializeImGui();

    /**
     * Shutdown ImGui
     */
    void ShutdownImGui();
};

} // namespace myg
