# DMCompilerCpp Missing Functionality Gap Analysis

This document compares the current C++ port in `DMCompilerCpp/` with the shipping C# toolchain in `DMCompiler/`. Items below call out compiler features that exist in the C# implementation but are absent or only stubbed in the C++ tree. File paths reference the repositories in this workspace.

## Preprocessing pipeline
- `DMCompilerCpp/src/DMPreprocessor.cpp` implements a minimal macro system: `ReadMacroArguments()` stops at newline and cannot handle nested parentheses, variadic parameters, or stringification/concatenation, while `DMCompiler/Compiler/DMPreprocessor/DMPreprocessor.cs` supports the full BYOND macro grammar.
- `HandleDefineDirective()` ignores `#define FILE_DIR` semantics and never invokes `DMCompiler::AddResourceDirectory`, leaving resource collection and `FILE_DIR` behaviour from the C# codebase unimplemented.
- Conditional evaluation is incomplete. `EvaluateCondition()` (C++ lines ~663-706) falls back to `true` for any non-trivial expression, whereas the C# preprocessor parses the full conditional grammar via `DMPreprocessorParser` and honours operators like `&&`, `||`, `==`, arithmetic, etc.
- `HandlePragmaDirective()` simply discards the directive; the C# counterpart wires pragmas into `DMCompiler.SetPragma` to toggle optional warnings.
- Directive error/warning handling prints directly to `std::cerr` and never calls `DMCompiler::Emit`, unlike the C# preprocessor which routes diagnostics through the compiler’s emission system.
- DMStandard resolution in `PreprocessFiles()` assumes `std::filesystem::current_path()`; the C# compiler locates `_Standard.dm` relative to the executing assembly and falls back gracefully.

## Parsing and AST post-processing
- The C# pipeline runs `DMASTFolder` (constant folding, `set` hoisting, aggregate flattening) before semantic analysis. No equivalent exists in the C++ tree—`DMCompilerCpp` never folds constants, never hoists proc `set` statements, and never collapses aggregated declarations.
- C# supports `DMASTMultipleObjectVarDefinitions` and other compound AST nodes; the C++ parser emits only single `DMASTObjectVarDefinition` nodes and lacks the helpers that the C# `DMCodeTreeBuilder` expects.

## Semantic model and type tree
- `DMCompilerCpp/src/DMCodeTree.cpp` is an empty stub. Consequently none of the responsibilities of `DMCompiler/DM/DMCodeTree*.cs` are present: no staged AddType/AddProc pipeline, no detection of overrides/final modifiers, no automatic initialization proc synthesis, and no code-tree printing.
- `DMCompiler::BuildObjectTree()` performs only shallow object traversal. Features present in the C# builder are missing:
  - `ProcessVarDefinition()` ignores the `const`, `tmp`, `final`, list length, and value-type flags that `DMCodeTree.AddObjectVar` records, and it never serialises default values.
  - `ProcessVarOverride()` contains a TODO and does not update existing vars.
  - `ProcessProcDefinition()` does not track proc attributes (`set opendream_unimplemented`, `set waitfor`, etc.), default parameters, or override detection.
- `DMObjectTree::UpwardSearch()` is simplified and cannot locate proc paths relative to scopes the way `DMCompiler/DM/DMObjectTree.cs` does; super proc resolution and `/proc` vs `/verb` matching are absent.
- There is no equivalent of `DMCompiler.GlobalInitProc` or `DMObject.CreateInitializationProc`; type initialization bytecode is never emitted.
- The string table managed by `DMObjectTree::AddString` is never populated by bytecode emission, so string literals compiled by the C++ backend remain local to `BytecodeWriter` instead of ending up in the global pool the runtime expects.

