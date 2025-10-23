# Project Structure

This is a dual-project repository with separate Java and C++ components.

## Root Level

```
/
├── src/                    # FastDMM Java source code
├── DMCompilerCpp/          # DM Compiler C++ implementation
├── build.gradle            # FastDMM Gradle build configuration
├── settings.gradle         # Gradle settings
├── gradlew / gradlew.bat   # Gradle wrapper scripts
└── README.md               # FastDMM documentation
```

## FastDMM (Java) Structure

```
src/
└── main/
    ├── java/
    │   └── com/github/monster860/fastdmm/  # Main package
    └── resources/                           # Application resources

gradle/wrapper/             # Gradle wrapper files
build/                      # Build output (generated)
```

### Conventions

- Package: `com.github.monster860.fastdmm`
- Java 8 compatibility required
- Uses Lombok for boilerplate reduction
- Fat JAR packaging via Capsule plugin

## DMCompiler (C++) Structure

```
DMCompilerCpp/
├── include/                # Public header files (.h)
│   ├── DMCompiler.h
│   ├── Lexer.h
│   ├── DMParser.h
│   ├── DMObjectTree.h
│   └── ...
├── src/                    # Implementation files (.cpp)
│   ├── main.cpp           # Compiler entry point
│   ├── disassembler_main.cpp  # Disassembler entry point
│   ├── DMCompiler.cpp
│   ├── Lexer.cpp
│   └── ...
├── tests/                  # Test suite
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   └── testdata/          # Test .dme/.dm files
├── DMStandard/            # Built-in standard library
├── docs/                  # Documentation
├── CMakeLists.txt         # Main CMake configuration
└── README.md              # DMCompiler documentation
```

### Core Components

1. **Lexer** - Tokenizes DM source code
2. **Preprocessor** - Handles #define, #include, #if directives
3. **Parser** - Builds Abstract Syntax Tree (AST)
4. **Object Tree** - Manages DM object hierarchy and type system
5. **Code Tree** - Variable and proc management
6. **Bytecode Emitter** - Generates bytecode from AST
7. **JSON Output** - Serializes to OpenDream-compatible JSON
8. **Disassembler** - Converts bytecode back to readable format

### Conventions

- C++17 standard required
- Headers in `include/`, implementations in `src/`
- Test data in `tests/testdata/`
- Build output in `build/` (git-ignored)
- MSVC requires `/FS` flag for parallel compilation

## File Extensions

- `.dm` - Dream Maker source files
- `.dme` - Dream Maker environment files (project root)
- `.dmm` - Dream Maker map files
- `.json` - Compiled bytecode output

## Documentation Files

Both projects include extensive markdown documentation:

- Status reports and progress tracking
- Implementation guides and session notes
- Architecture analysis documents
- Task completion summaries

These are primarily for development tracking and should not be modified during normal code changes.
