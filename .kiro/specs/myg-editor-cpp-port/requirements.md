# Requirements Document

## Introduction

This document specifies the requirements for MYG Editor, a C++ port of the FastDMM map editor with integrated DMCompiler support. MYG Editor will provide a native C++ application for editing BYOND map files (.dmm) with full compilation capabilities, object tree generation, and DMI sprite rendering. The editor will leverage the existing DMCompilerCpp library for parsing and compiling DM projects, while providing a modern GUI for visual map editing.

## Glossary

- **MYG Editor**: The new C++ map editor application being developed
- **FastDMM**: The original Java-based map editor being ported
- **DMCompiler**: The C++ compiler library in DMCompilerCpp/ that compiles DM code
- **Object Tree**: The hierarchical type system representing all DM objects (/datum, /atom, /turf, etc.)
- **DMI File**: Dream Maker Icon file containing sprites and icon states
- **DMM File**: Dream Maker Map file containing tile placement data
- **DME File**: Dream Maker Environment file (project root file)
- **Tile Instance**: A specific placement of an object on the map with coordinates and properties
- **Icon State**: A named sprite state within a DMI file (e.g., "open", "closed")
- **Viewport**: The visible rendering area showing the map
- **Compilation Session**: The process of compiling a DME project and generating the object tree
- **Persistence**: Saving compiled object tree data to disk for reuse across editor sessions

## Requirements

### Requirement 1

**User Story:** As a map editor user, I want to open a DME project file, so that I can load the project structure and prepare for map editing

#### Acceptance Criteria

1. WHEN the user selects "File > Open Project" from the menu, THE MYG Editor SHALL display a file dialog filtered to show only .dme files
2. WHEN the user selects a valid .dme file, THE MYG Editor SHALL load the project path and display the project name in the window title
3. IF the .dme file cannot be read, THEN THE MYG Editor SHALL display an error dialog with the specific file access error
4. WHEN a project is successfully loaded, THE MYG Editor SHALL enable the "Compile Project" menu option
5. WHEN a project is loaded, THE MYG Editor SHALL check for a cached compiled object tree file in the project directory

### Requirement 2

**User Story:** As a map editor user, I want to compile the loaded DME project, so that the object tree is generated and available for map editing

#### Acceptance Criteria

1. WHEN the user selects "Project > Compile" from the menu, THE MYG Editor SHALL invoke DMCompiler with the loaded .dme file path
2. WHILE compilation is in progress, THE MYG Editor SHALL display a progress dialog showing compilation status
3. WHEN compilation completes successfully, THE MYG Editor SHALL extract the object tree from DMCompiler and populate the internal type system
4. IF compilation fails with errors, THEN THE MYG Editor SHALL display a compilation error dialog listing all errors and warnings
5. WHEN compilation succeeds, THE MYG Editor SHALL serialize the object tree to a cache file in the project directory for future sessions
6. WHEN the object tree is generated, THE MYG Editor SHALL enable map file opening functionality

### Requirement 3

**User Story:** As a map editor user, I want the compiled object tree to persist between editor sessions, so that I don't need to recompile every time I open the editor

#### Acceptance Criteria

1. WHEN a project is compiled successfully, THE MYG Editor SHALL save the object tree data to a file named ".myg_cache.bin" in the project directory
2. WHEN a project is loaded, THE MYG Editor SHALL check if ".myg_cache.bin" exists and has a newer timestamp than the .dme file
3. IF a valid cache file exists, THEN THE MYG Editor SHALL load the object tree from the cache file instead of requiring compilation
4. WHEN loading from cache, THE MYG Editor SHALL validate the cache file format and version
5. IF the cache file is invalid or corrupted, THEN THE MYG Editor SHALL delete the cache file and require a fresh compilation

### Requirement 4

**User Story:** As a map editor user, I want to open DMM map files from the compiled project, so that I can view and edit the map layout

#### Acceptance Criteria

