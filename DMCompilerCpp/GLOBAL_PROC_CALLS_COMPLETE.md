# Global Proc Calls Implementation - COMPLETE ✅

**Date**: 2025-01-27  
**Session**: Global Proc Call Support Implementation  
**Status**: ✅ **COMPLETE** - 21/21 expression compiler tests passing

---

## Summary

Successfully implemented full support for **global procedure calls** in the C++ DM compiler. Global proc calls (like `sleep(10)`, `rand(1, 100)`, `world.log(msg)`) are now compiled correctly with proper bytecode emission.

### What Was Implemented

1. **Proc Lookup Infrastructure** (DMObjectTree)
   - `RegisterGlobalProc(procName, procId)` - Register global procs for lookup
   - `GetGlobalProcId(procName)` - Look up proc ID by name (returns -1 if not found)

2. **Bytecode Reference Emission** (BytecodeWriter)
   - `EmitGlobalProcReference(procId)` - Emit 5-byte GlobalProc reference (type byte + proc ID)

3. **Expression Compilation** (DMExpressionCompiler)
   - Updated `CompileCall()` to handle global proc calls
   - Proc ID lookup from ObjectTree
   - Error handling for unknown procs
   - Arguments compilation (push onto stack)
   - Call opcode emission with GlobalProc reference

4. **Infrastructure Setup** (DMCompiler)
   - Initialize `ObjectTree_` and `CodeTree_` in constructor

---

## Technical Details

### Bytecode Format

**Call Opcode** (0x0A):
```
<Call opcode> <DMReference> <args_type> <arg_count>
   1 byte        5 bytes       1 byte      4 bytes
```

**GlobalProc Reference** (DMReference.Type = 11):
```
<Type byte> <Proc ID>
    0x0B      4 bytes (int32, little-endian)
```

### Example: `sleep(10)`

**Bytecode**:
```
PushFloat 10.0      // 5 bytes: 0x28 + 4-byte float
Call                // 1 byte:  0x0A
  GlobalProc ref    // 5 bytes: 0x0B + proc_id (e.g., 0x2A000000 for ID 42)
  Args type         // 1 byte:  0x01 (FromStack)
  Arg count         // 4 bytes: 0x01000000 (1 arg)

Total: 16 bytes
```

---

## Files Modified

### 1. `include/DMObjectTree.h` (+13 lines)
```cpp
// Added public methods (lines 145-151)
void RegisterGlobalProc(const std::string& procName, int procId);
int GetGlobalProcId(const std::string& procName) const;
```

### 2. `src/DMObjectTree.cpp` (+13 lines)
```cpp
// Lines 160-171: Implementations
void DMObjectTree::RegisterGlobalProc(const std::string& procName, int procId) {
    GlobalProcs[procName] = procId;
}

int DMObjectTree::GetGlobalProcId(const std::string& procName) const {
    auto it = GlobalProcs.find(procName);
    if (it != GlobalProcs.end()) {
        return it->second;
    }
    return -1;  // Not found
}
```

### 3. `include/BytecodeWriter.h` (+8 lines)
```cpp
// Lines 110-116: Added method declaration
/// Emit a DMReference for a global proc (type byte + proc ID)
void EmitGlobalProcReference(int procId);
```

### 4. `src/BytecodeWriter.cpp` (+7 lines)
```cpp
// Lines 147-152: Implementation
void BytecodeWriter::EmitGlobalProcReference(int procId) {
    WriteByte(11);  // DMReference.Type.GlobalProc
    WriteInt(procId);
}
```

### 5. `src/DMExpressionCompiler.cpp` (+1 include, ~40 lines implementation)
```cpp
// Added include
#include "DMObjectTree.h"

// Lines 280-318: Global proc call implementation
auto* procIdent = dynamic_cast<DMASTIdentifier*>(expr->Target.get());
if (procIdent) {
    const std::string& procName = procIdent->Identifier;
    
    // Look up proc ID
    int procId = Compiler_->GetObjectTree()->GetGlobalProcId(procName);
    if (procId == -1) {
        std::cerr << "Error: Unknown global proc '" << procName << "()'" << std::endl;
        return false;
    }
    
    // Compile arguments (push them onto stack)
    int argCount = 0;
    for (const auto& param : expr->Parameters) {
        if (param->Key) {
            std::cerr << "Error: Named arguments not yet supported" << std::endl;
            return false;
        }
        if (!CompileExpression(param->Value.get())) {
            return false;
        }
        argCount++;
    }
    
    // Emit Call opcode with GlobalProc reference
    DMCallArgumentsType argsType = (argCount == 0) ? 
        DMCallArgumentsType::None : DMCallArgumentsType::FromStack;
    
    Writer_->Emit(DreamProcOpcode::Call);
    Writer_->EmitGlobalProcReference(procId);
    Writer_->AppendByte(static_cast<uint8_t>(argsType));
    Writer_->AppendInt(argCount);
    
    return true;
}
```

