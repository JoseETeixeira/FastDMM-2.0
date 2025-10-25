# Known Issues in DMCompiler C++

This document tracks known limitations and issues in the C++ implementation of the DM compiler.

## Parser/Semantic Analysis Issues

### 1. For-In Loop Variable Scope

**Status:** Not Implemented  
**Severity:** High  
**Description:** Loop variables declared in for-in loops are not properly added to the local variable scope.

**Example:**

```dm
for (var/mob/M as /mob|mob in world)
    if (M.key == key)  // Error: Unknown identifier 'M'
        mob = M
        break
```

**Error Messages:**

- `Warning: Unknown identifier 'M' ... (not found as local variable, object field, or global variable)`

**Files Affected:**

- `DMStandard/Types/Client.dm:64-67`
- `DMStandard/Types/Atoms/Movable.dm:31`
- `DMStandard/Types/Atoms/Turf.dm:12`

**Root Cause:** The parser's `ParseForStatement` method doesn't properly handle variable declarations in for-in loops with type paths and filters (the `as /mob|mob` syntax).

---

### 2. Type Path Resolution in istype()

**Status:** Not Implemented  
**Severity:** High  
**Description:** Type paths used as arguments to `istype()` cannot be resolved during compilation.

**Example:**

```dm
if (istype(statobj, /atom))  // Error: Type path '/atom' could not be resolved
    statobj.Stat()
```

**Error Messages:**

- `Error: Type path '/atom' could not be resolved in proc //Stat (searched from context: /)`
- `Error: Failed to compile istype() type argument`

**Files Affected:**

- `DMStandard/Types/Client.dm:84`
- `DMStandard/Types/Regex.dm:13`
- `DMStandard/Types/Sound.dm:30`

**Root Cause:** The expression compiler doesn't properly handle type path literals as function arguments. Type paths need special handling during bytecode emission.

---

### 3. Break Statement Outside Loop

**Status:** Parser Warning  
**Severity:** Low  
**Description:** Break statements are being flagged as outside loops even when they appear to be inside for loops.

**Example:**

```dm
for (var/mob/M as /mob|mob in world)
    if (M.key == key)
        mob = M
        break  // Warning: break statement outside of loop
```

**Error Messages:**

- `Warning: break statement outside of loop at ... in proc //New`

**Files Affected:**

- `DMStandard/Types/Client.dm:67`

**Root Cause:** Likely related to issue #1 - the for loop isn't being properly recognized as a loop context due to parsing failures.

---

### 4. Unsupported For-In Loop Variable Expression

**Status:** Not Implemented  
**Severity:** High  
**Description:** Complex for-in loop variable expressions with type filters are not supported.

**Example:**

```dm
for (var/atom/movable/A in src)  // Error: Unsupported for-in loop variable expression type
    // ...
```

**Error Messages:**

- `Error: Unsupported for-in loop variable expression type`

**Files Affected:**

- `DMStandard/Types/Atoms/Movable.dm:31`
- `DMStandard/Types/Atoms/Turf.dm:12`

**Root Cause:** The code generator doesn't support all forms of for-in loop variable declarations, particularly those with type paths.

---

## Impact Assessment

These issues affect the DMStandard library and prevent full compilation of DM code that uses:

- For-in loops with typed variables
- Type path arguments to built-in functions like `istype()`
- Complex loop constructs

## Workarounds

For testing purposes, you can:

1. Use `--no-standard` flag to skip DMStandard and test with simpler code
2. Write test files that avoid these language features
3. Use the C# implementation for full DM language support

## Related Components

- **Parser:** `DMCompilerCpp/src/DMParser.cpp` - Statement parsing
- **Code Generator:** `DMCompilerCpp/src/DMCodeTreeBuilder.cpp` - Bytecode emission
- **Expression Compiler:** Expression evaluation and type resolution

## Notes

These are **pre-existing limitations** of the C++ compiler implementation and are **not related to the preprocessor refactoring work**. The preprocessor successfully processes all `#include` directives and produces the correct token stream. These errors occur in later compilation phases (parsing, semantic analysis, code generation).
