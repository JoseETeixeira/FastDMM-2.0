# Building DMCompiler C++

## Windows Build Instructions

### Prerequisites
1. Install Visual Studio 2019 or later with C++ support
2. Install CMake 3.15 or later (https://cmake.org/download/)
3. (Optional) Install vcpkg for dependency management

### Build Steps

#### Using Visual Studio
```powershell
# Open PowerShell in the DMCompilerCpp directory
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Executables will be in: build/Release/dmcompiler.exe and build/Release/dmdisasm.exe
```

#### Using Visual Studio IDE
```powershell
# Generate Visual Studio solution
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"

# Then open DMCompilerCpp.sln in Visual Studio and build
```

### Running Tests
```powershell
cd build
ctest -C Release

# Or run the test executable directly
.\Release\dm_compiler_tests.exe
```

### Testing with BYOND Files

Place your .dme/.dm files in the `tests/testdata/` directory:
```
tests/testdata/
  my_project/
    game.dme
    code/
      main.dm
      objects.dm
```

Then compile:
```powershell
.\Release\dmcompiler.exe tests\testdata\my_project\game.dme
```

## Linux/macOS Build Instructions

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake g++

# macOS
brew install cmake
# Xcode command line tools should provide g++
```

### Build Steps
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)

# Executables will be in: build/dmcompiler and build/dmdisasm
```

### Running Tests
```bash
cd build
make test

# Or run directly
./dm_compiler_tests
```

## Using the Compiler

### Basic Usage
```bash
# Compile a .dme file
./dmcompiler path/to/your/project.dme

# With verbose output
./dmcompiler --verbose path/to/your/project.dme

# Show all options
./dmcompiler --help
```

### Example with TestGame
```bash
# From the DMCompilerCpp directory
./build/dmcompiler ../TestGame/environment.dme
```

## Using the Disassembler

```bash
# Interactive mode
./dmdisasm compiled_output.json

# Test mode (for CI)
./dmdisasm compiled_output.json crash-on-test

# Dump all
./dmdisasm compiled_output.json dump-all
```

## Troubleshooting

### CMake can't find C++ compiler
Make sure your compiler is in PATH:
```powershell
# Windows - check Visual Studio installation
where cl.exe

# Linux/macOS
which g++
```

### Missing nlohmann/json
The project will work without it, but if you want to use the system version:
```bash
# Using vcpkg (Windows)
vcpkg install nlohmann-json

# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev

# macOS
brew install nlohmann-json
```

### Build Errors
- Make sure you're using C++17 or later
- Check that all .cpp files are listed in CMakeLists.txt
- Try a clean build: `rm -rf build && mkdir build && cd build && cmake ..`

## IDE Setup

### Visual Studio Code
1. Install C/C++ extension
2. Install CMake Tools extension
3. Open the DMCompilerCpp folder
4. Select a kit (compiler) when prompted
5. Build with F7 or click "Build" in the status bar

### CLion
1. Open the DMCompilerCpp folder
2. CLion will automatically detect CMake
3. Build with Ctrl+F9

## Next Steps

This is a **partial implementation** with stubs for complex components. To complete:

1. Implement preprocessor logic (DMPreprocessor.cpp)
2. Implement full parser (DMParser.cpp)  
3. Implement object tree builder (DMObjectTree.cpp)
4. Implement bytecode emitter (BytecodeEmitter.cpp)
5. Implement JSON serialization (JsonOutput.cpp)
6. Implement map parser (DMMParser.cpp)

The lexer and basic structure are functional and can be tested with real .dm files.
