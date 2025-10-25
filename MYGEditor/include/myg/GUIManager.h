#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <functional>

namespace myg {

class MapManager;
class Map;
class TileInstance;
class ObjectInstance;
class Camera;

} // namespace myg

// Forward declaration - DMCompiler is in global namespace
namespace DMCompiler {
    class DMObjectTree;
    class DMObject;
}

namespace myg {

/**
 * GUIManager - Manages ImGui-based user interface
 * 
 * Responsibilities:
 * - Render all UI panels and windows
 * - Handle UI events and callbacks
 * - Manage dialog state
 * - Provide file picker dialogs
 * - Display compilation progress
 */
class GUIManager {
public:
    /**
     * Struct for menu action callbacks
     */
    struct MenuCallbacks {
        std::function<void()> on_open_project;
        std::function<void()> on_open_map;
        std::function<void()> on_save_map;
        std::function<void()> on_compile_project;
        std::function<void()> on_undo;
        std::function<void()> on_redo;
        std::function<void()> on_quit;
    };

    GUIManager();
    ~GUIManager();

    /**
     * Initialize ImGui with SDL3 and OpenGL backends
     * @param window SDL window
     * @param gl_context OpenGL context
     * @return true if initialization succeeded
     */
    bool Initialize(SDL_Window* window, SDL_GLContext gl_context);

    /**
     * Shutdown ImGui
     */
    void Shutdown();

    /**
     * Begin a new ImGui frame
     */
    void BeginFrame();

    /**
     * End the current ImGui frame and render draw data
     */
    void EndFrame();

    /**
     * Render the main menu bar
     * @param callbacks Callbacks for menu actions
     * @param current_map The currently active map (for undo/redo state)
     */
    void RenderMainMenuBar(const MenuCallbacks& callbacks, Map* current_map = nullptr);

