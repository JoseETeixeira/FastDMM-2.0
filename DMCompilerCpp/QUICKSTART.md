# Quick Start Guide

## Testing the C++ Compiler with BYOND Files

### Option 1: Test with Included Sample

The project includes a simple test in `tests/testdata/simple_test/`:

```powershell
# Build first
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Test with sample
cd ..
.\build\Release\dmcompiler.exe .\tests\testdata\simple_test\test.dme
```

### Option 2: Test with TestGame from Parent Directory

```powershell
# From DMCompilerCpp directory
.\build\Release\dmcompiler.exe ..\TestGame\environment.dme
```

### Option 3: Test with Your Own .dme File

```powershell
# Copy your BYOND project to testdata
cp -Recurse C:\path\to\your\byond\project .\tests\testdata\my_project\

# Compile it
.\build\Release\dmcompiler.exe .\tests\testdata\my_project\your_game.dme
```

## Running Tests

```powershell
# Run all tests
cd build
ctest -C Release

# Or run specific test categories
.\Release\dm_compiler_tests.exe [lexer]
.\Release\dm_compiler_tests.exe [compiler]
```

## Expected Output

Since this is a partial implementation, you should see:

```
OpenDream DM Compiler (C++ Implementation)
Compiling: test.dme
Phase 1: Preprocessing...
Phase 2: Parsing...
Phase 3: Building object tree...
Phase 4: Emitting bytecode...
Phase 5: Writing JSON output...
Output written to: test.json
Compilation succeeded with 0 warnings
Total time: 0s
```

The lexer will work and tokenize your .dm files correctly. The other phases are stubs that will need full implementation.

## Testing the Lexer Specifically

```powershell
# Create a test file
echo 'var x = 10; proc test() { return x; }' > test_input.dm

# The lexer tests will automatically run with:
.\build\Release\dm_compiler_tests.exe [lexer]
```

## What Works vs What Needs Implementation

### ✅ Fully Implemented
- **Lexer**: Complete tokenization of DM code
  - Keywords, identifiers, operators
  - String literals with escape sequences
  - Number literals (decimal, hex, float)
  - Comments (line and block)
- **Token system**: Full token representation
- **Location tracking**: File/line/column tracking
- **DreamPath**: Path manipulation and hierarchy
- **Command-line parsing**: Full argument handling
- **Test infrastructure**: Basic test framework

### ⚠️ Stub/Incomplete
- **Preprocessor**: #include, #define, #if (needs porting from C#)
- **Parser**: AST building (needs porting from C#)
- **Object Tree**: Type hierarchy management (needs porting from C#)
- **Code Tree**: Code structure (needs porting from C#)
- **Bytecode Emitter**: Opcode generation (needs porting from C#)
- **JSON Output**: Serialization (needs porting from C#)
- **Map Parser**: .dmm file parsing (needs porting from C#)
- **Disassembler**: Full disassembly (needs porting from C#)

## Next Steps for Development

1. **Port the Preprocessor**
   - Study `DMCompiler/Compiler/DMPreprocessor/`
   - Implement macro expansion and file inclusion

2. **Port the Parser**
   - Study `DMCompiler/Compiler/DM/DMParser.cs`
   - Build the AST structure

3. **Port Object Tree Builder**
   - Study `DMCompiler/DM/DMObjectTree.cs`
   - Implement type hierarchy

4. **Complete the Pipeline**
   - Add bytecode emission
   - Add JSON serialization
   - Test with real BYOND games

## Useful Test Commands

```powershell
# Verbose output
.\build\Release\dmcompiler.exe --verbose test.dme

# Dump preprocessor output (when implemented)
.\build\Release\dmcompiler.exe --dump-preprocessor test.dme

# Disable optimizations
.\build\Release\dmcompiler.exe --no-opts test.dme

# With custom defines
.\build\Release\dmcompiler.exe --define DEBUG=1 --define VERSION=2.0 test.dme
```

## Comparing with C# Version

To verify the C++ version matches the C# behavior:

```powershell
# Compile with C# version
cd ..\DMCompiler
dotnet run -- ..\TestGame\environment.dme

# Compile with C++ version
cd ..\DMCompilerCpp
.\build\Release\dmcompiler.exe ..\TestGame\environment.dme

# Compare outputs (when fully implemented)
diff output1.json output2.json
```
