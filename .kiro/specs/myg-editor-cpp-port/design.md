# MYG Editor Design Document

## Overview

MYG Editor is a native C++ map editor for BYOND projects, designed to replace the Java-based FastDMM editor with improved performance and tighter integration with the DMCompiler toolchain. The editor provides a complete visual editing environment for .dmm map files, with full support for object tree browsing, sprite rendering, and project compilation.

The architecture follows a modular design with clear separation between:

- **Core Engine**: DMCompiler integration, object tree management, and data models
- **Rendering System**: SDL3-based viewport with OpenGL rendering for sprites and tiles
- **GUI Layer**: ImGui-based user interface for panels, menus, and dialogs
- **File I/O**: DMI parsing, DMM serialization, and cache management

## Architecture

### High-Level Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      MYG Editor Application                  │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   GUI Layer  │  │   Rendering  │  │  Input       │      │
│  │   (ImGui)    │  │   (SDL3/GL)  │  │  Handler     │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
│         │                  │                  │               │
│         └──────────────────┼──────────────────┘               │
│                            │                                  │
│  ┌─────────────────────────┴────────────────────────┐        │
│  │           Application Core / State Manager       │        │
│  └─────────────────────────┬────────────────────────┘        │
│                            │                                  │
│  ┌────────────┬────────────┼────────────┬──────────┐        │
│  │  Project   │   Map      │  Object    │  DMI     │        │
│  │  Manager   │   Manager  │  Tree      │  Cache   │        │
│  └─────┬──────┘  └────┬────┘  └────┬───┘  └───┬───┘        │
│        │              │            │          │              │
└────────┼──────────────┼────────────┼──────────┼──────────────┘
         │              │            │          │
    ┌────┴────┐    ┌────┴────┐  ┌───┴───┐  ┌───┴───┐
    │ DME     │    │ DMM     │  │ Cache │  │ PNG   │
    │ Files   │    │ Files   │  │ Files │  │ Files │
    └─────────┘    └─────────┘  └───────┘  └───────┘
         │
    ┌────┴────────────────┐
    │  DMCompilerCpp Lib  │
    │  (External)         │
    └─────────────────────┘
```

### Technology Stack

- **Windowing & Input**: SDL3 (https://github.com/libsdl-org/SDL)
- **GUI Framework**: Dear ImGui (https://github.com/ocornut/imgui)
- **Graphics API**: OpenGL 3.3+ (via SDL3)
- **Image Loading**: stb_image.h for PNG decoding
- **Build System**: CMake 3.15+
- **Compiler Integration**: DMCompilerCpp library (existing)
- **Serialization**: JSON for cache files (nlohmann/json)

## Components and Interfaces

### 1. Application Core

**Purpose**: Main application lifecycle, window management, and state coordination

**Key Classes**:

```cpp
class MYGEditor {
public:
    MYGEditor();
    ~MYGEditor();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;

    std::unique_ptr<ProjectManager> project_manager_;
    std::unique_ptr<MapManager> map_manager_;
    std::unique_ptr<ObjectTreeManager> object_tree_manager_;
    std::unique_ptr<DMICache> dmi_cache_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<GUIManager> gui_manager_;
    std::unique_ptr<InputHandler> input_handler_;

    bool running_;

    void ProcessEvents();
    void Update(float delta_time);
    void Render();
};
```

**Responsibilities**:

- Initialize SDL3, OpenGL, and ImGui
- Create and manage the main window
- Run the main event loop
- Coordinate between subsystems
- Handle application-level events (quit, minimize, etc.)

### 2. Project Manager

**Purpose**: Manage DME project loading, compilation, and cache persistence

**Key Classes**:

```cpp
class ProjectManager {
public:
    bool LoadProject(const std::string& dme_path);
    bool CompileProject(CompilationCallback callback);
    bool HasValidObjectTree() const;

    const std::string& GetProjectPath() const;
    const std::string& GetProjectDirectory() const;
    DMCompiler::DMObjectTree* GetObjectTree();

private:
    std::string dme_path_;
    std::string project_directory_;
    std::unique_ptr<DMCompiler::DMCompiler> compiler_;
    std::unique_ptr<ObjectTreeCache> cache_;
    bool object_tree_loaded_;

