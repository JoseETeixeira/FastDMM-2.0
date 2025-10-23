# Architectural Issues Analysis

## Executive Summary

The C++ compiler has multiple fundamental architectural issues that prevent proper identifier resolution. These issues stem from the compiler not matching the C# implementation's architecture.

## Critical Issues Discovered

### 1. Preprocessor Not Expanding Macros ❌

**Problem**: `#define` constants like `NORTH`, `SOUTH`, `EAST`, `WEST`, `TRUE`, `FALSE` are NOT being expanded by the preprocessor.

**Evidence**:
- DMStandard/Defines.dm contains: `#define NORTH 1`
- Debug output shows: `"DEBUG: Type 'NORTH' not found as path '/NORTH'"`
- This means `NORTH` is reaching the parser as an identifier token, not as the integer `1`

**Expected Behavior** (C# implementation):
- Preprocessor should replace all `NORTH` tokens with `1` BEFORE parsing
- Parser should never see `NORTH` as an identifier
- Expression compiler should only see integer constant `1`

**Current Behavior**:
- `NORTH` reaches parser as identifier
- Parser creates `DMASTIdentifier("NORTH")`
- Expression compiler tries to resolve `"NORTH"` as type `/NORTH`
- Fails and generates warning

**Root Cause**: The preprocessor's `Define()` and macro expansion system is not being invoked properly, or macros are not being expanded during tokenization.

### 2. Boolean Literals Not Recognized ❌

**Problem**: `true` and `false` are being treated as identifiers, not as boolean literals.

**Evidence**:
- Debug output shows: `"DEBUG: Type 'true' not found as path '/true'"`
- Debug output shows: `"DEBUG: Type 'false' not found as path '/false'"`

**Expected Behavior** (C# implementation):
- Lexer/parser should recognize `true`/`false` as keywords or special tokens
- Should create constant expression nodes (e.g., `DMASTConstantInteger(1)` for true)
- Never reach identifier resolution

**Current Behavior**:
- Treated as regular identifiers
- Parser creates `DMASTIdentifier("true")`
- Expression compiler tries to resolve as type `/true`

**Root Cause**: Lexer doesn't tokenize `true`/`false` as special tokens, or parser doesn't handle them specially.

### 3. Type Name Lookup Incomplete ⚠️

**Problem**: While we implemented type path lookup, types like `/mob` and `/obj` are not being found even though they ARE in the object tree.

**Evidence**:
- Debug output shows: `"DEBUG: Type 'mob' not found as path '/mob'"`
- But BuildObjectTree() completes BEFORE EmitBytecode()
- `/mob` should be in the tree

**Hypothesis**: 
- Compilation order issue: DMStandard procs reference user types before they're defined
- OR: ObjectTree not properly populated at lookup time
- OR: TryGetTypeId() implementation bug

**Expected Behavior**:
- All types defined before any proc compilation
- Type lookups should succeed

**Current Status**: Partially implemented but not working in all cases.

### 4. Identifier Resolution Strategy Wrong ⚠️

**Current Resolution Order**:
1. Local variables
2. Special identifiers (src, usr, world, args)
3. Member variables
4. **NEW**: Type names (partially working)
5. ~~Global variables~~ (NOT IMPLEMENTED)
6. ~~Global proc names~~ (NOT IMPLEMENTED)
7. Fallback: Warning + null

**C# Implementation Order** (needs verification):
1. Preprocessor macros (#define) - **DONE DURING TOKENIZATION, not in CompileIdentifier**
2. Special literals (true, false, null) - **DONE DURING PARSING, not in CompileIdentifier**
3. Local variables
4. Special identifiers (src, usr, world, args)
5. Member variables
6. Type names
7. Global variables
8. Global proc names
9. Fallback: Error

**Key Difference**: C# does preprocessing and parsing correctly, so CompileIdentifier() only deals with actual variable/type/proc names, not constants or literals.

## Impact on Warnings

**Current Warning Count**: 44 warnings

**Breakdown by Category** (estimated from debug output):
- Direction constants (NORTH, SOUTH, EAST, WEST, NORTHEAST, etc.): ~16 warnings
- Boolean literals (true, false): ~4 warnings  
- Type references (List, mob, obj, player, item): ~10 warnings
- Unknown identifiers (Ref, Trg, Glue, Start, End): ~10 warnings
- Other: ~4 warnings

**Expected Warning Count After Proper Implementation**: 0-5 warnings (only truly unknown identifiers)

## Root Cause Analysis

The fundamental issue is that the **C++ compiler's architecture doesn't match the C# implementation**:

| Component | C# Implementation | C++ Implementation | Status |
|-----------|------------------|-------------------|--------|
| Preprocessor | Expands #defines fully | Partially implemented | ❌ BROKEN |
| Lexer | Recognizes true/false as keywords | Treats as identifiers | ❌ BROKEN |
| Parser | Creates constant nodes for literals | Creates identifier nodes | ❌ BROKEN |
| Type Resolution | Happens before proc compilation | Happens during | ⚠️ QUESTIONABLE |
| Identifier Resolution | Clean separation of concerns | Mixed responsibilities | ⚠️ INCOMPLETE |

## Recommendations

### Option 1: Surgical Fixes (Current Approach)

**Pros**:
- Faster implementation
- Incremental progress
- Easier to test

**Cons**:
- May miss underlying architectural issues
- Band-aid solutions
- Tech debt accumulation

**Estimated Effort**: 1-2 days

**Approach**:
1. Fix preprocessor macro expansion
2. Add true/false as special tokens in lexer
3. Fix type lookup timing issue
4. Add global variable/proc lookup

### Option 2: Architectural Rewrite (User's Request)

**Pros**:
- Match C# implementation exactly
- Correct architecture from the start
- Prevent future issues
- Cleaner codebase

**Cons**:
- Much longer implementation time
- Requires deep C# code analysis
- Higher risk of breaking existing functionality
- All-or-nothing approach

**Estimated Effort**: 1-2 weeks

**Approach**:
1. Analyze C# lexer/tokenizer implementation
2. Rewrite C++ lexer to match exactly
3. Analyze C# parser implementation  
4. Rewrite C++ parser to match exactly
5. Analyze C# expression compiler
6. Rewrite C++ expression compiler to match exactly
7. Verify all test cases pass

### Option 3: Hybrid Approach (RECOMMENDED)

**Pros**:
- Balance between speed and correctness
- Use C# as reference for fixes
- Maintain architectural compatibility
- Progressive improvement

**Cons**:
- Still requires C# analysis
- May take longer than Option 1
- Requires discipline to maintain C# parity

**Estimated Effort**: 3-5 days

**Approach**:
1. **Study C# preprocessor** - understand macro expansion
2. **Fix C++ preprocessor** to match C# behavior
3. **Study C# lexer** - understand keyword/literal handling
4. **Fix C++ lexer** to recognize true/false/null
5. **Study C# parser** - understand constant handling
6. **Fix C++ parser** to create proper AST nodes
7. **Study C# expression compiler** - understand identifier resolution
8. **Fix C++ identifier resolution** to match C# logic
9. **Verify** against C# test cases

## Next Steps

1. **Immediate**: Remove debug logging (added for investigation)
2. **Decision Point**: Choose Option 1, 2, or 3 based on project priorities
3. **If Option 1/3**: Start with preprocessor macro expansion fix
4. **If Option 2**: Begin comprehensive C# source code analysis

## Technical Debt Assessment

**Current State**: 
- Preprocessor: 40% complete
- Lexer: 60% complete  
- Parser: 70% complete
- Expression Compiler: 60% complete

**Target State** (match C#):
- All components: 100% C# parity

**Debt**: Significant architectural misalignment with reference implementation

## Conclusion

The identifier resolution issues are symptoms of deeper architectural problems. While surgical fixes can reduce warning count, achieving full C# parity requires either:

1. **Systematic comparison** of each component against C# implementation, OR
2. **Complete rewrite** following C# architecture exactly

The user's request for a rewrite is well-founded given the number of architectural discrepancies discovered.
