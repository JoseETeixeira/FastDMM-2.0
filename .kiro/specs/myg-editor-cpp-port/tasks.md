# Implementation Plan

- [x] 1. Set up project structure and build system

  - Create MYGEditor/ directory with CMake build configuration
  - Set up directory structure (src/, include/, external/, tests/)
  - Configure CMakeLists.txt to link against DMCompilerCpp library
  - Add SDL3, ImGui, and stb_image as dependencies
  - Create main.cpp with basic SDL3 window initialization
  - Verify project builds on target platform
  - _Requirements: 13.3, 13.4, 13.5_

- [x] 2. Implement core application framework

  - [x] 2.1 Create MYGEditor main class with SDL3 initialization

    - Implement MYGEditor::Initialize() to create SDL window and OpenGL context
    - Set up ImGui integration with SDL3 and OpenGL backend
    - Implement main event loop in MYGEditor::Run()
    - Add ProcessEvents(), Update(), and Render() methods
    - Handle window close and application shutdown
    - _Requirements: 13.1, 13.2_

  - [x] 2.2 Implement InputHandler for user input processing
    - Create InputHandler class to process SDL events
    - Track keyboard state (key press/release)
    - Track mouse state (position, buttons, wheel)
    - Implement mouse delta tracking for camera panning
    - Add input mapping for common operations (Ctrl+Z, Ctrl+S, etc.)
    - _Requirements: 6.1, 6.2_

- [x] 3. Implement ProjectManager and compilation integration

  - [x] 3.1 Create ProjectManager class for DME project loading

    - Implement LoadProject() to read .dme file path
    - Store project directory and file path
    - Add validation for .dme file existence
    - Update window title with project name
    - _Requirements: 1.1, 1.2, 1.3, 1.4_

  - [x] 3.2 Integrate DMCompiler for project compilation

    - Implement CompileProject() to invoke DMCompiler with DME path
    - Create DMCompilerSettings with appropriate flags
    - Call DMCompiler::Compile() and capture result
    - Extract DMObjectTree from successful compilation
    - Handle compilation errors and display error messages
    - _Requirements: 2.1, 2.2, 2.3, 2.4_

  - [x] 3.3 Implement ObjectTreeCache for persistence
    - Create ObjectTreeCache class with Load() and Save() methods
    - Define cache file format with header (magic, version, timestamp)
    - Implement SerializeObjectTree() to write object tree to binary format
    - Implement DeserializeObjectTree() to read object tree from cache
    - Add cache validation based on DME file timestamp
    - Save cache to .myg_cache.bin after successful compilation
    - Load cache on project open if valid
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 2.5_

- [x] 4. Implement DMI parsing and caching

  - [x] 4.1 Create DMI class for sprite file parsing

    - Use stb_image to load PNG file data
    - Extract PNG tEXt chunk containing DMI metadata
    - Parse metadata format (width, height, states, dirs, frames)
    - Build IconState structures with substate information
    - Calculate sprite positions in sheet (x, y coordinates)
    - _Requirements: 14.1, 14.2, 14.3, 14.4, 5.2_

  - [x] 4.2 Implement OpenGL texture loading for DMI sprites

    - Generate OpenGL texture ID
    - Upload PNG image data to GPU texture
    - Set texture parameters (filtering, wrapping)
    - Store texture ID in DMI object
    - _Requirements: 5.2, 13.2_

  - [x] 4.3 Create DMICache for managing loaded DMI files
    - Implement GetDMI() to load or retrieve cached DMI
    - Build file path from project directory and icon variable
    - Cache loaded DMI objects in unordered_map
    - Handle missing DMI files gracefully with placeholder
    - _Requirements: 5.1, 5.5, 14.5_

