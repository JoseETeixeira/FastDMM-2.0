#include "myg/GUIManager.h"
#include "myg/MapManager.h"
#include "myg/Map.h"
#include "myg/TileInstance.h"
#include "myg/Camera.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

// Include DMCompiler headers
#include "DMObjectTree.h"
#include "DMObject.h"

namespace myg {

GUIManager::GUIManager()
    : initialized_(false)
    , window_(nullptr)
    , gl_context_(nullptr)
    , show_object_tree_(true)
    , show_inspector_(true)
    , show_demo_window_(false)
    , selected_z_level_(1)
    , show_error_dialog_(false)
    , show_compilation_dialog_(false)
    , compilation_progress_(0.0f)
{
}

GUIManager::~GUIManager() {
    if (initialized_) {
        Shutdown();
    }
}

bool GUIManager::Initialize(SDL_Window* window, SDL_GLContext gl_context) {
    if (initialized_) {
        std::cerr << "GUIManager already initialized" << std::endl;
        return false;
    }

    window_ = window;
    gl_context_ = gl_context;

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
    SetupStyle();

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

    initialized_ = true;
    return true;
}

void GUIManager::Shutdown() {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    initialized_ = false;
}

void GUIManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GUIManager::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::SetupStyle() {
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
}

void GUIManager::RenderMainMenuBar(const MenuCallbacks& callbacks) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Project...", "Ctrl+Shift+O")) {
                if (callbacks.on_open_project) {
                    callbacks.on_open_project();
                }
            }
            
            if (ImGui::MenuItem("Open Map...", "Ctrl+O")) {
                if (callbacks.on_open_map) {
                    callbacks.on_open_map();
                }
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (callbacks.on_save_map) {
                    callbacks.on_save_map();
                }
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                if (callbacks.on_quit) {
                    callbacks.on_quit();
                }
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                if (callbacks.on_undo) {
                    callbacks.on_undo();
                }
            }
            
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                if (callbacks.on_redo) {
                    callbacks.on_redo();
                }
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Compile", "F5")) {
                if (callbacks.on_compile_project) {
                    callbacks.on_compile_project();
                }
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Object Tree", nullptr, show_object_tree_)) {
                show_object_tree_ = !show_object_tree_;
            }
            if (ImGui::MenuItem("Inspector", nullptr, show_inspector_)) {
                show_inspector_ = !show_inspector_;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Demo Window", nullptr, show_demo_window_)) {
                show_demo_window_ = !show_demo_window_;
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // TODO: Show about dialog
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Show demo window if enabled
    if (show_demo_window_) {
        ImGui::ShowDemoWindow(&show_demo_window_);
    }
}

void GUIManager::RenderObjectTreePanel(::DMCompiler::DMObjectTree* object_tree) {
    if (!show_object_tree_ || !object_tree) {
        return;
    }
    
    ImGui::Begin("Object Tree", &show_object_tree_);
    
    if (object_tree->AllObjects.empty()) {
        ImGui::TextDisabled("No object tree loaded");
    } else {
        // For now, display a flat list of all objects
        // TODO: Implement proper tree hierarchy when DMObject has Children member
        ImGui::Text("Objects: %zu", object_tree->AllObjects.size());
        ImGui::Separator();
        
        if (ImGui::BeginChild("ObjectList")) {
            for (const auto& obj : object_tree->AllObjects) {
                if (!obj) continue;
                
                std::string path_str = obj->Path.ToString();
                bool is_selected = (selected_object_path_ == path_str);
                
                if (ImGui::Selectable(path_str.c_str(), is_selected)) {
                    selected_object_path_ = path_str;
                }
                
                // Handle double-click to set as active placement object
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    selected_object_path_ = path_str;
                    // TODO: Set as active placement object
                }
            }
        }
        ImGui::EndChild();
    }
    
    ImGui::End();
}

void GUIManager::RenderObjectTreeNode(::DMCompiler::DMObject* obj, ::DMCompiler::DMObjectTree* object_tree) {
    // This method is currently unused but kept for future implementation
    // when DMObject has a Children member or we implement a way to get children
    if (!obj) {
        return;
    }
    
    std::string path_str = obj->Path.ToString();
    
    // Set up tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    
    // Check if this node is selected
    bool is_selected = (selected_object_path_ == path_str);
    if (is_selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    // Render the tree node
    ImGui::TreeNodeEx(path_str.c_str(), flags);
    
    // Handle selection
    if (ImGui::IsItemClicked()) {
        selected_object_path_ = path_str;
    }
    
    // Handle double-click to set as active placement object
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        selected_object_path_ = path_str;
        // TODO: Set as active placement object
    }
}