    /**
     * Render the object tree panel
     * @param object_tree The object tree to display
     */
    void RenderObjectTreePanel(::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render map tabs for multiple open maps
     * @param map_manager The map manager with open maps
     */
    void RenderMapTabs(MapManager* map_manager);

    /**
     * Render the inspector panel for tile details
     * @param selected_tile The currently selected tile
     * @param object_tree The object tree for variable resolution
     */
    void RenderInspectorPanel(TileInstance* selected_tile, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render the status bar at the bottom of the window
     * @param map_file_path Current map file path
     * @param tile_x Tile X coordinate under mouse
     * @param tile_y Tile Y coordinate under mouse
     * @param tile_z Current Z-level
     * @param zoom Current zoom level
     * @param min_z Minimum Z-level in the map
     * @param max_z Maximum Z-level in the map
     */
    void RenderStatusBar(const std::string& map_file_path, int tile_x, int tile_y, int tile_z, float zoom, int min_z = 1, int max_z = 1);

    /**
     * Render viewport overlay text (e.g., "Loading...", "No map loaded")
     * @param message Message to display
     */
    void RenderViewportOverlay(const std::string& message);

    /**
     * Show keyboard shortcuts dialog
     */
    void ShowKeyboardShortcutsDialog();

    /**
     * Render keyboard shortcuts dialog if it's open
     * This should be called every frame after BeginFrame()
     */
    void RenderKeyboardShortcutsDialog();

    /**
     * Show file picker dialog for opening a project
     * @param out_path Output parameter for selected file path
     * @return true if a file was selected, false if cancelled
     */
    bool ShowOpenProjectDialog(std::string& out_path);

    /**
     * Show file picker dialog for opening a map
     * @param out_path Output parameter for selected file path
     * @return true if a file was selected, false if cancelled
     */
    bool ShowOpenMapDialog(std::string& out_path);

    /**
     * Show compilation progress dialog
     * @param message Status message
     * @param progress Progress value (0.0 to 1.0)
     * @return true if dialog is still open, false if closed
     */
    bool ShowCompilationDialog(const std::string& message, float progress);

    /**
     * Close the compilation progress dialog
     */
    void CloseCompilationDialog();

    /**
     * Show loading progress dialog
     * @param message Status message
     * @param progress Progress value (0.0 to 1.0)
     * @return true if dialog is still open, false if closed
     */
    bool ShowLoadingDialog(const std::string& message, float progress);

    /**
     * Close the loading progress dialog
     */
    void CloseLoadingDialog();

    /**
     * Render loading dialog if it's open
     * This should be called every frame after BeginFrame()
     */
    void RenderLoadingDialog();

    /**
     * Show error dialog
     * @param title Dialog title
     * @param message Error message
     */
    void ShowErrorDialog(const std::string& title, const std::string& message);

    /**
     * Render the error dialog if it's open
     * This should be called every frame after BeginFrame()
     */
    void RenderErrorDialog();

    /**
     * Close the error dialog
     */
    void CloseErrorDialog();

    /**
     * Show unsaved changes confirmation dialog
     * @param filename Name of the file with unsaved changes
     * @return 0=still open, 1=save, 2=discard, 3=cancel
     */
    int ShowUnsavedChangesDialog(const std::string& filename);

    /**
     * Render the unsaved changes dialog if it's open
     * This should be called every frame after BeginFrame()
     * @return 0=still open, 1=save, 2=discard, 3=cancel
     */
    int RenderUnsavedChangesDialog();

    /**
     * Close the unsaved changes dialog
     */
    void CloseUnsavedChangesDialog();

    /**
     * Get the pending close map index
     * @return Map index pending close, or -1 if none
     */
    int GetPendingCloseMapIndex() const { return pending_close_map_index_; }

    /**
     * Clear the pending close map index
     */
    void ClearPendingCloseMapIndex() { pending_close_map_index_ = -1; }

    /**
     * Get the selected object path from the object tree panel
     * @return Selected object path, or empty string if none selected
     */
    const std::string& GetSelectedObjectPath() const { return selected_object_path_; }

    /**
     * Show tile context menu at the specified screen position
     * @param screen_x Screen X coordinate
     * @param screen_y Screen Y coordinate
     * @param tile The tile to show context menu for
     * @param object_tree The object tree for variable resolution
     */
    void ShowTileContextMenu(int screen_x, int screen_y, TileInstance* tile, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render the tile context menu if it's open
     * This should be called every frame after BeginFrame()
     * @return Context menu action (0=none, 1=edit_vars, 2=delete, 3=move_top, 4=move_bottom)
     */
    int RenderTileContextMenu();

    /**
     * Get the selected object index from the context menu
     */
    int GetContextMenuSelectedObjectIndex() const { return context_menu_selected_object_; }

    /**
     * Get the selected object from the context menu
     * @return Pointer to the selected object, or nullptr if none selected
     */
    ObjectInstance* GetContextMenuSelectedObject();

    /**
     * Show variable editor dialog for an object
     * @param object The object instance to edit
     * @param object_tree The object tree for variable resolution
     */
    void ShowVariableEditorDialog(ObjectInstance* object, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render the variable editor dialog if it's open
     * This should be called every frame after BeginFrame()
     * @return true if changes were confirmed, false otherwise
     */
    bool RenderVariableEditorDialog();

    /**
     * Check if object tree panel is visible
     */
    bool IsObjectTreeVisible() const { return show_object_tree_; }

    /**
     * Set object tree panel visibility
     */
    void SetObjectTreeVisible(bool visible) { show_object_tree_ = visible; }

    /**
     * Check if inspector panel is visible
     */
    bool IsInspectorVisible() const { return show_inspector_; }

    /**
     * Set inspector panel visibility
     */
    void SetInspectorVisible(bool visible) { show_inspector_ = visible; }

    /**
     * Get the current Z-level from the GUI
     */
    int GetCurrentZLevel() const { return current_z_level_; }

    /**
     * Set the current Z-level in the GUI
     */
    void SetCurrentZLevel(int z_level) { current_z_level_ = z_level; }

private:
    bool initialized_;
    SDL_Window* window_;
    SDL_GLContext gl_context_;

    // Panel visibility
    bool show_object_tree_;
    bool show_inspector_;
    bool show_demo_window_;

    // Selected state
    std::string selected_object_path_;
    int selected_z_level_;  // Deprecated - use current_z_level_ instead
    int current_z_level_;   // Current Z-level for editing and rendering

    // Dialog state
    bool show_error_dialog_;
    std::string error_dialog_title_;
    std::string error_dialog_message_;

    bool show_compilation_dialog_;
    std::string compilation_message_;
    float compilation_progress_;

    bool show_loading_dialog_;
    std::string loading_message_;
    float loading_progress_;

    bool show_unsaved_changes_dialog_;
    std::string unsaved_changes_filename_;
    int unsaved_changes_result_;
    int pending_close_map_index_;

    // Context menu state
    bool show_context_menu_;
    int context_menu_x_;
    int context_menu_y_;
    TileInstance* context_menu_tile_;
    int context_menu_selected_object_;
    ::DMCompiler::DMObjectTree* context_menu_object_tree_;

    // Variable editor state
    bool show_variable_editor_;
    ObjectInstance* variable_editor_object_;
    ::DMCompiler::DMObjectTree* variable_editor_object_tree_;
    std::unordered_map<std::string, std::string> variable_editor_values_;
    bool variable_editor_confirmed_;

    bool show_keyboard_shortcuts_dialog_;

    /**
     * Render a single object tree node recursively
     * @param obj The object to render
     * @param object_tree The object tree for navigation
     */
    void RenderObjectTreeNode(::DMCompiler::DMObject* obj, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Set up ImGui style
     */
    void SetupStyle();

#ifdef _WIN32
    /**
     * Show Windows native file dialog
     * @param out_path Output parameter for selected file path
     * @param filter File filter string
     * @param default_ext Default file extension
     * @return true if a file was selected
     */
    bool ShowOpenFileDialogWindows(std::string& out_path, const char* filter, const char* default_ext);
#endif

    /**
     * Show ImGui-based file dialog (fallback for non-Windows platforms)
     * @param out_path Output parameter for selected file path
     * @param extension File extension filter
     * @return true if a file was selected
     */
    bool ShowOpenFileDialogImGui(std::string& out_path, const char* extension);
};

} // namespace myg