    bool LoadCachedObjectTree();
    bool SaveObjectTreeCache();
    bool IsCacheValid() const;
};

class ObjectTreeCache {
public:
    bool Load(const std::string& cache_path, DMCompiler::DMObjectTree* tree);
    bool Save(const std::string& cache_path, const DMCompiler::DMObjectTree* tree);

private:
    struct CacheHeader {
        uint32_t magic;           // "MYGC"
        uint32_t version;         // Cache format version
        uint64_t dme_timestamp;   // DME file modification time
        uint32_t object_count;
        uint32_t string_count;
    };

    bool ValidateHeader(const CacheHeader& header, uint64_t current_timestamp);
    void SerializeObjectTree(std::ostream& out, const DMCompiler::DMObjectTree* tree);
    void DeserializeObjectTree(std::istream& in, DMCompiler::DMObjectTree* tree);
};
```

**Responsibilities**:

- Load .dme project files
- Invoke DMCompiler for compilation
- Manage object tree lifecycle
- Cache compiled object trees to disk
- Validate cache freshness against source files

**Cache File Format** (`.myg_cache.bin`):

```
Header (32 bytes):
  - Magic: "MYGC" (4 bytes)
  - Version: uint32 (4 bytes)
  - DME Timestamp: uint64 (8 bytes)
  - Object Count: uint32 (4 bytes)
  - String Table Size: uint32 (4 bytes)
  - Reserved: (8 bytes)

String Table:
  - Count: uint32
  - For each string:
    - Length: uint32
    - Data: char[length]

Object Tree:
  - For each object:
    - Path: string_id (uint32)
    - Parent ID: int32
    - Variable Count: uint32
    - For each variable:
      - Name: string_id
      - Value: string_id
      - Type: uint8
```

### 3. Map Manager

**Purpose**: Load, edit, and save DMM map files

**Key Classes**:

```cpp
class MapManager {
public:
    bool LoadMap(const std::string& dmm_path, DMCompiler::DMObjectTree* tree);
    bool SaveMap(const std::string& dmm_path);
    void CloseMap();

    Map* GetCurrentMap();
    const std::vector<std::unique_ptr<Map>>& GetOpenMaps() const;
    void SetActiveMap(int index);

private:
    std::vector<std::unique_ptr<Map>> open_maps_;
    int active_map_index_;
};

class Map {
public:
    struct Bounds {
        int min_x, min_y, min_z;
        int max_x, max_y, max_z;
    };

    bool Load(const std::string& path, DMCompiler::DMObjectTree* tree);
    bool Save(const std::string& path);

    TileInstance* GetTile(int x, int y, int z);
    void SetTile(int x, int y, int z, TileInstance* tile);

    const Bounds& GetBounds() const;
    const std::string& GetFilePath() const;
    bool IsModified() const;

    // Undo/Redo
    void PushUndoState(std::unique_ptr<UndoableAction> action);
    bool Undo();
    bool Redo();

private:
    std::string file_path_;
    Bounds bounds_;
    bool is_tgm_format_;
    int key_length_;

    // Map data: location -> tile key
    std::unordered_map<Location, std::string, LocationHash> tiles_;

    // Tile instances: key -> tile data
    std::unordered_map<std::string, std::unique_ptr<TileInstance>> instances_;

    // Available keys for new instances
    std::vector<std::string> unused_keys_;

    // Undo/Redo stacks
    std::vector<std::unique_ptr<UndoableAction>> undo_stack_;
    std::vector<std::unique_ptr<UndoableAction>> redo_stack_;

    bool modified_;
    DMCompiler::DMObjectTree* object_tree_;

    std::string AllocateKey();
    void ReleaseKey(const std::string& key);
    void GenerateKeys(int length);
};

struct Location {
    int x, y, z;

