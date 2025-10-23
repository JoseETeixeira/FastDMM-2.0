# Identifier Lookup Fix - TODO

## Problem
When compiling expressions, identifiers like `mob`, `obj`, `player`, `item` are not being resolved even though the corresponding types `/mob`, `/obj`, `/mob/player`, `/obj/item` are correctly defined in the object tree.

## Root Cause
The `CompileIdentifier()` function only checks:
1. Local variables/parameters
2. Special identifiers (src, usr, args, world)
3. Member variables of the owning object

It does NOT check:
4. ❌ Global types in the object tree
5. ❌ Global variables
6. ❌ Global proc names

## Solution Plan

### Phase 1: Add Type Path Lookup (CRITICAL - Do First)
When an identifier is not found as a local/member variable, check if it's a type name in the object tree:
- Convert identifier to absolute path (e.g., "mob" → "/mob")
- Look up in object tree using TryGetTypeId()
- If found, emit PushType opcode with the type ID

### Phase 2: Add Global Variable Lookup
- Implement global variable registry in DMCompiler
- Check if identifier is a global variable
- Emit appropriate opcode to push global variable value

### Phase 3: Add Global Proc Lookup
- Check if identifier refers to a global proc name
- Handle proc references (e.g., for callback purposes)

### Phase 4: Match C# Implementation Exactly
This requires comparing:
- DMCompiler/Compiler/DM/DMExpression.cs
- DMCompiler/Compiler/DM/Expressions/*.cs
- Particularly DMIdentifier and how it resolves

## Implementation

See below for the immediate fix to Phase 1.