- [x] 5. Implement map loading and data structures

  - [x] 5.1 Create Map class with DMM file parsing

    - Implement Load() to read DMM file format
    - Parse tile instance definitions (key = objects)
    - Parse map grid sections ((x,y,z) = {"keys"})
    - Detect TGM vs standard format
    - Extract map bounds (min/max x, y, z)
    - Build tiles* map (Location -> key) and instances* map (key -> TileInstance)
    - _Requirements: 4.1, 4.2, 15.1_

  - [x] 5.2 Implement TileInstance and ObjectInstance classes

    - Create TileInstance with vector of ObjectInstance
    - Implement FromString() to parse object definitions with variables
    - Parse modified types: /path{var="value"; var2=123}
    - Store type_path and vars map in ObjectInstance
    - Implement ToString() for serialization back to DMM format
    - _Requirements: 4.2, 4.4_

  - [x] 5.3 Implement layer sorting for tile rendering

    - Add GetLayerSorted() to TileInstance
    - Sort objects by plane variable (primary)
    - Sort by layer variable (secondary)
    - Sort by type category (turf < obj < mob < area) as tiebreaker
    - Cache sorted result for performance
    - _Requirements: 5.3_

  - [x] 5.4 Create MapManager for multiple open maps
    - Implement LoadMap() to create and load Map objects
    - Store open maps in vector with active map index
    - Implement SetActiveMap() to switch between tabs
    - Add CloseMap() to remove map from list
    - _Requirements: 4.5_

- [x] 6. Implement rendering system

  - [x] 6.1 Create Renderer class with OpenGL setup

    - Compile vertex and fragment shaders for sprite rendering
    - Create VAO, VBO, EBO for quad rendering
    - Set up orthographic projection matrix
    - Implement BeginFrame() and EndFrame() for render loop
    - _Requirements: 13.2_

  - [x] 6.2 Implement Camera class for viewport navigation

    - Add camera position (x, y) and zoom level
    - Implement ScreenToWorld() coordinate conversion
    - Implement WorldToScreen() coordinate conversion
    - Calculate GetVisibleBounds() for frustum culling
    - Clamp zoom between 8 and 128 pixels per tile
    - _Requirements: 6.1, 6.2, 6.4, 6.5_

  - [x] 6.3 Implement tile rendering with sprite compositing

    - Create RenderMap() to iterate visible tiles
    - For each tile, call RenderTile() with TileInstance
    - Get layer-sorted objects from tile
    - For each object, resolve icon and icon_state variables
    - Look up DMI and IconState from cache
    - Calculate sprite source rect from IconSubstate
    - Calculate destination rect from world position and camera
    - Render sprite quad with texture
    - _Requirements: 5.3, 5.4_

  - [x] 6.4 Implement camera controls with mouse input
    - Handle middle mouse button drag to pan camera
    - Update camera position based on mouse delta
    - Handle mouse wheel scroll to zoom camera
    - Zoom centered on mouse cursor position
    - Update viewport rendering each frame
    - _Requirements: 6.1, 6.2_

- [x] 7. Implement GUI system with ImGui

  - [x] 7.1 Create GUIManager class with ImGui integration

    - Initialize ImGui with SDL3 and OpenGL backends
    - Implement BeginFrame() to start ImGui frame
    - Implement EndFrame() to render ImGui draw data
    - Set up ImGui style and fonts
    - _Requirements: 13.2_

  - [x] 7.2 Implement main menu bar

    - Create RenderMainMenuBar() with File, Edit, Project, View, Help menus
    - Add "File > Open Project" menu item
    - Add "File > Open Map" menu item
    - Add "File > Save" menu item
    - Add "Project > Compile" menu item
    - Add "Edit > Undo" and "Edit > Redo" menu items
    - _Requirements: 1.1, 2.1, 4.1, 9.1_

  - [x] 7.3 Implement object tree panel

    - Create RenderObjectTreePanel() to display type hierarchy
    - Render tree starting from root object
    - Implement RenderObjectTreeNode() recursively for children
    - Add expand/collapse functionality for tree nodes
    - Handle node click to select object
    - Handle node double-click to set active placement object
    - _Requirements: 7.1, 7.2, 7.4_

  - [x] 7.4 Implement inspector panel for tile details

    - Create RenderInspectorPanel() to show selected tile
    - Display list of objects on tile with type paths
    - Show variable names and values for each object
    - Add click handler to highlight object in viewport
    - _Requirements: 11.2, 11.3, 11.4_

  - [x] 7.5 Implement status bar

    - Create RenderStatusBar() at bottom of window
    - Display current map file path
    - Display tile coordinates under mouse cursor
    - Display current zoom level
    - Display current Z-level
    - _Requirements: 6.3, 15.4_

  - [x] 7.6 Implement map tabs for multiple open maps
    - Create RenderMapTabs() with ImGui tab bar
    - Add tab for each open map with file name
    - Handle tab selection to switch active map
    - Add close button on each tab
    - Show modified indicator (\*) on unsaved maps
    - _Requirements: 4.5_

