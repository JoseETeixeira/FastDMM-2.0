#!/bin/bash
# Bash script to set up MYG Editor dependencies

echo "Setting up MYG Editor dependencies..."

# Check if external directory exists
if [ ! -d "external" ]; then
    echo "Error: Run this script from the MYGEditor directory"
    exit 1
fi

# Download stb_image.h
echo ""
echo "Downloading stb_image.h..."
STB_PATH="external/stb/stb_image.h"
if [ -f "$STB_PATH" ]; then
    echo "  stb_image.h already exists"
else
    if curl -o "$STB_PATH" "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"; then
        echo "  Downloaded stb_image.h successfully"
    else
        echo "  Failed to download stb_image.h"
    fi
fi

# Clone ImGui
echo ""
echo "Cloning Dear ImGui..."
if [ -d "external/imgui" ]; then
    echo "  ImGui directory already exists"
else
    if git clone https://github.com/ocornut/imgui.git external/imgui; then
        echo "  Cloned ImGui successfully"
    else
        echo "  Failed to clone ImGui"
        echo "  You can manually clone: git clone https://github.com/ocornut/imgui.git external/imgui"
    fi
fi

# Check for SDL3
echo ""
echo "Checking for SDL3..."
if [ -d "external/SDL3" ]; then
    echo "  SDL3 found in external/SDL3"
else
    echo "  SDL3 not found in external/SDL3"
    echo "  Options:"
    echo "    1. Install SDL3 system-wide (recommended for Linux/macOS)"
    echo "    2. Clone SDL3 to external/SDL3: git clone https://github.com/libsdl-org/SDL.git external/SDL3"
fi

echo ""
echo "Dependency setup complete!"
echo "Next steps:"
echo "  1. Ensure SDL3 is available (see above)"
echo "  2. Run: mkdir build && cd build && cmake .."
echo "  3. Run: make -j\$(nproc)"

# Make this script executable
chmod +x "$0" 2>/dev/null
