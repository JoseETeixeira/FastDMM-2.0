# Implementation Plan - Remaining TODOs and Stubs

## Priority Classification

### 🔴 CRITICAL - Blocks core functionality
1. **DMASTConstantPath compilation** - Currently pushes null, breaks locate() tests
2. **Identifier resolution** - Need proper variable/field lookup
3. **Variable tracking in proc context** - Assignment operations need this

### 🟡 HIGH - Needed for complete compiler
4. **ParseFiles()** - Main parsing phase (Phase 2)
5. **BuildObjectTree()** - Object tree construction (Phase 3)  
6. **EmitBytecode()** - Bytecode generation (Phase 4)
7. **OutputJson()** - JSON serialization (Phase 5)

### 🟢 MEDIUM - Nice to have
8. **DMDisassembler** - For debugging bytecode
9. **Named function parameters** - parser line 522
10. **Full expression evaluator for preprocessor** - line 607
11. **Statement parsing stubs** - DMStatementCompiler.cpp line 412+

### ⚪ LOW - Future enhancements
12. **DMMParser** - Map file parsing
13. **DMCodeTree** - Code optimization
14. **BytecodeEmitter** - Additional emission helpers
15. **Test-all and dump-all in disassembler**

## Implementation Strategy

### Phase 1: Fix Critical Bugs (IMMEDIATE)
- ✅ Implement CompileConstantPath with PushType opcode
- ✅ Add type lookup from ObjectTree
- ✅ Fix identifier resolution to check locals, fields, globals

### Phase 2: Parser Integration (NEXT)
- Use existing DMParser to parse PreprocessedTokens_
- Build AST from parser output
- Store AST for later phases

### Phase 3: Object Tree & Bytecode (AFTER PARSER)
- Walk AST to build object tree
- Compile procedures to bytecode
- Use existing DMExpressionCompiler and DMStatementCompiler

### Phase 4: Output & Testing (FINAL)
- Serialize object tree and bytecode to JSON
- Test with real DM files
- Validate against C# output

## Current Status

- ✅ Preprocessing complete
- ✅ Expression compiler implemented (48/48 tests + 5 built-ins)
- ✅ Statement compiler implemented
- ✅ DMStandard integration complete
- 🔄 Path constant compilation (in progress)
- ❌ Parser integration pending
- ❌ Object tree building pending
- ❌ Bytecode emission pending
- ❌ JSON output pending

## Next Steps

1. Fix DMASTConstantPath compilation (THIS FILE)
2. Add type ID lookup from path
3. Test locate() with type paths
4. Document results