    bool operator==(const Location& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct LocationHash {
    size_t operator()(const Location& loc) const {
        return std::hash<int>()(loc.x) ^
               (std::hash<int>()(loc.y) << 1) ^
               (std::hash<int>()(loc.z) << 2);
    }
};

class TileInstance {
public:
    std::vector<ObjectInstance> objects;

    std::string ToString() const;
    std::string ToTGMString() const;

    static std::unique_ptr<TileInstance> FromString(
        const std::string& str,
        DMCompiler::DMObjectTree* tree
    );

    std::vector<ObjectInstance*> GetLayerSorted();
    ObjectInstance* GetArea();

private:
    mutable std::vector<ObjectInstance*> cached_sorted_;
    mutable ObjectInstance* cached_area_;
};

class ObjectInstance {
public:
    std::string type_path;
    std::unordered_map<std::string, std::string> vars;

    std::string GetVar(const std::string& name, DMCompiler::DMObjectTree* tree) const;
    void SetVar(const std::string& name, const std::string& value);
    bool IsType(const std::string& path) const;

    std::string ToString() const;
    std::string ToTGMString() const;
};
```

**Responsibilities**:

- Parse DMM file format (both standard and TGM)
- Manage multiple open maps with tabs
- Handle tile instance creation and modification
- Implement undo/redo system
- Serialize maps back to DMM format

**DMM File Format**:

```
// Standard format:
"aaa" = (/turf/floor,/obj/item)
"aab" = (/turf/wall)

(1,1,1) = {"
aaa
aab
"}

// TGM format:
"aaa" = (
/turf/floor,
/obj/item)
"aab" = (
/turf/wall)

(1,1,1) = {"
aaa
aab
"}
```

### 4. Object Tree Manager

**Purpose**: Provide UI-friendly access to the DMObjectTree

**Key Classes**:

```cpp
class ObjectTreeManager {
public:
    void SetObjectTree(DMCompiler::DMObjectTree* tree);

    // Tree navigation
    DMCompiler::DMObject* GetRoot();
    std::vector<DMCompiler::DMObject*> GetChildren(DMCompiler::DMObject* obj);
    DMCompiler::DMObject* GetObjectByPath(const std::string& path);

    // Search and filtering
    std::vector<DMCompiler::DMObject*> Search(const std::string& query);
    std::vector<DMCompiler::DMObject*> FilterByType(const std::string& base_type);

    // Variable access
    std::string GetVariable(DMCompiler::DMObject* obj, const std::string& var_name);
    std::vector<std::pair<std::string, std::string>> GetAllVariables(
        DMCompiler::DMObject* obj
    );

private:
    DMCompiler::DMObjectTree* tree_;

    // Cache for performance
    std::unordered_map<std::string, DMCompiler::DMObject*> path_cache_;
    std::unordered_map<DMCompiler::DMObject*, std::vector<DMCompiler::DMObject*>> children_cache_;
};
```

**Responsibilities**:

- Wrap DMObjectTree for UI consumption
- Provide efficient tree traversal
- Cache frequently accessed data
- Handle variable inheritance resolution

### 5. DMI Cache and Rendering

**Purpose**: Load, parse, and cache DMI sprite files

**Key Classes**:

```cpp
class DMICache {
public:
    DMICache(const std::string& project_dir);

    DMI* GetDMI(const std::string& icon_path);
    void Clear();

private:
    std::string project_directory_;
    std::unordered_map<std::string, std::unique_ptr<DMI>> cache_;

    std::unique_ptr<DMI> LoadDMI(const std::string& path);
};

class DMI {
public:
    struct IconState {
        std::string name;
        int dir_count;      // 1, 4, or 8
        int frame_count;
        float delay;        // Animation delay per frame
        bool loop;
        bool rewind;
        bool movement;

        std::vector<IconSubstate> substates;
    };

    struct IconSubstate {
        int dir;            // NORTH=1, SOUTH=2, EAST=4, WEST=8
        int frame;
        int x, y;           // Position in sprite sheet
    };

    bool Load(const std::string& path);

    IconState* GetState(const std::string& state_name);
    IconState* GetDefaultState();

    GLuint GetTextureID() const { return texture_id_; }
    int GetIconWidth() const { return icon_width_; }
    int GetIconHeight() const { return icon_height_; }

private:
    int icon_width_;
    int icon_height_;
    int sheet_width_;
    int sheet_height_;

    GLuint texture_id_;
    std::unordered_map<std::string, IconState> states_;
    IconState* default_state_;

    bool ParseMetadata(const std::string& description);
    bool LoadTexture(const unsigned char* image_data, int width, int height);
};
```

**DMI Metadata Format** (PNG tEXt chunk "Description"):

```
# DMI 4.0
width = 32
height = 32
state = "default"
	dirs = 1
	frames = 1
state = "open"
	dirs = 4
	frames = 2
	delay = 5.0
```

**Responsibilities**:

- Parse PNG files with stb_image
- Extract DMI metadata from PNG text chunks
- Upload textures to OpenGL
- Cache loaded DMIs in memory
- Provide sprite lookup by state name

### 6. Renderer

**Purpose**: Render the map viewport with OpenGL

**Key Classes**:

```cpp
class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize();
    void SetViewport(int width, int height);

    void BeginFrame();
    void EndFrame();

    void RenderMap(Map* map, int z_level, const Camera& camera);
    void RenderTile(TileInstance* tile, int x, int y, const Camera& camera,
                    DMICache* dmi_cache, DMCompiler::DMObjectTree* tree);

private:
    GLuint shader_program_;
    GLuint vao_, vbo_, ebo_;

    int viewport_width_;
    int viewport_height_;

    void CompileShaders();
    void SetupBuffers();
    void RenderSprite(GLuint texture, const Rect& src, const Rect& dst,
                     float alpha, const Color& color);
};

struct Camera {
    float x, y;             // World position
    float zoom;             // Pixels per tile
    int viewport_width;
    int viewport_height;

    // Convert screen to world coordinates
    void ScreenToWorld(int screen_x, int screen_y, int& world_x, int& world_y) const;

    // Convert world to screen coordinates
    void WorldToScreen(int world_x, int world_y, int& screen_x, int& screen_y) const;

    // Get visible tile bounds
    void GetVisibleBounds(int& min_x, int& min_y, int& max_x, int& max_y) const;
};
```

**Rendering Pipeline**:

1. Clear framebuffer
2. Calculate visible tile bounds from camera
3. For each visible tile:
   - Get TileInstance from map
   - Sort objects by plane/layer
   - For each object:
     - Resolve icon and icon_state variables
     - Get DMI and IconState
     - Render sprite quad with texture
4. Render UI overlay (grid, selection, etc.)

**Shader Design**:

```glsl
// Vertex Shader
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}

// Fragment Shader
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;
uniform vec4 colorMod;
uniform float alphaMod;

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = texColor * colorMod * vec4(1.0, 1.0, 1.0, alphaMod);
}
```

### 7. GUI Manager

**Purpose**: Manage ImGui-based user interface

**Key Classes**:

```cpp
class GUIManager {
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(SDL_Window* window, SDL_GLContext gl_context);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderMainMenuBar();
    void RenderObjectTreePanel(ObjectTreeManager* tree_mgr);
    void RenderMapTabs(MapManager* map_mgr);
    void RenderInspectorPanel(TileInstance* selected_tile);
    void RenderStatusBar(const std::string& status);