- [x] 8. Implement file dialogs and user prompts

  - [x] 8.1 Implement file picker dialogs

    - Create ShowOpenProjectDialog() with .dme file filter
    - Create ShowOpenMapDialog() with .dmm file filter
    - Use platform-specific file dialog (SDL or native)
    - Return selected file path to caller
    - _Requirements: 1.1, 4.1_

  - [x] 8.2 Implement compilation progress dialog

    - Create ShowCompilationDialog() with progress bar
    - Display compilation status message
    - Show progress percentage
    - Allow cancellation (future enhancement)
    - _Requirements: 2.2_

  - [x] 8.3 Implement error dialogs
    - Create ShowErrorDialog() with title and message
    - Display scrollable text for long error messages
    - Add OK button to dismiss
    - Use for compilation errors and file I/O errors
    - _Requirements: 1.3, 2.4, 9.4_

- [ ] 9. Implement map editing functionality

  - [ ] 9.1 Implement object placement on tiles

    - Handle left mouse click in viewport
    - Convert screen coordinates to world tile coordinates
    - Get selected object from object tree panel
    - Create ObjectInstance with selected type path
    - Add object to tile based on type category rules
    - Replace turf if placing turf type
    - Append to object list if placing obj/mob type
    - Update viewport rendering immediately
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5_

  - [ ] 9.2 Implement tile context menu

    - Handle right mouse click in viewport
    - Display ImGui context menu at mouse position
    - Add "Edit Variables" menu item
    - Add "Delete Object" menu item
    - Add "Move to Top" and "Move to Bottom" menu items
    - Store selected tile and object for menu actions
    - _Requirements: 11.1, 11.5_

  - [ ] 9.3 Implement variable editing dialog
    - Create variable editor dialog with ImGui
    - List all variables for selected object
    - Show current values with editable text inputs
    - Validate input based on variable type
    - Update ObjectInstance vars map on confirm
    - Update viewport if visual properties changed (icon, icon_state, dir, etc.)
    - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5_

- [ ] 10. Implement undo/redo system

  - [ ] 10.1 Create UndoableAction base class and concrete actions

    - Define UndoableAction interface with Undo() and Redo() methods
    - Implement PlaceObjectAction for object placement
    - Implement DeleteObjectAction for object deletion
    - Implement ModifyVariableAction for variable edits
    - Store necessary state to reverse each action
    - _Requirements: 10.1_

  - [ ] 10.2 Implement undo/redo stacks in Map class

    - Add undo*stack* and redo*stack* vectors to Map
    - Implement PushUndoState() to add action to undo stack
    - Implement Undo() to pop from undo stack and push to redo stack
    - Implement Redo() to pop from redo stack and push to undo stack
    - Clear redo stack when new action is performed
    - Call Undo()/Redo() methods on actions
    - _Requirements: 10.2, 10.3, 10.5_

  - [ ] 10.3 Wire undo/redo to input and menu
    - Handle Ctrl+Z keyboard shortcut to call Map::Undo()
    - Handle Ctrl+Y keyboard shortcut to call Map::Redo()
    - Add Edit > Undo and Edit > Redo menu items
    - Enable/disable menu items based on stack state
    - Update viewport after undo/redo
    - _Requirements: 10.2, 10.3, 10.4_

- [ ] 11. Implement map saving

  - [ ] 11.1 Implement DMM serialization

    - Create Map::Save() to write DMM file format
    - Serialize tile instances to key definitions
    - Serialize map grid to coordinate sections
    - Preserve original format (standard vs TGM)
    - Preserve key assignments where possible
    - _Requirements: 9.1, 9.2_

  - [ ] 11.2 Implement save workflow

    - Handle Ctrl+S keyboard shortcut
    - Handle File > Save menu item
    - Call Map::Save() with current file path
    - Clear modified flag on successful save
    - Display error dialog on save failure
    - _Requirements: 9.1, 9.3, 9.4_

  - [ ] 11.3 Implement unsaved changes prompt
    - Detect when user closes map tab with unsaved changes
    - Display confirmation dialog with Save/Discard/Cancel options
    - Save map if user chooses Save
    - Close map if user chooses Discard
    - Cancel close if user chooses Cancel
    - _Requirements: 9.5_

