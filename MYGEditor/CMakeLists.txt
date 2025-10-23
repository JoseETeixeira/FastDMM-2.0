cmake_minimum_required(VERSION 3.15)
project(MYGEditor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)

# SDL3 - Try to find it, or provide instructions
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found. You can:")
    message(STATUS "  1. Install SDL3 system-wide")
    message(STATUS "  2. Add SDL3 as a subdirectory in external/")
    message(STATUS "  3. Set SDL3_DIR to point to SDL3Config.cmake")
    message(STATUS "")
    message(STATUS "For now, attempting to use external/SDL3 if available...")
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/CMakeLists.txt")
        add_subdirectory(external/SDL3)
    else()
        message(FATAL_ERROR "SDL3 not found. Please install SDL3 or add it to external/SDL3")
    endif()
endif()

# ImGui - Add as subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.h")
    # ImGui doesn't have CMakeLists.txt by default, so we create a library manually
    add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_sdl3.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
    )
    
    target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
    )
    
    target_link_libraries(imgui PUBLIC SDL3::SDL3 OpenGL::GL)
else()
    message(STATUS "ImGui not found in external/imgui")
    message(STATUS "Please clone ImGui: git clone https://github.com/ocornut/imgui.git external/imgui")
    message(FATAL_ERROR "ImGui is required")
endif()

# stb_image - Header-only library
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h")
    message(STATUS "stb_image.h not found in external/stb")
    message(STATUS "Please download from: https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
    message(WARNING "stb_image.h is required for DMI loading")
endif()

# DMCompilerCpp - Link to existing library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../DMCompilerCpp/CMakeLists.txt")
    # Set the include directory before adding subdirectory
    set(DMCOMPILER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DMCompilerCpp/include")
    add_subdirectory(../DMCompilerCpp ${CMAKE_BINARY_DIR}/DMCompilerCpp)
    # Manually fix the include directories for DMCompilerLib since it uses CMAKE_SOURCE_DIR
    target_include_directories(DMCompilerLib PUBLIC ${DMCOMPILER_INCLUDE_DIR})
else()
    message(FATAL_ERROR "DMCompilerCpp not found at ../DMCompilerCpp")
endif()

# MYG Editor source files
set(MYG_SOURCES
    src/main.cpp
    src/MYGEditor.cpp
    src/InputHandler.cpp
    src/ProjectManager.cpp
    src/ObjectTreeCache.cpp
    src/DMI.cpp
    src/DMICache.cpp
    src/Map.cpp
    src/TileInstance.cpp
    src/MapManager.cpp
    src/Renderer.cpp
    src/Camera.cpp
    src/GUIManager.cpp
    src/UndoableAction.cpp
)

# MYG Editor executable
add_executable(myg_editor ${MYG_SOURCES})

target_include_directories(myg_editor PRIVATE
    include
    external/stb
    ../DMCompilerCpp/include
)

target_link_libraries(myg_editor PRIVATE
    SDL3::SDL3
    OpenGL::GL
    imgui
    DMCompilerLib
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(myg_editor PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    # Copy SDL3 DLL to output directory on Windows
    if(TARGET SDL3::SDL3)
        add_custom_command(TARGET myg_editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:myg_editor>
        )
    endif()
elseif(APPLE)
    # macOS-specific settings
    target_link_libraries(myg_editor PRIVATE "-framework Cocoa" "-framework IOKit")
endif()

# Tests
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS myg_editor
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "")
message(STATUS "MYG Editor Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
