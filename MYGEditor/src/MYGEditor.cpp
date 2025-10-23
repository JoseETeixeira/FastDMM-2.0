#include "myg/MYGEditor.h"
#include "myg/InputHandler.h"
#include "myg/ProjectManager.h"
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

    // Initialize ImGui
    if (!InitializeImGui()) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
        SDL_GL_DestroyContext(gl_context_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        return false;
    }

    std::cout << "MYG Editor initialized successfully!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    initialized_ = true;
    running_ = true;

    return true;
}

bool MYGEditor::InitializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    
    // Enable docking if available
    #ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    #endif

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL3_InitForOpenGL(window_, gl_context_)) {
        std::cerr << "Failed to initialize ImGui SDL3 backend" << std::endl;
        return false;
    }

    // Determine GLSL version based on OpenGL version
    const char* glsl_version = "#version 330";
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend" << std::endl;
        ImGui_ImplSDL3_Shutdown();
        return false;
    }

    return true;
}

void MYGEditor::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
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
        // Let ImGui handle the event first
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
                break;
        }
    }
}

void MYGEditor::Update(float delta_time) {
    // Update input handler state
    input_handler_->Update();

    // Application state updates will go here
    // For now, this is a placeholder
}

void MYGEditor::Render() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Render ImGui demo window for testing
    ImGui::ShowDemoWindow();

    // Render ImGui
    ImGui::Render();

    // Clear screen
    glViewport(0, 0, window_width_, window_height_);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render ImGui draw data
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    SDL_GL_SwapWindow(window_);
}

void MYGEditor::Shutdown() {
    if (!initialized_) {
        return;
    }

    std::cout << "Shutting down MYG Editor..." << std::endl;

    // Shutdown ImGui
    ShutdownImGui();

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

} // namespace myg