###6. `src/DMCompiler.cpp` (+2 lines)
```cpp
// Lines 13-16: Initialize ObjectTree and CodeTree
DMCompiler::DMCompiler()
    : ErrorCount_(0)
    , WarningCount_(0)
    , ObjectTree_(std::make_unique<DMObjectTree>())
    , CodeTree_(std::make_unique<DMCodeTree>())
{
```

### 7. `tests/test_expression_compiler.cpp` (+1 include, +191 lines tests)
```cpp
// Added include
#include "../include/DMObjectTree.h"

// Added 4 comprehensive test cases:
// - TestCompileGlobalProcCallSimple()      - sleep(10)
// - TestCompileGlobalProcCallMultipleArgs() - rand(1, 100)
// - TestCompileGlobalProcCallNoArgs()       - time()
// - TestCompileGlobalProcInExpression()     - rand(5) + 10
```

---

## Test Results

### ✅ All 21 Expression Compiler Tests PASSED

```
=== Running Expression Compiler Tests ===
  TestCompileIntegerConstant... PASSED
  TestCompileAddition... PASSED
  TestCompileUnaryNegation... PASSED
  TestCompileLocalVariable... PASSED
  TestCompileParameter... PASSED
  TestCompileSpecialIdentifierSrc... PASSED
  TestCompileExpressionWithVariable... PASSED
  TestCompileMultipleVariables... PASSED
  TestCompileFieldAccess... PASSED
  TestCompileChainedFieldAccess... PASSED
  TestCompileFieldAccessWithArithmetic... PASSED
  TestCompileMethodCallNoArgs... PASSED
  TestCompileMethodCallWithArgs... PASSED
  TestCompileMethodCallInExpression... PASSED
  TestCompileWorldMethodCall... PASSED
  TestCompileMethodCallMixedArgs... PASSED
  TestCompileNestedMethodCalls... PASSED
  TestCompileGlobalProcCallSimple... PASSED           ← NEW
  TestCompileGlobalProcCallMultipleArgs... PASSED     ← NEW
  TestCompileGlobalProcCallNoArgs... PASSED           ← NEW
  TestCompileGlobalProcInExpression... PASSED         ← NEW

All expression compiler tests passed!
```

### Test Coverage

| Test | DM Code | Bytecode Size | Args | Description |
|------|---------|--------------|------|-------------|
| **TestCompileGlobalProcCallSimple** | `sleep(10)` | 16 bytes | 1 | Simple global proc with one argument |
| **TestCompileGlobalProcCallMultipleArgs** | `rand(1, 100)` | 21 bytes | 2 | Global proc with multiple arguments |
| **TestCompileGlobalProcCallNoArgs** | `time()` | 11 bytes | 0 | Global proc with no arguments |
| **TestCompileGlobalProcInExpression** | `rand(5) + 10` | 22 bytes | 1 | Global proc used in binary expression |

---

## Bytecode Validation Examples

### Test 1: `sleep(10)` (Proc ID 42)

**Expected Bytecode** (16 bytes):
```
Offset  Opcode/Data         Description
------  ------------------  ------------------------------------
0x00    0x28                PushFloat opcode
0x01    [4 bytes]           Float value: 10.0
0x05    0x0A                Call opcode
0x06    0x0B                GlobalProc reference type
0x07    0x2A 0x00 0x00 0x00 Proc ID: 42 (little-endian)
0x0B    0x01                Args type: FromStack
0x0C    0x01 0x00 0x00 0x00 Arg count: 1 (little-endian)
```

### Test 2: `rand(1, 100)` (Proc ID 99)

**Expected Bytecode** (21 bytes):
```
Offset  Opcode/Data         Description
------  ------------------  ------------------------------------
0x00    0x28                PushFloat opcode (arg 1)
0x01    [4 bytes]           Float value: 1.0
0x05    0x28                PushFloat opcode (arg 2)
0x06    [4 bytes]           Float value: 100.0
0x0A    0x0A                Call opcode
0x0B    0x0B                GlobalProc reference type
0x0C    0x63 0x00 0x00 0x00 Proc ID: 99 (little-endian)
0x10    0x01                Args type: FromStack
0x11    0x02 0x00 0x00 0x00 Arg count: 2 (little-endian)
```

### Test 3: `time()` (Proc ID 15)

**Expected Bytecode** (11 bytes):
```
Offset  Opcode/Data         Description
------  ------------------  ------------------------------------
0x00    0x0A                Call opcode
0x01    0x0B                GlobalProc reference type
0x02    0x0F 0x00 0x00 0x00 Proc ID: 15 (little-endian)
0x06    0x00                Args type: None
0x07    0x00 0x00 0x00 0x00 Arg count: 0 (little-endian)
```

---

## Implementation Challenges & Solutions

### Challenge 1: Finding Existing Infrastructure
- **Problem**: Need to determine if proc lookup infrastructure exists
- **Discovery**: `GlobalProcs` map exists in DMObjectTree but had no accessor methods
- **Solution**: Added `RegisterGlobalProc()` and `GetGlobalProcId()` methods