- [ ] 12. Implement Z-level support

  - [ ] 12.1 Add Z-level selector to GUI

    - Create Z-level spinner control in status bar or toolbar
    - Set range based on map bounds (min_z to max_z)
    - Handle value change to update current Z-level
    - _Requirements: 15.1, 15.2_

  - [ ] 12.2 Filter rendering and editing by Z-level

    - Pass current Z-level to RenderMap()
    - Only render tiles matching current Z-level
    - Only allow placement on current Z-level
    - Update status bar to show current Z-level
    - _Requirements: 15.2, 15.3, 15.4_

  - [ ] 12.3 Preserve Z-levels in map save
    - Ensure Save() writes all Z-levels to file
    - Group tiles by Z-level in output
    - Maintain Z-level order
    - _Requirements: 15.5_

- [ ] 13. Implement ObjectTreeManager wrapper

  - [ ] 13.1 Create ObjectTreeManager class

    - Implement SetObjectTree() to store DMObjectTree pointer
    - Implement GetRoot() to return root object
    - Implement GetChildren() to return child objects
    - Implement GetObjectByPath() with path lookup
    - _Requirements: 7.1, 7.2_

  - [ ] 13.2 Add caching for performance

    - Cache path lookups in unordered_map
    - Cache children lists for each object
    - Invalidate cache when object tree changes
    - _Requirements: 7.1_

  - [ ] 13.3 Implement variable resolution with inheritance
    - Create GetVariable() to look up variable value
    - Walk up parent chain if variable not found on object
    - Return default value if not found in hierarchy
    - Implement GetAllVariables() to collect all inherited variables
    - _Requirements: 7.3, 12.2_

- [ ] 14. Polish and error handling

  - [ ] 14.1 Add comprehensive error handling

    - Wrap file I/O in try-catch blocks
    - Display user-friendly error messages for common failures
    - Log detailed errors to console for debugging
    - Handle missing DMI files with placeholder rendering
    - Handle invalid object references in maps
    - _Requirements: 1.3, 2.4, 4.4, 5.5, 9.4_

  - [ ] 14.2 Implement loading indicators

    - Show progress dialog during project compilation
    - Show progress dialog during large map loading
    - Display "Loading..." text in viewport during map load
    - _Requirements: 2.2_

  - [ ] 14.3 Add keyboard shortcuts reference
    - Create Help > Keyboard Shortcuts menu item
    - Display dialog with list of all shortcuts
    - Include common operations (save, undo, redo, etc.)
    - _Requirements: N/A (usability enhancement)_

- [ ]\* 15. Testing and validation

  - [ ]\* 15.1 Create unit tests for core functionality

    - Write tests for DMM parser with sample files
    - Write tests for DMI parser with sample files
    - Write tests for ObjectTreeCache serialization
    - Write tests for TileInstance layer sorting
    - Write tests for Camera coordinate conversions
    - _Requirements: All (validation)_

  - [ ]\* 15.2 Create integration tests

    - Test full workflow: load project -> compile -> load map -> edit -> save
    - Test undo/redo with multiple operations
    - Test multi-map tabs
    - _Requirements: All (validation)_

  - [ ] 15.3 Perform manual testing
    - Test with real BYOND projects
    - Test with large maps (200x200+)
    - Test with missing/corrupted files
    - Test on all target platforms (Windows, Linux, macOS)
    - _Requirements: All (validation)_

- [ ]\* 16. Documentation and deployment

  - [ ]\* 16.1 Write user documentation

    - Create README.md with feature overview
    - Document installation instructions
    - Document basic usage workflow
    - Add screenshots of UI
    - _Requirements: N/A (documentation)_

  - [ ]\* 16.2 Write developer documentation

    - Document build instructions for each platform
    - Document architecture and design decisions
    - Add code comments for complex algorithms
    - Create CONTRIBUTING.md for contributors
    - _Requirements: N/A (documentation)_

  - [ ]\* 16.3 Set up CI/CD pipeline

    - Configure GitHub Actions for automated builds
    - Build on Windows, Linux, and macOS
    - Run tests on each platform
    - Create release artifacts
    - _Requirements: 13.3_

  - [ ] 16.4 Create release packages
    - Bundle SDL3 libraries with executable
    - Create installers for Windows (NSIS)
    - Create AppImage for Linux
    - _Requirements: N/A (deployment)_