1. WHEN the user selects "File > Open Map" and an object tree is loaded, THE MYG Editor SHALL display a file dialog filtered to show only .dmm files
2. WHEN the user selects a valid .dmm file, THE MYG Editor SHALL parse the map file format and load all tile instances
3. WHEN a map is loaded, THE MYG Editor SHALL display the map in the viewport with proper tile rendering
4. IF the map file references objects not in the object tree, THEN THE MYG Editor SHALL log warnings but continue loading valid tiles
5. WHEN multiple maps are opened, THE MYG Editor SHALL display them in separate tabs within the editor

### Requirement 5

**User Story:** As a map editor user, I want to see sprites rendered from DMI files for each object on the map, so that I can visually identify tiles

#### Acceptance Criteria

1. WHEN an object in the object tree has an "icon" variable, THE MYG Editor SHALL locate and load the corresponding DMI file from the project resources
2. WHEN a DMI file is loaded, THE MYG Editor SHALL parse the PNG image data and extract all icon states
3. WHEN rendering a tile, THE MYG Editor SHALL composite all objects on that tile (turf, area, objects, mobs) in the correct layer order
4. WHEN an object has an "icon_state" variable, THE MYG Editor SHALL render the specific icon state from the DMI file
5. IF a DMI file cannot be found or loaded, THEN THE MYG Editor SHALL render a placeholder sprite for that object

### Requirement 6

**User Story:** As a map editor user, I want to navigate the map viewport with mouse controls, so that I can view different areas of the map

#### Acceptance Criteria

1. WHEN the user drags with the middle mouse button in the viewport, THE MYG Editor SHALL pan the camera to follow the mouse movement
2. WHEN the user scrolls the mouse wheel in the viewport, THE MYG Editor SHALL zoom the camera in or out centered on the mouse position
3. WHEN the viewport is rendered, THE MYG Editor SHALL display coordinates of the tile under the mouse cursor in the status bar
4. THE MYG Editor SHALL maintain a minimum zoom level of 8 pixels per tile
5. THE MYG Editor SHALL maintain a maximum zoom level of 128 pixels per tile

### Requirement 7

**User Story:** As a map editor user, I want to browse the object tree hierarchy in a panel, so that I can select objects to place on the map

#### Acceptance Criteria

1. WHEN an object tree is loaded, THE MYG Editor SHALL display a tree view panel showing the complete type hierarchy starting from root
2. WHEN the user expands a node in the tree view, THE MYG Editor SHALL show all child types of that object
3. WHEN the user clicks on an object in the tree view, THE MYG Editor SHALL display that object's variables in a properties panel
4. WHEN the user double-clicks an object in the tree view, THE MYG Editor SHALL set that object as the active placement object
5. THE MYG Editor SHALL display object icons next to each tree node when available

### Requirement 8

**User Story:** As a map editor user, I want to place objects on the map by clicking tiles, so that I can build and modify the map layout

#### Acceptance Criteria

1. WHEN an object is selected from the object tree and the user clicks a tile in the viewport, THE MYG Editor SHALL add that object to the clicked tile
2. WHEN placing an object, THE MYG Editor SHALL respect the object's type category (turf, area, obj, mob) and placement rules
3. WHEN placing a turf, THE MYG Editor SHALL replace the existing turf on that tile
4. WHEN placing an obj or mob, THE MYG Editor SHALL add it to the tile's object list without replacing existing objects
5. WHEN an object is placed, THE MYG Editor SHALL immediately update the viewport rendering to show the change

### Requirement 9

**User Story:** As a map editor user, I want to save modified maps back to DMM files, so that my changes are persisted

#### Acceptance Criteria

1. WHEN the user selects "File > Save" for a modified map, THE MYG Editor SHALL serialize the map data to the DMM file format
2. WHEN saving, THE MYG Editor SHALL preserve the original DMM format structure and key assignments
3. WHEN a map is saved successfully, THE MYG Editor SHALL clear the modified flag for that map tab
4. IF the file cannot be written, THEN THE MYG Editor SHALL display an error dialog with the specific file write error
5. WHEN the user closes a modified map without saving, THE MYG Editor SHALL prompt with a confirmation dialog

### Requirement 10