## Bytecode generation
- `DMCompilerCpp/src/BytecodeEmitter.cpp` is a placeholder; the rich emitter/optimizer stack (`DMProcBuilder`, `DMExpressionBuilder`, `BytecodeOptimizer`) in C# has no direct analogue.
- `DMStatementCompiler` lacks support for multiple core statement forms that the C# builder handles: `for-in`, `switch`, `spawn`, `label`, `goto`, `try/catch`, `throw`, `set`, browse/output/input statements, and proper local-variable initialisation. Many methods return `false` with `ForcedWarning`.
- `DMExpressionCompiler` still contains numerous gaps:
  - `EmitReference()` only understands bare identifiers and special globals, defaulting everything else to a `Local` reference id 0; field writes, indexed writes, global vars, and `initial()` references are unsupported.
  - `CompileCall()` ignores named arguments and cannot resolve global procs or callables beyond `DMASTDereference`/`DMASTIdentifier`; `compile_global_proc` support from the C# builder is absent.
  - `CompileList()` rejects associative list syntax; `CompileNewPath()` emits a hard-coded `typeId = 0`; built-ins such as `spawn`, `sleep`, `initial`, `isnull`, etc., are missing.
- `BytecodeWriter::Finalize()` is never invoked in `DMCompiler::EmitBytecode()`, leaving jump placeholders unfixed. In contrast, the C# `AnnotatedByteCodeWriter` resolves jumps before serialisation.
- String operands produced by `BytecodeWriter::EmitString()` are stored in a private `Strings_` vector that is never merged into `DMObjectTree.StringTable`, so compiled procs lose the string table compatibility guarantees provided by the C# pipeline.
- There is no bytecode optimisation pass (`BytecodeOptimizer` in C#), so even the supported opcodes are emitted without the post-processing performed by the managed compiler.

## Output format and packaging
- `DMCompilerCpp/src/JsonOutput.cpp` is empty. The current `DMCompiler::OutputJson()` writes a minimal structure and omits large portions of the JSON contract produced by `DMCompiler.SaveJson()`:
  - Maps and interface metadata (`DreamMapJson` / DMF paths) are never included because `ConvertMaps()` has no analogue.
  - `GlobalInitProc`, proc locals, argument type flags, annotations, and real `MaxStackSize` values are missing (hard-coded to `100`).
  - Global variables never serialise their default values, whereas the C# compiler emits `GlobalListJson` entries.
  - Optional error overrides, opcode hash metadata, and compression of verb categories mirror fields in `DreamCompiledJson` but are absent in the C++ output.
- Without the JSON writer parity, the runtime cannot treat the C++ compiler as a drop-in replacement.

## Maps, interface files, and ancillary tools
- `DMCompilerCpp/src/DMMParser.cpp` is a stub, so `.dmm` map conversion (`DMCompiler.Compiler.DMM.DMMParser` in C#) is unsupported. `DMPreprocessor::IncludedMaps_` is populated but never consumed.
- Interface (`.dmf`) handling is limited to tracking `IncludedInterface_`; there is no serialization step equivalent to the C# `SaveJson` path that records the interface file and adds it to resources.
- `DMCompilerCpp/src/DMDisassembler.cpp` is empty, whereas the C# repo ships a functional disassembler (`DMDisassembler/`).

## Diagnostics and command-line behaviour
- `WarningCode` in `DMCompilerCpp/include/DMCompiler.h` only covers a minority of codes; numerous emissions used by the C# compiler (invalid include, pragma misuse, type warnings, etc.) have no counterparts.
- Diagnostics emitted during preprocessing and semantic analysis go straight to `std::cerr` rather than through `DMCompiler::Emit`, which means error levels, suppression, and `UniqueEmissions` logic are bypassed.
- Pragmas and optional-error configuration (`SetPragma`, `OptionalErrors` serialization) have no implementation in the C++ port.
- Command-line flags such as `--print-code-tree`, `--store-messages`, or explicit DM version/build override (with integer coercion) are not honoured.

---
Addressing the gaps above will move the C++ compiler closer to feature parity with the proven C# toolchain and make it viable as a drop-in replacement for OpenDream’s current DM build stack.