    // Dialogs
    bool ShowOpenProjectDialog(std::string& out_path);
    bool ShowOpenMapDialog(std::string& out_path);
    bool ShowCompilationDialog(const std::string& message, float progress);
    bool ShowErrorDialog(const std::string& title, const std::string& message);

private:
    bool show_object_tree_;
    bool show_inspector_;
    bool show_demo_window_;

    std::string selected_object_path_;
    int selected_z_level_;

    void RenderObjectTreeNode(DMCompiler::DMObject* obj, ObjectTreeManager* tree_mgr);
};
```

**UI Layout**:

```
┌────────────────────────────────────────────────────────────┐
│ File  Edit  Project  View  Help                           │
├──────────┬─────────────────────────────────────┬───────────┤
│          │                                     │           │
│ Object   │                                     │ Inspector │
│ Tree     │         Map Viewport                │           │
│          │                                     │ Selected  │
│ /datum   │                                     │ Tile:     │
│  /atom   │                                     │  /turf/   │
│   /turf  │                                     │   floor   │
│   /obj   │                                     │           │
│   /mob   │                                     │ Variables:│
│  /area   │                                     │  icon=... │
│          │                                     │  dir=2    │
│          │                                     │           │
├──────────┴─────────────────────────────────────┴───────────┤
│ Status: Map loaded | Tile: (15, 23, 1) | Zoom: 32px       │
└────────────────────────────────────────────────────────────┘
```

**Responsibilities**:

- Render all UI panels and windows
- Handle UI events and callbacks
- Manage dialog state
- Provide file picker dialogs
- Display compilation progress

### 8. Input Handler

**Purpose**: Process user input and translate to editor actions

**Key Classes**:

```cpp
class InputHandler {
public:
    void ProcessEvent(const SDL_Event& event, MYGEditor* editor);
    void Update(MYGEditor* editor);