**User Story:** As a map editor user, I want undo and redo functionality, so that I can revert mistakes and restore changes

#### Acceptance Criteria

1. WHEN the user performs a map modification action, THE MYG Editor SHALL add that action to the undo stack
2. WHEN the user selects "Edit > Undo" or presses Ctrl+Z, THE MYG Editor SHALL revert the last action and update the viewport
3. WHEN the user selects "Edit > Redo" or presses Ctrl+Y, THE MYG Editor SHALL restore the last undone action
4. THE MYG Editor SHALL maintain separate undo stacks for each open map tab
5. WHEN a new action is performed after undo operations, THE MYG Editor SHALL clear the redo stack

### Requirement 11

**User Story:** As a map editor user, I want to select and inspect tiles on the map, so that I can see what objects are placed at specific locations

#### Acceptance Criteria

1. WHEN the user right-clicks a tile in the viewport, THE MYG Editor SHALL display a context menu with tile operations
2. WHEN a tile is selected, THE MYG Editor SHALL display all objects on that tile in an inspector panel
3. WHEN viewing the inspector panel, THE MYG Editor SHALL show each object's type path and variable values
4. WHEN the user clicks an object in the inspector panel, THE MYG Editor SHALL highlight that object in the viewport
5. WHEN the user selects "Delete" from the context menu, THE MYG Editor SHALL remove the selected object from the tile

### Requirement 12

**User Story:** As a map editor user, I want to edit object variables on placed instances, so that I can customize object properties without changing the type definition

#### Acceptance Criteria

1. WHEN the user selects "Edit Variables" from a tile context menu, THE MYG Editor SHALL display a variable editor dialog
2. WHEN the variable editor is shown, THE MYG Editor SHALL list all variables for the selected object with current values
3. WHEN the user modifies a variable value, THE MYG Editor SHALL validate the input against the variable's type
4. WHEN the user confirms variable changes, THE MYG Editor SHALL update the tile instance with the modified variables
5. WHEN variables are modified, THE MYG Editor SHALL update the viewport rendering if visual properties changed

### Requirement 13

**User Story:** As a developer, I want the MYG Editor to use a modern C++ GUI framework, so that the application is maintainable and cross-platform

#### Acceptance Criteria

1. THE MYG Editor SHALL use SDL3 (https://github.com/libsdl-org/SDL) for windowing, input handling, and rendering
2. THE MYG Editor SHALL use a GUI library compatible with SDL for UI elements (ImGui, Dear ImGui, or similar)
3. THE MYG Editor SHALL compile on Windows with MSVC, Linux with GCC, and macOS with Clang
4. THE MYG Editor SHALL use CMake as the build system
5. THE MYG Editor SHALL link against the DMCompilerCpp library for compilation functionality

### Requirement 14

**User Story:** As a developer, I want the MYG Editor to properly parse DMI files, so that sprites are correctly extracted and rendered

#### Acceptance Criteria

1. THE MYG Editor SHALL use a PNG decoding library (libpng or stb_image) to read DMI file image data
2. WHEN parsing a DMI file, THE MYG Editor SHALL extract metadata from PNG text chunks to identify icon states
3. WHEN extracting icon states, THE MYG Editor SHALL correctly handle directional states (NORTH, SOUTH, EAST, WEST)
4. WHEN extracting icon states, THE MYG Editor SHALL correctly handle animated states with multiple frames
5. THE MYG Editor SHALL cache loaded DMI files in memory to avoid repeated file I/O

### Requirement 15

**User Story:** As a map editor user, I want the editor to handle Z-levels, so that I can edit multi-level maps

#### Acceptance Criteria

1. WHEN a map with multiple Z-levels is loaded, THE MYG Editor SHALL display a Z-level selector control
2. WHEN the user changes the Z-level selector, THE MYG Editor SHALL update the viewport to show the selected level
3. WHEN placing objects, THE MYG Editor SHALL place them on the currently selected Z-level
4. THE MYG Editor SHALL display the current Z-level number in the status bar
5. WHEN saving a map, THE MYG Editor SHALL preserve all Z-levels in the output file