void GUIManager::RenderMapTabs(MapManager* map_manager) {
    if (!map_manager) {
        return;
    }
    
    const auto& open_maps = map_manager->GetOpenMaps();
    if (open_maps.empty()) {
        return;
    }
    
    if (ImGui::BeginTabBar("MapTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll)) {
        for (size_t i = 0; i < open_maps.size(); ++i) {
            const auto& map = open_maps[i];
            if (!map) {
                continue;
            }
            
            // Extract filename from path
            std::string filename = map->GetFilePath();
            size_t last_slash = filename.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                filename = filename.substr(last_slash + 1);
            }
            
            // Add modified indicator
            if (map->IsModified()) {
                filename += " *";
            }
            
            // Render tab
            bool tab_open = true;
            if (ImGui::BeginTabItem(filename.c_str(), &tab_open)) {
                // Set this as the active map
                if (map_manager->GetActiveMapIndex() != static_cast<int>(i)) {
                    map_manager->SetActiveMap(static_cast<int>(i));
                }
                
                ImGui::EndTabItem();
            }
            
            // Handle tab close
            if (!tab_open) {
                // TODO: Check for unsaved changes
                map_manager->CloseMap(static_cast<int>(i));
            }
        }
        
        ImGui::EndTabBar();
    }
}

void GUIManager::RenderInspectorPanel(TileInstance* selected_tile, ::DMCompiler::DMObjectTree* object_tree) {
    if (!show_inspector_) {
        return;
    }
    
    ImGui::Begin("Inspector", &show_inspector_);
    
    if (!selected_tile) {
        ImGui::TextDisabled("No tile selected");
    } else {
        ImGui::Text("Selected Tile");
        ImGui::Separator();
        
        // Display objects on the tile
        const auto& objects = selected_tile->objects;
        
        if (objects.empty()) {
            ImGui::TextDisabled("No objects on this tile");
        } else {
            for (size_t i = 0; i < objects.size(); ++i) {
                const auto& obj = objects[i];
                
                // Create a collapsing header for each object
                std::string header = obj.type_path;
                if (ImGui::CollapsingHeader(header.c_str())) {
                    ImGui::Indent();
                    
                    // Display type path
                    ImGui::Text("Type: %s", obj.type_path.c_str());
                    
                    // Display variables
                    if (!obj.vars.empty()) {
                        ImGui::Separator();
                        ImGui::Text("Variables:");
                        
                        for (const auto& [var_name, var_value] : obj.vars) {
                            ImGui::BulletText("%s = %s", var_name.c_str(), var_value.c_str());
                        }
                    }
                    
                    ImGui::Unindent();
                }
            }
        }
    }
    
    ImGui::End();
}

void GUIManager::RenderStatusBar(const std::string& map_file_path, int tile_x, int tile_y, int tile_z, float zoom) {
    // Get main viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Position at bottom of window
    ImVec2 window_pos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 25);
    ImVec2 window_size = ImVec2(viewport->Size.x, 25);
    
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                             ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("StatusBar", nullptr, flags)) {
        // Display map file path
        if (!map_file_path.empty()) {
            ImGui::Text("Map: %s", map_file_path.c_str());
            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();
        }
        
        // Display tile coordinates
        ImGui::Text("Tile: (%d, %d, %d)", tile_x, tile_y, tile_z);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        
        // Display zoom level
        ImGui::Text("Zoom: %.0fpx", zoom);
        
        ImGui::End();
    }
}

bool GUIManager::ShowOpenProjectDialog(std::string& out_path) {
    // Use SDL3's native file dialog if available
    #if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_DialogFileFilter filter;
    filter.name = "Dream Maker Environment";
    filter.pattern = "*.dme";
    
    const char* result = nullptr;
    
    // SDL_ShowOpenFileDialog is async in SDL3, but we need sync behavior
    // For now, use a simple approach with SDL_ShowSimpleMessageBox as fallback
    // In a production implementation, you'd want to handle this asynchronously
    
    // Fallback to platform-specific implementation
    #endif
    
    #ifdef _WIN32
    return ShowOpenFileDialogWindows(out_path, "Dream Maker Environment (*.dme)\0*.dme\0All Files (*.*)\0*.*\0", "dme");
    #else
    // For non-Windows platforms, use a simple ImGui-based dialog
    return ShowOpenFileDialogImGui(out_path, ".dme");
    #endif
}

bool GUIManager::ShowOpenMapDialog(std::string& out_path) {
    #ifdef _WIN32
    return ShowOpenFileDialogWindows(out_path, "Dream Maker Map (*.dmm)\0*.dmm\0All Files (*.*)\0*.*\0", "dmm");
    #else
    // For non-Windows platforms, use a simple ImGui-based dialog
    return ShowOpenFileDialogImGui(out_path, ".dmm");
    #endif
}

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>

