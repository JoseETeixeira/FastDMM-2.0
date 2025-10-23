# Technology Stack

## FastDMM (Java)

### Build System

- **Gradle** with wrapper scripts (gradlew/gradlew.bat)
- Capsule plugin for fat JAR packaging
- Natives plugin for LWJGL native libraries

### Tech Stack

- Java 8 (minimum requirement)
- LWJGL 2.9.3 (OpenGL bindings)
- PNGJ (PNG image handling)
- Google Guava 19.0
- JLine 2.12.1 (console input)
- org.json 20160810
- Lombok 1.16.10 (compile-time only)

### Common Commands

```bash
# Build fat JAR (includes all dependencies)
.\gradlew fatCapsule

# Run application
.\gradlew run

# Build project
.\gradlew build

# Run tests
.\gradlew test
```

Output JAR location: `build/libs/`

## DMCompiler (C++)

### Build System

- **CMake** 3.15+ for cross-platform builds
- CTest for testing

### Tech Stack

- C++17 standard
- nlohmann/json (optional, embedded version available)
- Standard C++ containers (std::vector, std::unordered_map)

### Common Commands

#### Windows (PowerShell)

```powershell
# Build
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Run tests
ctest -C Release

# Compile a .dme file
.\Release\dmcompiler.exe path\to\project.dme

# Disassemble
.\Release\dmdisasm.exe compiled.json
```

#### Linux/macOS

```bash
# Build
mkdir build
cd build
cmake ..
make -j$(nproc)

# Run tests
make test

# Compile a .dme file
./dmcompiler path/to/project.dme

# Disassemble
./dmdisasm compiled.json
```

### Compiler Options

- `--verbose` - Show verbose output
- `--suppress-unimplemented` - Don't warn about unimplemented features
- `--version [VER].[BUILD]` - Set DM_VERSION and DM_BUILD macros
- `--dump-preprocessor` - Save preprocessor output
- `--no-standard` - Disable built-in standard library
- `--define KEY=VAL` - Add custom defines

Output location: `build/Release/` (Windows) or `build/` (Linux/macOS)

## Platform Support

- Windows (Visual Studio 2017+, MSVC)
- Linux (GCC 7+)
- macOS (Clang 5+, Xcode command line tools)
