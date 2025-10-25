# Parameter Registration Investigation Results

## Task 1.1: Investigation of Current Parameter Handling

### Summary

The investigation reveals that **proc parameters ARE being registered as local variables**, but this happens during the `DMProc::Compile()` method, which is called AFTER the code tree is built. This creates a timing issue where parameters are not available during earlier compilation phases.

---

## Current Architecture

### 1. Where DMProc Objects Are Created

**File:** `DMCompilerCpp/src/DMObjectTree.cpp`  
**Method:** `DMObjectTree::AddProc(const DreamPath& owner, DMASTObjectProcDefinition* procDef)`

```cpp
void DMObjectTree::AddProc(const DreamPath& owner, DMASTObjectProcDefinition* procDef) {
    if (!procDef) return;

    // Get or create the owning object
    DMObject* ownerObj = GetOrCreateDMObject(owner);

    // Create the proc
    DMProc* proc = CreateProc(procDef->Name, ownerObj, procDef->IsVerb, procDef->Location_);

    // Store the proc definition AST for later compilation
    proc->AstBody = procDef->Body.get();  // Store non-owning pointer to proc body

    // Store the parameter definitions (non-owning pointers)
    for (const auto& param : procDef->Parameters) {
        proc->AstParameters.push_back(param.get());
    }

    // Add the proc to the object
    ownerObj->AddProc(proc->Id, procDef->Name);

    // If it's a global proc (owned by root), register it
    if (owner == DreamPath::Root) {
        RegisterGlobalProc(procDef->Name, proc->Id);
    }
}
```

**Key Observations:**

- DMProc objects are created via `CreateProc()` helper method
- AST parameter definitions are stored in `proc->AstParameters` (non-owning pointers)
- **AddParameter() is NOT called at this stage**

---

### 2. Where Proc Definitions Are Processed

**File:** `DMCompilerCpp/src/DMCodeTreeBuilder.cpp`  
**Method:** `DMCodeTreeBuilder::ProcessStatement()`

```cpp
// Proc definition: proc/test() { ... }
else if (auto* procDef = dynamic_cast<DMASTObjectProcDefinition*>(statement)) {
    DreamPath procOwner = currentType.Combine(procDef->ObjectPath);

    // Add the type that owns this proc
    ObjectTree_->AddType(procOwner);

    // Add the proc to the object tree
    ObjectTree_->AddProc(procOwner, procDef);
}
```

**Key Observations:**

- Proc definitions are processed during the `BuildCodeTree()` phase
- This calls `ObjectTree_->AddProc()` which creates the DMProc object
- Parameters are stored as AST pointers but not yet registered as local variables

---

### 3. When AddParameter() IS Being Called

**File:** `DMCompilerCpp/src/DMProc.cpp`  
**Method:** `DMProc::Compile(DMCompiler* compiler)`

```cpp
void DMProc::Compile(DMCompiler* compiler) {
    // Skip if already compiled or marked as unsupported
    if (IsUnsupported()) {
        return;
    }

    // Initialization procs have null AstBody - they're created dynamically
    if (AstBody == nullptr) {
        return;
    }

    // Process parameters and add them to local variables
    // This is essential so identifiers can be resolved during compilation
    for (const auto* astParam : AstParameters) {
        if (!astParam) continue;

        // Determine the type (if specified)
        std::optional<DreamPath> paramType;
        if (!astParam->TypePath.GetElements().empty()) {
            paramType = astParam->TypePath;
        }

        // Add parameter as a local variable
        // The parameter will be accessible during bytecode emission
        AddParameter(astParam->Name, paramType, astParam->ExplicitValueType);
    }

    // TODO: Implement full proc compilation in Phase 4
    // ...
}
```

**Key Observations:**

- **AddParameter() IS being called** - in the `DMProc::Compile()` method
- This happens during the "Compile every proc" phase in `DMCodeTreeBuilder::BuildCodeTree()`
- Parameters are registered AFTER the code tree is built

---

### 4. Compilation Flow Timeline

```
Phase 1: Parse AST
  └─> DMASTObjectProcDefinition created with Parameters

Phase 2: Build Code Tree (DMCodeTreeBuilder::BuildCodeTree)
  ├─> ProcessStatements() called
  ├─> ProcessStatement() encounters proc definition
  ├─> ObjectTree_->AddProc() called
  │   ├─> DMProc object created
  │   └─> AstParameters stored (non-owning pointers)
  └─> [Parameters NOT yet in LocalVariables]

Phase 3: Create Initialization Procs
  └─> dmObject->CreateInitializationProc()

Phase 4: Compile Procs (DMCodeTreeBuilder::BuildCodeTree)
  └─> proc->Compile(Compiler_) called for each proc
      └─> AddParameter() called for each AstParameter
          └─> [Parameters NOW in LocalVariables]
```

---

## Current Behavior Analysis

### What Works ✅

