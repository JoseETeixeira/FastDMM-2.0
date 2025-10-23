#include "myg/MYGEditor.h"
#include "myg/InputHandler.h"
#include "myg/ProjectManager.h"
#include "myg/MapManager.h"
#include "myg/Map.h"
#include "myg/Renderer.h"
#include "myg/Camera.h"
#include "myg/GUIManager.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace myg {

MYGEditor::MYGEditor()
    : window_(nullptr)
    , gl_context_(nullptr)
    , running_(false)
    , initialized_(false)
    , window_width_(1280)
    , window_height_(720)
    , input_handler_(std::make_unique<InputHandler>())
    , project_manager_(std::make_unique<ProjectManager>())
    , map_manager_(std::make_unique<MapManager>())
    , renderer_(std::make_unique<Renderer>())
    , camera_(std::make_unique<Camera>())
    , gui_manager_(std::make_unique<GUIManager>())
{
}

MYGEditor::~MYGEditor() {
    if (initialized_) {
        Shutdown();
    }
}

bool MYGEditor::Initialize() {
    if (initialized_) {
        std::cerr << "MYGEditor already initialized" << std::endl;
        return false;
    }

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window
    window_ = SDL_CreateWindow(
        "MYG Editor",
        window_width_,
        window_height_,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Create OpenGL context
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    // Make context current
    SDL_GL_MakeCurrent(window_, gl_context_);

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    std::cout << "MYG Editor initialized successfully!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Initialize renderer
    if (!renderer_->Initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        SDL_GL_DestroyContext(gl_context_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    // Initialize camera
    camera_->SetViewportSize(window_width_, window_height_);
    camera_->SetPosition(0.0f, 0.0f);
    camera_->SetZoom(32.0f);  // Default 32 pixels per tile

    // Initialize GUI manager
    if (!gui_manager_->Initialize(window_, gl_context_)) {
        std::cerr << "Failed to initialize GUI manager" << std::endl;
        SDL_GL_DestroyContext(gl_context_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    initialized_ = true;
    running_ = true;

    return true;
}

void MYGEditor::Run() {
    if (!initialized_) {
        std::cerr << "Cannot run: MYGEditor not initialized" << std::endl;
        return;
    }

    Uint64 last_time = SDL_GetTicks();

    // Main event loop
    while (running_) {
        // Calculate delta time
        Uint64 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // Process events
        ProcessEvents();

        // Update application state
        Update(delta_time);

        // Render frame
        Render();
    }
}

void MYGEditor::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Let ImGui handle the event first (through ImGui_ImplSDL3_ProcessEvent in GUIManager)
        ImGui_ImplSDL3_ProcessEvent(&event);

        // Let InputHandler process the event
        input_handler_->ProcessEvent(event);

        // Handle application-level events
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running_ = false;
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == SDL_GetWindowID(window_)) {
                    running_ = false;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    running_ = false;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                window_width_ = event.window.data1;
                window_height_ = event.window.data2;
                glViewport(0, 0, window_width_, window_height_);
                renderer_->SetViewport(window_width_, window_height_);
                camera_->SetViewportSize(window_width_, window_height_);
                break;
        }
    }
}

void MYGEditor::Update(float delta_time) {
    // Update input handler state
    input_handler_->Update();

    // Handle camera controls
    // Middle mouse button drag to pan
    if (input_handler_->IsMiddleMouseDragging()) {
        int dx, dy;
        input_handler_->GetMouseDelta(dx, dy);
        camera_->Pan(static_cast<float>(dx), static_cast<float>(dy));
    }

    // Mouse wheel to zoom
    int wheel_delta = input_handler_->GetMouseWheelDelta();
    if (wheel_delta != 0) {
        int mouse_x, mouse_y;
        input_handler_->GetMousePosition(mouse_x, mouse_y);
        
        // Zoom in/out based on wheel direction
        float zoom_delta = wheel_delta * 4.0f;  // 4 pixels per wheel tick
        camera_->ZoomAt(zoom_delta, mouse_x, mouse_y);
    }

    // Application state updates will go here
}

void MYGEditor::Render() {
    // Clear screen
    glViewport(0, 0, window_width_, window_height_);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Render map viewport here

    // Start ImGui frame
    gui_manager_->BeginFrame();

    // Set up menu callbacks
    GUIManager::MenuCallbacks callbacks;
    callbacks.on_open_project = [this]() { OnOpenProject(); };
    callbacks.on_open_map = [this]() { OnOpenMap(); };
    callbacks.on_save_map = [this]() { OnSaveMap(); };
    callbacks.on_compile_project = [this]() { OnCompileProject(); };
    callbacks.on_undo = [this]() { OnUndo(); };
    callbacks.on_redo = [this]() { OnRedo(); };
    callbacks.on_quit = [this]() { OnQuit(); };

    // Render GUI elements
    gui_manager_->RenderMainMenuBar(callbacks);
    gui_manager_->RenderObjectTreePanel(project_manager_->GetObjectTree());
    gui_manager_->RenderMapTabs(map_manager_.get());
    gui_manager_->RenderInspectorPanel(nullptr, project_manager_->GetObjectTree());
    
    // Get mouse position for status bar
    int mouse_x, mouse_y;
    input_handler_->GetMousePosition(mouse_x, mouse_y);
    
    // Convert to tile coordinates (placeholder)
    int tile_x = 0, tile_y = 0, tile_z = 1;
    
    // Get current map path
    std::string map_path;
    if (map_manager_->GetCurrentMap()) {
        map_path = map_manager_->GetCurrentMap()->GetFilePath();
    }
    
    gui_manager_->RenderStatusBar(map_path, tile_x, tile_y, tile_z, camera_->GetZoom());

    // Render dialogs (these need to be rendered every frame to stay open)
    gui_manager_->RenderErrorDialog();

    // End ImGui frame
    gui_manager_->EndFrame();

    // Swap buffers
    SDL_GL_SwapWindow(window_);
}

void MYGEditor::Shutdown() {
    if (!initialized_) {
        return;
    }

    std::cout << "Shutting down MYG Editor..." << std::endl;

    // Shutdown GUI manager
    gui_manager_->Shutdown();

    // Cleanup SDL
    if (gl_context_) {
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();

    initialized_ = false;
    running_ = false;

    std::cout << "MYG Editor shutdown complete." << std::endl;
}

void MYGEditor::SetWindowTitle(const std::string& title) {
    if (window_) {
        SDL_SetWindowTitle(window_, title.c_str());
    }
}

void MYGEditor::OnOpenProject() {
    std::string path;
    if (gui_manager_->ShowOpenProjectDialog(path)) {
        if (project_manager_->LoadProject(path)) {
            SetWindowTitle("MYG Editor - " + project_manager_->GetProjectName());
        } else {
            gui_manager_->ShowErrorDialog("Error", "Failed to load project: " + project_manager_->GetLastError());
        }
    }
}

void MYGEditor::OnOpenMap() {
    if (!project_manager_->HasValidObjectTree()) {
        gui_manager_->ShowErrorDialog("Error", "Please load and compile a project first");
        return;
    }
    
    std::string path;
    if (gui_manager_->ShowOpenMapDialog(path)) {
        if (map_manager_->LoadMap(path, project_manager_->GetObjectTree())) {
            // Map loaded successfully
        } else {
            gui_manager_->ShowErrorDialog("Error", "Failed to load map");
        }
    }
}

void MYGEditor::OnSaveMap() {
    auto* current_map = map_manager_->GetCurrentMap();
    if (!current_map) {
        gui_manager_->ShowErrorDialog("Error", "No map is currently open");
        return;
    }
    
    if (current_map->Save(current_map->GetFilePath())) {
        // Map saved successfully
    } else {
        gui_manager_->ShowErrorDialog("Error", "Failed to save map");
    }
}

void MYGEditor::OnCompileProject() {
    if (project_manager_->GetProjectPath().empty()) {
        gui_manager_->ShowErrorDialog("Error", "No project loaded");
        return;
    }
    
    bool success = project_manager_->CompileProject([this](const std::string& message, float progress) {
        gui_manager_->ShowCompilationDialog(message, progress);
    });
    
    if (!success) {
        gui_manager_->ShowErrorDialog("Compilation Failed", project_manager_->GetLastError());
    }
}

void MYGEditor::OnUndo() {
    auto* current_map = map_manager_->GetCurrentMap();
    if (current_map) {
        current_map->Undo();
    }
}

void MYGEditor::OnRedo() {
    auto* current_map = map_manager_->GetCurrentMap();
    if (current_map) {
        current_map->Redo();
    }
}

void MYGEditor::OnQuit() {
    running_ = false;
}

} // namespace myg
