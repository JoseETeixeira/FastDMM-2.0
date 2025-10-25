#include "myg/GUIManager.h"
#include "myg/MapManager.h"
#include "myg/Map.h"
#include "myg/TileInstance.h"
#include "myg/Camera.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <algorithm>

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
    , current_z_level_(1)
    , show_error_dialog_(false)
    , show_compilation_dialog_(false)
    , compilation_progress_(0.0f)
    , show_loading_dialog_(false)
    , loading_progress_(0.0f)
    , show_unsaved_changes_dialog_(false)
    , unsaved_changes_result_(0)
    , pending_close_map_index_(-1)
    , show_context_menu_(false)
    , context_menu_x_(0)
    , context_menu_y_(0)
    , context_menu_tile_(nullptr)
    , context_menu_selected_object_(-1)
    , context_menu_object_tree_(nullptr)
    , show_variable_editor_(false)
    , variable_editor_object_(nullptr)
    , variable_editor_object_tree_(nullptr)
    , variable_editor_confirmed_(false)
    , show_keyboard_shortcuts_dialog_(false)
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

void GUIManager::RenderMainMenuBar(const MenuCallbacks& callbacks, Map* current_map) {
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
            // Enable/disable undo based on map state
            bool can_undo = current_map && current_map->CanUndo();
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, can_undo)) {
                if (callbacks.on_undo) {
                    callbacks.on_undo();
                }
            }
            
            // Enable/disable redo based on map state
            bool can_redo = current_map && current_map->CanRedo();
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, can_redo)) {
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
            if (ImGui::MenuItem("Keyboard Shortcuts", "F1")) {
                ShowKeyboardShortcutsDialog();
            }
            
            ImGui::Separator();
            
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
                // Check for unsaved changes
                if (map->IsModified()) {
                    // Extract filename from path
                    std::string close_filename = map->GetFilePath();
                    size_t last_slash = close_filename.find_last_of("/\\");
                    if (last_slash != std::string::npos) {
                        close_filename = close_filename.substr(last_slash + 1);
                    }
                    
                    // Show unsaved changes dialog
                    ShowUnsavedChangesDialog(close_filename);
                    
                    // Store the map index to close after dialog is handled
                    pending_close_map_index_ = static_cast<int>(i);
                } else {
                    // No unsaved changes, close immediately
                    map_manager->CloseMap(static_cast<int>(i));
                }
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

void GUIManager::RenderStatusBar(const std::string& map_file_path, int tile_x, int tile_y, int tile_z, float zoom, int min_z, int max_z) {
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
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        
        // Z-level selector
        ImGui::Text("Z-Level:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputInt("##zlevel", &current_z_level_, 1, 1)) {
            // Clamp Z-level to valid range
            if (current_z_level_ < min_z) {
                current_z_level_ = min_z;
            }
            if (current_z_level_ > max_z) {
                current_z_level_ = max_z;
            }
        }
        
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

void GUIManager::ShowTileContextMenu(int screen_x, int screen_y, TileInstance* tile, ::DMCompiler::DMObjectTree* object_tree) {
    show_context_menu_ = true;
    context_menu_x_ = screen_x;
    context_menu_y_ = screen_y;
    context_menu_tile_ = tile;
    context_menu_object_tree_ = object_tree;
    context_menu_selected_object_ = -1;
}

int GUIManager::RenderTileContextMenu() {
    if (!show_context_menu_ || !context_menu_tile_) {
        return 0;
    }

    int action = 0;

    // Open context menu popup on first frame
    static bool popup_opened = false;
    if (!popup_opened) {
        ImGui::OpenPopup("TileContextMenu");
        popup_opened = true;
    }

    // Set popup position
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(context_menu_x_), static_cast<float>(context_menu_y_)), ImGuiCond_Appearing);
    
    if (ImGui::BeginPopup("TileContextMenu")) {
        ImGui::Text("Tile Objects:");
        ImGui::Separator();

        // List all objects on the tile
        for (size_t i = 0; i < context_menu_tile_->objects.size(); ++i) {
            const auto& obj = context_menu_tile_->objects[i];
            
            bool is_selected = (context_menu_selected_object_ == static_cast<int>(i));
            if (ImGui::Selectable(obj.type_path.c_str(), is_selected)) {
                context_menu_selected_object_ = static_cast<int>(i);
            }
        }

        ImGui::Separator();

        // Context menu actions
        if (context_menu_selected_object_ >= 0 && 
            context_menu_selected_object_ < static_cast<int>(context_menu_tile_->objects.size())) {
            
            if (ImGui::MenuItem("Edit Variables")) {
                action = 1;
                show_context_menu_ = false;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Delete Object")) {
                action = 2;
                show_context_menu_ = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Move to Top")) {
                action = 3;
                show_context_menu_ = false;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Move to Bottom")) {
                action = 4;
                show_context_menu_ = false;
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::TextDisabled("Select an object first");
        }

        ImGui::EndPopup();
    } else {
        // Popup was closed
        show_context_menu_ = false;
        popup_opened = false;
    }

    return action;
}

ObjectInstance* GUIManager::GetContextMenuSelectedObject() {
    if (!context_menu_tile_ || context_menu_selected_object_ < 0 || 
        context_menu_selected_object_ >= static_cast<int>(context_menu_tile_->objects.size())) {
        return nullptr;
    }

    return &context_menu_tile_->objects[context_menu_selected_object_];
}

void GUIManager::ShowVariableEditorDialog(ObjectInstance* object, ::DMCompiler::DMObjectTree* object_tree) {
    if (!object) {
        return;
    }

    show_variable_editor_ = true;
    variable_editor_object_ = object;
    variable_editor_object_tree_ = object_tree;
    variable_editor_confirmed_ = false;

    // Copy current variable values to editor
    variable_editor_values_.clear();
    for (const auto& [key, value] : object->vars) {
        variable_editor_values_[key] = value;
    }

    // If no variables are set, add some common ones as empty
    if (variable_editor_values_.empty()) {
        variable_editor_values_["icon"] = "";
        variable_editor_values_["icon_state"] = "";
        variable_editor_values_["dir"] = "";
        variable_editor_values_["name"] = "";
    }
}

bool GUIManager::RenderVariableEditorDialog() {
    if (!show_variable_editor_ || !variable_editor_object_) {
        return false;
    }

    bool confirmed = false;

    // Center the dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);

    if (ImGui::Begin("Edit Variables", &show_variable_editor_, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Object Type: %s", variable_editor_object_->type_path.c_str());
        ImGui::Separator();

        // Display editable variables
        if (ImGui::BeginChild("VariableList", ImVec2(0, -40), true)) {
            // Create a list of variable names for iteration
            std::vector<std::string> var_names;
            for (const auto& [key, value] : variable_editor_values_) {
                var_names.push_back(key);
            }

            // Sort for consistent display
            std::sort(var_names.begin(), var_names.end());

            for (const auto& var_name : var_names) {
                ImGui::PushID(var_name.c_str());

                // Variable name label
                ImGui::Text("%s:", var_name.c_str());
                ImGui::SameLine();

                // Variable value input
                char buffer[256];
                std::string& value = variable_editor_values_[var_name];
                strncpy_s(buffer, sizeof(buffer), value.c_str(), _TRUNCATE);

                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
                    value = buffer;
                }

                ImGui::PopID();
            }

            // Add new variable button
            ImGui::Separator();
            static char new_var_name[64] = "";
            ImGui::Text("Add Variable:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("##newvar", new_var_name, sizeof(new_var_name));
            ImGui::SameLine();
            if (ImGui::Button("Add") && strlen(new_var_name) > 0) {
                variable_editor_values_[new_var_name] = "";
                new_var_name[0] = '\0';
            }
        }
        ImGui::EndChild();

        // Buttons
        ImGui::Separator();
        float button_width = 100.0f;
        float spacing = 10.0f;
        float total_width = button_width * 2 + spacing;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - total_width) * 0.5f);

        if (ImGui::Button("OK", ImVec2(button_width, 0))) {
            // Apply changes to the object
            variable_editor_object_->vars.clear();
            for (const auto& [key, value] : variable_editor_values_) {
                if (!value.empty()) {
                    variable_editor_object_->vars[key] = value;
                }
            }

            confirmed = true;
            variable_editor_confirmed_ = true;
            show_variable_editor_ = false;
        }

        ImGui::SameLine(0, spacing);

        if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
            show_variable_editor_ = false;
        }

        ImGui::End();
    }

    return confirmed;
}