### Challenge 2: ObjectTree Initialization
- **Problem**: Tests crashed with null pointer when calling `compiler.GetObjectTree()`
- **Root Cause**: `ObjectTree_` (unique_ptr) not initialized in DMCompiler constructor
- **Solution**: Added initialization in constructor: `ObjectTree_(std::make_unique<DMObjectTree>())`

### Challenge 3: DMASTCallParameter Constructor Parameter Order
- **Problem**: Tests passing `(location, key, value)` but constructor expects `(location, value, key)`
- **Symptom**: "Named arguments not yet supported" error
- **Solution**: Fixed all test cases to use correct parameter order: `(location, std::move(arg), nullptr)`

### Challenge 4: Code Duplication During Development
- **Problem**: String replacement created duplicate code blocks
- **Detection**: Compilation errors from duplicate variable names
- **Solution**: Carefully replaced entire method block with correct implementation

---

## Architecture Notes

### Proc Lookup Flow

```
User code: sleep(10)
    ↓
Parser creates: DMASTCall(target=DMASTIdentifier("sleep"), params=[10])
    ↓
CompileCall() checks:
    1. Is target DMASTDereference? → No (that's method calls)
    2. Is target DMASTIdentifier? → Yes! (global proc call)
    ↓
Extract proc name: "sleep"
    ↓
Lookup: Compiler_->GetObjectTree()->GetGlobalProcId("sleep") → 42
    ↓
Compile args: Push 10.0 onto stack
    ↓
Emit bytecode:
    - Call opcode (0x0A)
    - GlobalProc reference (0x0B + proc_id)
    - Args type (0x01 = FromStack)
    - Arg count (1)
```

### DMReference Structure

The C# runtime uses `DMReference` union to represent different reference types:

```csharp
public enum DMReference.Type {
    Src = 0,
    Self = 1,
    Argument = 2,
    Local = 3,
    Global = 4,
    GlobalProc = 11,  ← Used for global proc calls
    // ... other types
}
```

**GlobalProc encoding**: `[Type byte: 11] [ProcID: int32]`

---

## Error Handling

The implementation includes proper error handling for:

1. **Unknown Proc Names**:
   ```cpp
   int procId = Compiler_->GetObjectTree()->GetGlobalProcId(procName);
   if (procId == -1) {
       std::cerr << "Error: Unknown global proc '" << procName << "()'" << std::endl;
       return false;
   }
   ```

2. **Named Arguments** (not yet supported):
   ```cpp
   if (param->Key) {
       std::cerr << "Error: Named arguments not yet supported" << std::endl;
       return false;
   }
   ```

---

## Next Steps & Future Work

### Immediate Next Steps (Suggested by Previous Session)
1. ✅ **Global Proc Calls** - COMPLETE
2. 📋 **List Operations** - Create/access lists
3. 📋 **Object Creation** - `new /obj/item()`
4. 📋 **Other Operators** - Ternary, bitwise, etc.
5. 📋 **Statement Compilation** - if, while, for, return, etc.

### Limitations & TODOs

**Current Limitations**:
- ❌ Named arguments not supported (`proc(arg1 = value)`)
- ❌ No runtime proc lookup (all procs must be registered at compile time)
- ❌ No varargs support
- ❌ No default parameter values in calls

**Future Enhancements**:
- [ ] Named argument support
- [ ] Runtime proc lookup for dynamic calls
- [ ] Proc reference support (`var/proc/p = /proc/foo; p()`)
- [ ] Built-in proc auto-registration (sleep, rand, etc.)

---

## Code Statistics

| Metric | Value |
|--------|-------|
| Files Modified | 7 |
| Lines Added (Implementation) | ~82 |
| Lines Added (Tests) | ~192 |
| Total Lines Added | ~274 |
| Test Cases Added | 4 |
| Test Pass Rate | 21/21 (100%) |

---

## Related Documentation

- **Previous**: `FUNCTION_CALL_COMPLETE.md` - Method call implementation
- **Reference**: C# `DMProc.cs` - Runtime proc execution
- **Reference**: C# `DMReference.cs` - Reference type definitions
- **Reference**: C# `DreamProcOpcode.cs` - Opcode definitions

---

## Conclusion

Global proc call support is now **fully functional** with:
- ✅ Complete infrastructure (ObjectTree lookup, BytecodeWriter emission)
- ✅ Proper bytecode generation matching C# runtime format
- ✅ Comprehensive test coverage (4 tests, multiple scenarios)
- ✅ Error handling for edge cases
- ✅ Clean integration with existing call compilation

The compiler can now handle all three call types:
1. ✅ Method calls: `obj.method(args)`
2. ✅ Global proc calls: `sleep(10)`, `rand(1, 100)`
3. 📋 Proc references: `call(procRef)(args)` (future work)

**Total Expression Compiler Test Count**: 21/21 PASSED ✅

---

*Generated: 2025-01-27*  
*Session: Global Proc Call Implementation*  
*Next: Consider list operations or statement compilation*