    bool IsKeyPressed(SDL_Keycode key) const;
    bool IsMouseButtonPressed(int button) const;

    void GetMousePosition(int& x, int& y) const;
    void GetMouseDelta(int& dx, int& dy) const;

private:
    std::unordered_map<SDL_Keycode, bool> key_states_;
    std::unordered_map<int, bool> mouse_button_states_;

    int mouse_x_, mouse_y_;
    int mouse_dx_, mouse_dy_;
    int mouse_wheel_delta_;

    bool middle_mouse_dragging_;
    int drag_start_x_, drag_start_y_;

    void HandleKeyPress(SDL_Keycode key, MYGEditor* editor);
    void HandleMouseButton(int button, bool pressed, MYGEditor* editor);
    void HandleMouseMotion(int x, int y, MYGEditor* editor);
    void HandleMouseWheel(int delta, MYGEditor* editor);
};
```

**Input Mapping**:

- **Middle Mouse Drag**: Pan camera
- **Mouse Wheel**: Zoom camera
- **Left Click**: Place selected object
- **Right Click**: Open tile context menu
- **Ctrl+Z**: Undo
- **Ctrl+Y**: Redo
- **Ctrl+S**: Save map
- **Ctrl+O**: Open map
- **Delete**: Delete selected object

## Data Models

### Object Tree Structure

The DMObjectTree from DMCompilerCpp provides:

```cpp
class DMObject {
    int Id;
    std::string Path;
    DMObject* Parent;
    std::vector<DMObject*> Children;
    std::unordered_map<std::string, DMVariable> Variables;
    std::vector<DMProc*> Procs;
};
```

### Map Data Structure

```cpp
// In-memory representation
Map {
    tiles: HashMap<Location, String>           // Location -> tile key
    instances: HashMap<String, TileInstance>   // Key -> tile data
    unused_keys: Vec<String>
}

TileInstance {
    objects: Vec<ObjectInstance>
}

ObjectInstance {
    type_path: String
    vars: HashMap<String, String>
}
```

### Cache Data Structure

```cpp
ObjectTreeCache {
    header: CacheHeader
    string_table: Vec<String>
    objects: Vec<CachedObject>
}

CachedObject {
    path_id: u32
    parent_id: i32
    variables: Vec<(u32, u32)>  // (name_id, value_id)
}
```

## Error Handling

### Compilation Errors

When DMCompiler fails:

1. Capture error messages from DMCompiler
2. Display in a scrollable error dialog
3. Highlight error count in status bar
4. Prevent map operations until successful compilation

### File I/O Errors

- **DME not found**: Show error dialog, return to project selection
- **DMM parse error**: Show error with line number, load partial map if possible
- **DMI not found**: Log warning, render placeholder sprite
- **Cache corruption**: Delete cache, force recompilation

### Runtime Errors

- Use exceptions for critical errors (out of memory, OpenGL failures)
- Use return codes for recoverable errors (file not found, parse errors)
- Log all errors to console and optional log file

## Testing Strategy

### Unit Tests

**Test Framework**: Google Test (gtest)

**Test Coverage**:

1. **DMM Parser**:

   - Parse standard format
   - Parse TGM format
   - Handle malformed input
   - Preserve key assignments

2. **DMI Parser**:

   - Parse metadata
   - Extract icon states
   - Handle directional states
   - Handle animated states

3. **Object Tree Cache**:

   - Serialize and deserialize
   - Validate cache freshness
   - Handle version mismatches

4. **Tile Instance**:

   - Layer sorting
   - Variable inheritance
   - ToString/FromString round-trip

5. **Camera**:
   - Screen to world conversion
   - World to screen conversion
   - Visible bounds calculation

### Integration Tests

1. **Project Loading**:

   - Load real DME project
   - Compile with DMCompiler
   - Verify object tree structure

2. **Map Loading**:

   - Load real DMM files
   - Verify tile count
   - Verify object instances

3. **Rendering**:
   - Load DMI files
   - Render test map
   - Verify sprite positions

### Manual Testing

1. **UI Workflow**:

   - Open project → Compile → Open map → Edit → Save
   - Undo/Redo operations
   - Multi-map tabs

2. **Performance**:

   - Large maps (200x200+)
   - Many open maps
   - Rapid zoom/pan

3. **Edge Cases**:
   - Missing DMI files
   - Corrupted DMM files
   - Invalid object references

## Performance Considerations

### Optimization Strategies

1. **Rendering**:

   - Only render visible tiles (frustum culling)
   - Batch sprite draws by texture
   - Use texture atlases for small icons
   - Cache layer-sorted object lists

2. **Memory**:

   - Lazy-load DMI files
   - Unload unused DMIs after timeout
   - Use string interning for paths and keys
   - Share TileInstance objects across locations

3. **Compilation**:

   - Cache object tree to disk
   - Incremental compilation (future)
   - Background compilation thread

4. **UI**:
   - Virtualize large tree views
   - Debounce search input
   - Lazy-render off-screen panels

### Profiling Points

- Frame time breakdown (render, UI, logic)
- DMI load times
- Map parse times
- Memory usage per open map
- Cache save/load times

## Build System

### CMakeLists.txt Structure

```cmake
cmake_minimum_required(VERSION 3.15)
project(MYGEditor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

# ImGui (as subdirectory or FetchContent)
add_subdirectory(external/imgui)

# stb_image (header-only)
include_directories(external/stb)

# DMCompilerCpp
add_subdirectory(../DMCompilerCpp DMCompilerCpp)

# MYG Editor executable
add_executable(myg_editor
    src/main.cpp
    src/MYGEditor.cpp
    src/ProjectManager.cpp
    src/MapManager.cpp
    src/ObjectTreeManager.cpp
    src/DMICache.cpp
    src/Renderer.cpp
    src/GUIManager.cpp
    src/InputHandler.cpp
    # ... more sources
)

target_link_libraries(myg_editor
    SDL3::SDL3
    OpenGL::GL
    imgui
    DMCompilerCpp
)

# Tests
enable_testing()
add_subdirectory(tests)
```

### Directory Structure

```
MYGEditor/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── MYGEditor.{h,cpp}
│   ├── ProjectManager.{h,cpp}
│   ├── MapManager.{h,cpp}
│   ├── ObjectTreeManager.{h,cpp}
│   ├── DMICache.{h,cpp}
│   ├── Renderer.{h,cpp}
│   ├── GUIManager.{h,cpp}
│   ├── InputHandler.{h,cpp}
│   └── ...
├── include/
│   └── myg/
│       └── *.h
├── external/
│   ├── imgui/
│   └── stb/
│       └── stb_image.h
├── tests/
│   ├── CMakeLists.txt
│   ├── test_dmm_parser.cpp
│   ├── test_dmi_parser.cpp
│   └── testdata/
│       ├── test.dme
│       ├── test.dmm
│       └── test.dmi
└── build/
```

## Deployment

### Platform-Specific Considerations

**Windows**:

- Bundle SDL3.dll with executable
- Use MSVC runtime (static or dynamic)
- Provide installer (NSIS or WiX)

**Linux**:

- Depend on system SDL3 package
- Provide AppImage or Flatpak
- Desktop entry file for menu integration

**macOS**:

- Create .app bundle
- Include SDL3 framework
- Code signing for distribution

### Distribution

- GitHub Releases with pre-built binaries
- Include DMCompilerCpp as git submodule
- Provide build instructions for each platform
- Version numbering: MAJOR.MINOR.PATCH
