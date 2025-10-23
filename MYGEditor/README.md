# MYG Editor

A native C++ map editor for BYOND projects with integrated DMCompiler support.

## Overview

MYG Editor is a modern, cross-platform map editor for BYOND that provides:

- Visual editing of .dmm map files
- Integrated compilation with DMCompilerCpp
- Object tree browsing and inspection
- DMI sprite rendering with OpenGL
- Undo/redo support
- Multi-map tab interface

## Requirements

### Build Dependencies

- CMake 3.15 or higher
- C++17 compatible compiler (MSVC 2017+, GCC 7+, Clang 5+)
- SDL3 (https://github.com/libsdl-org/SDL)
- OpenGL 3.3+
- Dear ImGui (https://github.com/ocornut/imgui)
- stb_image.h (https://github.com/nothings/stb)

### Runtime Dependencies

- DMCompilerCpp library (included in parent directory)

## Building

### Setup Dependencies

1. **SDL3**: Install system-wide or place in `external/SDL3/`
2. **ImGui**: Clone into `external/imgui/`
   ```bash
   git clone https://github.com/ocornut/imgui.git external/imgui
   ```
3. **stb_image**: Download to `external/stb/stb_image.h`
   ```bash
   curl -o external/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
   ```

### Windows (PowerShell)

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Running

```bash
# Windows
.\build\bin\Release\myg_editor.exe

# Linux/macOS
./build/bin/myg_editor
```

## Project Structure

```
MYGEditor/
├── CMakeLists.txt          # Main build configuration
├── README.md               # This file
├── src/                    # Source files (.cpp)
│   └── main.cpp           # Application entry point
├── include/                # Public headers
│   └── myg/               # MYG Editor headers
├── external/               # Third-party dependencies
│   ├── imgui/             # Dear ImGui (git clone)
│   └── stb/               # stb_image.h (download)
├── tests/                  # Test suite
│   ├── CMakeLists.txt
│   └── testdata/          # Test data files
└── build/                  # Build output (generated)
```

## Development Status

This project is currently in initial development. See the spec files in `.kiro/specs/myg-editor-cpp-port/` for detailed requirements, design, and implementation plan.

## License

[To be determined - should match parent project license]