bool GUIManager::ShowOpenFileDialogWindows(std::string& out_path, const char* filter, const char* default_ext) {
    char filename[MAX_PATH] = "";
    
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open File";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = default_ext;
    
    if (GetOpenFileNameA(&ofn)) {
        out_path = filename;
        return true;
    }
    
    return false;
}
#endif

bool GUIManager::ShowOpenFileDialogImGui(std::string& out_path, const char* extension) {
    // Simple ImGui-based file browser
    // This is a basic implementation - a production version would be more sophisticated
    static bool dialog_open = false;
    static std::string current_path;
    static std::string selected_file;
    
    if (!dialog_open) {
        dialog_open = true;
        current_path = ".";
        selected_file.clear();
    }
    
    bool result = false;
    
    ImGui::OpenPopup("Open File");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);
    
    if (ImGui::BeginPopupModal("Open File", &dialog_open, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Current Path: %s", current_path.c_str());
        ImGui::Separator();
        
        // File list
        if (ImGui::BeginChild("FileList", ImVec2(0, -30))) {
            // TODO: List files in current_path with extension filter
            ImGui::TextDisabled("File browser not fully implemented");
            ImGui::TextDisabled("Please use the command line or drag-and-drop");
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        
        if (ImGui::Button("Open", ImVec2(120, 0))) {
            if (!selected_file.empty()) {
                out_path = selected_file;
                result = true;
                dialog_open = false;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            dialog_open = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    if (!dialog_open) {
        // Reset state when dialog closes
        current_path.clear();
        selected_file.clear();
    }
    
    return result;
}

bool GUIManager::ShowCompilationDialog(const std::string& message, float progress) {
    compilation_message_ = message;
    compilation_progress_ = progress;
    
    // Open the popup if not already open
    if (!show_compilation_dialog_) {
        show_compilation_dialog_ = true;
        ImGui::OpenPopup("Compiling Project");
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    bool is_open = show_compilation_dialog_;
    if (ImGui::BeginPopupModal("Compiling Project", &is_open, 
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        // Display compilation message
        ImGui::Text("%s", compilation_message_.c_str());
        ImGui::Spacing();
        
        // Display progress bar
        char progress_text[32];
        snprintf(progress_text, sizeof(progress_text), "%.0f%%", compilation_progress_ * 100.0f);
        ImGui::ProgressBar(compilation_progress_, ImVec2(400, 0), progress_text);
        
        ImGui::Spacing();
        
        // Show close button when compilation is complete
        if (compilation_progress_ >= 1.0f) {
            ImGui::Separator();
            
            // Center the button
            float button_width = 120.0f;
            float window_width = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
            
            if (ImGui::Button("Close", ImVec2(button_width, 0))) {
                show_compilation_dialog_ = false;
                ImGui::CloseCurrentPopup();
            }
        } else {
            // Show cancel button (future enhancement)
            ImGui::Separator();
            ImGui::TextDisabled("Cancel not yet implemented");
        }
        
        ImGui::EndPopup();
    }
    
    // Update state if dialog was closed
    if (!is_open) {
        show_compilation_dialog_ = false;
    }
    
    return show_compilation_dialog_;
}

void GUIManager::CloseCompilationDialog() {
    show_compilation_dialog_ = false;
}

void GUIManager::ShowErrorDialog(const std::string& title, const std::string& message) {
    error_dialog_title_ = title;
    error_dialog_message_ = message;
    
    // Open the popup if not already open
    if (!show_error_dialog_) {
        show_error_dialog_ = true;
        ImGui::OpenPopup(error_dialog_title_.c_str());
    }
}

void GUIManager::RenderErrorDialog() {
    if (!show_error_dialog_) {
        return;
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);
    
    bool is_open = show_error_dialog_;
    if (ImGui::BeginPopupModal(error_dialog_title_.c_str(), &is_open, ImGuiWindowFlags_NoResize)) {
        // Display error message in a scrollable text area
        ImGui::Text("Error Details:");
        ImGui::Separator();
        
        // Create a child window for scrollable text
        if (ImGui::BeginChild("ErrorMessage", ImVec2(0, -35), true, ImGuiWindowFlags_HorizontalScrollbar)) {
            ImGui::TextWrapped("%s", error_dialog_message_.c_str());
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        
        // Center the OK button
        float button_width = 120.0f;
        float window_width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
        
        if (ImGui::Button("OK", ImVec2(button_width, 0))) {
            show_error_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Update state if dialog was closed
    if (!is_open) {
        show_error_dialog_ = false;
    }
}

void GUIManager::CloseErrorDialog() {
    show_error_dialog_ = false;
}

} // namespace myg