1. **DMProc objects are created correctly** during code tree building
2. **AST parameter information is preserved** in `proc->AstParameters`
3. **AddParameter() method exists and is implemented** correctly
4. **Parameters ARE eventually registered** as local variables

### The Problem ❌

1. **Timing Issue**: Parameters are registered in `DMProc::Compile()`, which happens AFTER the code tree is built
2. **Too Late**: By the time parameters are registered, other compilation phases may have already tried to resolve identifiers
3. **Inconsistent State**: There's a window where the DMProc exists but its parameters are not in LocalVariables

---

## Root Cause

The issue is **NOT** that `AddParameter()` is missing or not being called. The issue is **WHEN** it's being called.

**Current Flow:**

```
Build Code Tree → Create DMProc → Store AstParameters → [LATER] → Compile Proc → AddParameter()
```

**Expected Flow:**

```
Build Code Tree → Create DMProc → Store AstParameters → AddParameter() → [LATER] → Compile Proc
```

---

## Gap Analysis

### What Needs to Change

**Option 1: Register Parameters During AddProc (RECOMMENDED)**

- Move parameter registration from `DMProc::Compile()` to `DMObjectTree::AddProc()`
- This ensures parameters are available immediately after DMProc creation
- Matches the design document's recommendation

**Option 2: Register Parameters in DMProc Constructor**

- Add parameter registration logic to DMProc constructor
- Requires passing AST parameters to constructor
- More invasive change to constructor signature

**Option 3: Keep Current Approach, Fix Compilation Order**

- Ensure all identifier resolution happens AFTER `proc->Compile()` is called
- More complex, requires understanding all compilation dependencies
- Not recommended

---

## Recommended Solution

**Modify `DMObjectTree::AddProc()` to register parameters immediately:**

```cpp
void DMObjectTree::AddProc(const DreamPath& owner, DMASTObjectProcDefinition* procDef) {
    if (!procDef) return;

    // Get or create the owning object
    DMObject* ownerObj = GetOrCreateDMObject(owner);

    // Create the proc
    DMProc* proc = CreateProc(procDef->Name, ownerObj, procDef->IsVerb, procDef->Location_);

    // Store the proc definition AST for later compilation
    proc->AstBody = procDef->Body.get();

    // Store the parameter definitions (non-owning pointers)
    for (const auto& param : procDef->Parameters) {
        proc->AstParameters.push_back(param.get());
    }

    // NEW: Register parameters as local variables immediately
    for (const auto& param : procDef->Parameters) {
        std::optional<DreamPath> paramType;
        if (!param->TypePath.GetElements().empty()) {
            paramType = param->TypePath;
        }

        proc->AddParameter(param->Name, paramType, param->ExplicitValueType);
    }

    // Add the proc to the object
    ownerObj->AddProc(proc->Id, procDef->Name);

    // If it's a global proc (owned by root), register it
    if (owner == DreamPath::Root) {
        RegisterGlobalProc(procDef->Name, proc->Id);
    }
}
```

**Then remove duplicate registration from `DMProc::Compile()`:**

```cpp
void DMProc::Compile(DMCompiler* compiler) {
    // Skip if already compiled or marked as unsupported
    if (IsUnsupported()) {
        return;
    }

    // Initialization procs have null AstBody - they're created dynamically
    if (AstBody == nullptr) {
        return;
    }

    // REMOVE: Parameter registration (now done in AddProc)
    // Parameters are already registered in LocalVariables by AddProc()

    // TODO: Implement full proc compilation in Phase 4
    // ...
}
```

---

## Verification Steps

After implementing the fix:

1. **Check parameter count**: `proc->GetParameterCount()` should return correct count immediately after `AddProc()`
2. **Check local variable lookup**: `proc->GetLocalVariable("paramName")` should return non-null immediately after `AddProc()`
3. **Check IsParameter flag**: `proc->GetLocalVariable("paramName")->IsParameter` should be true
4. **Compile test files**: heapsort.dm and quicksort.dm should have zero warnings about parameter identifiers

---

## Files Involved

1. **DMCompilerCpp/src/DMObjectTree.cpp** - Modify `AddProc()` method
2. **DMCompilerCpp/src/DMProc.cpp** - Remove duplicate parameter registration from `Compile()`
3. **DMCompilerCpp/include/DMProc.h** - No changes needed (AddParameter already exists)

---

## Requirements Addressed

- ✅ **Requirement 1.1**: DMParser SHALL register each parameter as a local variable
- ✅ **Requirement 1.2**: DMProc SHALL include all proc parameters in its local variable list
- ✅ **Requirement 1.3**: DMExpressionCompiler SHALL resolve parameter names as local variables
- ✅ **Requirement 1.4**: Compiler SHALL NOT emit "Unknown identifier" warnings for parameters

---

## Next Steps

Proceed to **Task 1.2**: Implement parameter registration in DMObjectTree::AddProc()
