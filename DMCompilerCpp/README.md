# DM Compiler & Disassembler - C++ Implementation

This is a C++ port of the OpenDream DM (BYOND) compiler and disassembler, designed to compile and analyze `.dm` and `.dme` files.

## Features

- **DM Compiler**: Compiles BYOND Dream Maker files (.dme/.dm) to JSON bytecode
- **DM Disassembler**: Disassembles compiled JSON back to readable format
- **Full Preprocessor**: Handles #define, #include, #if, #ifdef, etc.
- **Lexer & Parser**: Complete lexical analysis and syntax parsing
- **Bytecode Generation**: Generates optimized bytecode
- **Map Support**: Handles .dmm map files

## Building

### Requirements

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- nlohmann/json library (optional, will use embedded version if not found)

### Build Instructions

#### Windows (Visual Studio)
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

### Compiler

```bash
# Compile a .dme file
./dmcompiler path/to/your/project.dme

# With options
./dmcompiler --verbose --suppress-unimplemented path/to/project.dme

# Show help
./dmcompiler --help
```

#### Compiler Options

- `--help` - Show help message
- `--version [VER].[BUILD]` - Set DM_VERSION and DM_BUILD macros
- `--skip-bad-args` - Skip unrecognized arguments
- `--suppress-unimplemented` - Don't warn about unimplemented features
- `--suppress-unsupported` - Don't warn about unsupported features
- `--dump-preprocessor` - Save preprocessor output
- `--no-standard` - Disable built-in standard library
- `--define KEY=VAL` - Add custom defines
- `--verbose` - Show verbose output
- `--no-opts` - Disable optimizations (debug only)

### Disassembler

```bash
# Disassemble a compiled JSON file
./dmdisasm path/to/compiled.json

# Test mode (for CI)
./dmdisasm compiled.json crash-on-test

# Dump all types
./dmdisasm compiled.json dump-all
```

## Testing

The project includes test cases that can be run with actual BYOND .dme/.dm files:

```bash
# Run all tests
cd build
ctest

# Or run test executable directly
./tests/dm_compiler_tests
```

### Test Structure

Place your test .dme files in the `tests/testdata/` directory:

```
tests/
  testdata/
    simple_test/
      test.dme
      test.dm
    complex_test/
      game.dme
      code/
        main.dm
```

## Architecture

### Core Components

1. **Lexer** (`Lexer.cpp`/`DMLexer.cpp`)
   - Tokenizes input source code
   - Handles DM-specific syntax

2. **Preprocessor** (`DMPreprocessor.cpp`)
   - Processes #define, #include, #if directives
   - Macro expansion
   - Conditional compilation

3. **Parser** (`DMParser.cpp`)
   - Builds Abstract Syntax Tree (AST)
   - Handles DM language grammar
   - Error recovery

4. **Code Tree** (`DMCodeTree.cpp`/`DMObjectTree.cpp`)
   - Manages object hierarchy
   - Variable and proc management
   - Type system

5. **Bytecode Emitter** (`BytecodeEmitter.cpp`)
   - Generates bytecode from AST
   - Optimization passes
   - Opcode emission

6. **JSON Output** (`JsonOutput.cpp`)
   - Serializes compiled code to JSON
   - Compatible with OpenDream runtime

### File Structure

```
DMCompilerCpp/
├── include/           # Header files
│   ├── DMCompiler.h
│   ├── Lexer.h
│   ├── Token.h
│   ├── DMParser.h
│   ├── DMObjectTree.h
│   └── ...
├── src/              # Implementation files
│   ├── DMCompiler.cpp
│   ├── main.cpp
│   ├── disassembler_main.cpp
│   └── ...
├── tests/            # Test suite
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   └── testdata/
└── CMakeLists.txt
```

## Contributing

When contributing, please:

1. Follow C++17 standard
2. Use consistent naming conventions
3. Add tests for new features
4. Update documentation

## Differences from C# Version

- Uses standard C++ containers (std::vector, std::unordered_map, etc.)
- Error handling via exceptions and return codes
- nlohmann/json for JSON serialization
- CMake for cross-platform builds

## License

Same license as the parent OpenDream project.

## See Also

- [OpenDream Project](https://github.com/OpenDreamProject/OpenDream)
- [BYOND Reference](http://www.byond.com/docs/ref/)