int GUIManager::ShowUnsavedChangesDialog(const std::string& filename) {
    unsaved_changes_filename_ = filename;
    unsaved_changes_result_ = 0;
    
    // Open the popup if not already open
    if (!show_unsaved_changes_dialog_) {
        show_unsaved_changes_dialog_ = true;
        ImGui::OpenPopup("Unsaved Changes");
    }
    
    return unsaved_changes_result_;
}

int GUIManager::RenderUnsavedChangesDialog() {
    if (!show_unsaved_changes_dialog_) {
        return 0;
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    bool is_open = show_unsaved_changes_dialog_;
    if (ImGui::BeginPopupModal("Unsaved Changes", &is_open, 
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        // Display message
        ImGui::Text("The file '%s' has unsaved changes.", unsaved_changes_filename_.c_str());
        ImGui::Text("Do you want to save your changes?");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Buttons
        float button_width = 100.0f;
        float spacing = 10.0f;
        float total_width = button_width * 3 + spacing * 2;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - total_width) * 0.5f);
        
        if (ImGui::Button("Save", ImVec2(button_width, 0))) {
            unsaved_changes_result_ = 1;
            show_unsaved_changes_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine(0, spacing);
        
        if (ImGui::Button("Discard", ImVec2(button_width, 0))) {
            unsaved_changes_result_ = 2;
            show_unsaved_changes_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine(0, spacing);
        
        if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
            unsaved_changes_result_ = 3;
            show_unsaved_changes_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Update state if dialog was closed
    if (!is_open) {
        show_unsaved_changes_dialog_ = false;
        unsaved_changes_result_ = 3; // Treat close as cancel
    }
    
    return unsaved_changes_result_;
}

void GUIManager::CloseUnsavedChangesDialog() {
    show_unsaved_changes_dialog_ = false;
    unsaved_changes_result_ = 0;
}

bool GUIManager::ShowLoadingDialog(const std::string& message, float progress) {
    loading_message_ = message;
    loading_progress_ = progress;
    
    // Open the popup if not already open
    if (!show_loading_dialog_) {
        show_loading_dialog_ = true;
        ImGui::OpenPopup("Loading");
    }
    
    return show_loading_dialog_;
}

void GUIManager::CloseLoadingDialog() {
    show_loading_dialog_ = false;
}

void GUIManager::RenderLoadingDialog() {
    if (!show_loading_dialog_) {
        return;
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    bool is_open = show_loading_dialog_;
    if (ImGui::BeginPopupModal("Loading", &is_open, 
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        // Display loading message
        ImGui::Text("%s", loading_message_.c_str());
        ImGui::Spacing();
        
        // Display progress bar
        char progress_text[32];
        snprintf(progress_text, sizeof(progress_text), "%.0f%%", loading_progress_ * 100.0f);
        ImGui::ProgressBar(loading_progress_, ImVec2(400, 0), progress_text);
        
        ImGui::Spacing();
        
        // Show close button when loading is complete
        if (loading_progress_ >= 1.0f) {
            ImGui::Separator();
            
            // Center the button
            float button_width = 120.0f;
            float window_width = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
            
            if (ImGui::Button("Close", ImVec2(button_width, 0))) {
                show_loading_dialog_ = false;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::EndPopup();
    }
    
    // Update state if dialog was closed
    if (!is_open) {
        show_loading_dialog_ = false;
    }
}

void GUIManager::RenderViewportOverlay(const std::string& message) {
    if (message.empty()) {
        return;
    }

    // Get main viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Calculate center position
    ImVec2 center = viewport->GetCenter();
    
    // Create an overlay window
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.7f); // Semi-transparent background
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoInputs;
    
    if (ImGui::Begin("ViewportOverlay", nullptr, flags)) {
        // Display message with larger font
        ImGui::SetWindowFontScale(2.0f);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", message.c_str());
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::End();
    }
}

void GUIManager::ShowKeyboardShortcutsDialog() {
    show_keyboard_shortcuts_dialog_ = true;
    ImGui::OpenPopup("Keyboard Shortcuts");
}

void GUIManager::RenderKeyboardShortcutsDialog() {
    if (!show_keyboard_shortcuts_dialog_) {
        return;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);

    bool is_open = show_keyboard_shortcuts_dialog_;
    if (ImGui::BeginPopupModal("Keyboard Shortcuts", &is_open, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("MYG Editor Keyboard Shortcuts");
        ImGui::Separator();
        ImGui::Spacing();

        // Create a table for shortcuts
        if (ImGui::BeginTable("ShortcutsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            // Set column widths
            ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            // File operations
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Ctrl+Shift+O");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Open Project");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Ctrl+O");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Open Map");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Ctrl+S");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Save Map");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Alt+F4");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Exit Application");

            // Edit operations
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Ctrl+Z");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Undo");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Ctrl+Y");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Redo");

            // Project operations
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("F5");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Compile Project");

            // Viewport operations
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Left Click");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Place Selected Object");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Right Click");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Open Tile Context Menu");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Middle Mouse Drag");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Pan Camera");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Mouse Wheel");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Zoom Camera");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Delete");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Delete Selected Object");

            // Help
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("F1");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Show Keyboard Shortcuts");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Esc");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Close Application");

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Center the close button
        float button_width = 120.0f;
        float window_width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX((window_width - button_width) * 0.5f);

        if (ImGui::Button("Close", ImVec2(button_width, 0))) {
            show_keyboard_shortcuts_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Update state if dialog was closed
    if (!is_open) {
        show_keyboard_shortcuts_dialog_ = false;
    }
}

} // namespace myg
