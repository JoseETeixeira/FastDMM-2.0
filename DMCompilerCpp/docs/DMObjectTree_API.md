# DMObjectTree API Documentation

## Overview

The `DMObjectTree` is the central type system manager for the DM compiler. It maintains the complete object hierarchy, manages global state, and provides efficient type lookup mechanisms.

## Architecture

```
DMObjectTree
├── AllObjects: vector<unique_ptr<DMObject>>  // All types indexed by ID
├── Globals: vector<DMVariable>               // Global variables
├── StringTable: vector<string>               // Interned strings
├── GlobalProcs: map<string, int>             // Global proc name → ID
└── Resources: set<string>                    // Resource paths
```

Each `DMObject` represents a compile-time type definition:

```
DMObject (/mob/player)
├── Id: 3
├── Path: /mob/player
├── Parent: DMObject* → /mob
├── Variables: map<string, DMVariable>
├── Procs: map<string, vector<int>>
└── GlobalVariables: map<string, int>
```

## Core Concepts

### Type Hierarchy

DM uses a single-inheritance object system:
- All types descend from the root object `/`
- Special types (`/datum`, `/client`, `/list`, etc.) have special parent rules
- Most gameplay types (`/mob`, `/obj`, `/turf`, `/area`) inherit from `/datum`

### Parent Determination Rules

The `DMObjectTree` automatically determines parents based on these rules:

| Type Path | Parent |
|-----------|--------|
| `/` (root) | None (nullptr) |
| `/datum`, `/client`, `/list`, `/savefile`, `/world` | `/` (root) |
| `/mob`, `/obj`, `/turf`, `/area` | `/datum` |
| All others | Immediate parent path |

**Examples:**
- `/mob/player` → parent is `/mob`
- `/mob` → parent is `/datum`
- `/datum` → parent is `/` (root)
- `/obj/item/weapon/sword` → parent is `/obj/item/weapon`

### String Interning

String literals are automatically deduplicated:

```cpp
int id1 = tree.AddString("hello");  // Returns 0
int id2 = tree.AddString("world");  // Returns 1
int id3 = tree.AddString("hello");  // Returns 0 (same as id1!)
```

This saves memory and enables O(1) string equality checks.

## API Reference

### Construction

```cpp
// Create with optional compiler reference for error reporting
DMObjectTree tree(nullptr);

// With compiler
DMObjectTree tree(&myCompiler);
```

### Type Creation and Lookup

#### GetOrCreateDMObject()

Creates a type or returns existing one. **Always creates parent chain.**

```cpp
// Create /mob/player (also creates /, /datum, /mob if needed)
DMObject* player = tree.GetOrCreateDMObject(
    DreamPath(DreamPath::PathType::Absolute, {"mob", "player"})
);

// Returns the same object on subsequent calls
DMObject* player2 = tree.GetOrCreateDMObject(
    DreamPath(DreamPath::PathType::Absolute, {"mob", "player"})
);
assert(player == player2);
```

#### TryGetDMObject()

Lookup without creation. Returns false if type doesn't exist.

```cpp
DMObject* result = nullptr;
if (tree.TryGetDMObject(mobPath, &result)) {
    // Object exists, use result
} else {
    // Object doesn't exist
}
```

#### TryGetTypeId()

Get the numeric ID for a type path.

```cpp
int typeId;
if (tree.TryGetTypeId(mobPath, typeId)) {
    // Use typeId to index into AllObjects
    DMObject* mob = tree.AllObjects[typeId].get();
}
```

#### GetRoot()

Get the root object `/`. Creates it if it doesn't exist.

```cpp
DMObject* root = tree.GetRoot();
assert(root->IsRoot());
assert(root->Parent == nullptr);
```

### Variable Management

#### Adding Instance Variables

```cpp
DMObject* mob = tree.GetOrCreateDMObject(mobPath);

// Create a health variable
DMVariable healthVar(
    std::nullopt,          // No type constraint
    "health",              // Variable name
    false,                 // Not global
    false,                 // Not const
    false,                 // Not final
    false                  // Not tmp
);

mob->Variables["health"] = healthVar;
```

#### Creating Global Variables

```cpp
DMVariable global;
int globalId = tree.CreateGlobal(
    global,                // Output variable
    std::nullopt,          // No type constraint
    "game_version",        // Variable name
    true,                  // Is const
    false,                 // Not final
    DMComplexValueType("num")  // Type info
);

// Access via tree.Globals[globalId]
```

#### Variable Lookup with Inheritance

```cpp
// Search up the inheritance chain
const DMVariable* var = player->GetVariable("health");
if (var != nullptr) {
    // Variable found (either on player, mob, datum, or root)
}

// Check if variable exists
bool hasHealth = player->HasLocalVariable("health");
```

### Proc Management

#### Adding Procs

```cpp
DMObject* mob = tree.GetOrCreateDMObject(mobPath);

// Add a proc (proc ID from future DMProc implementation)
mob->AddProc(100, "Attack");

// Add another overload
mob->AddProc(101, "Attack");

// Check if proc exists
if (mob->HasProc("Attack")) {
    const vector<int>* procIds = mob->GetProcs("Attack");
    // procIds contains [100, 101]
}
```

#### Proc Inheritance

```cpp
// Parent defines Move()
parent->AddProc(200, "Move");

// Child inherits it
assert(child->HasProc("Move"));      // true (inherited)
assert(child->OwnsProc("Move"));     // false (not owned)

// Child overrides
child->AddProc(201, "Move");
assert(child->OwnsProc("Move"));     // true (now owned)

// GetProcs returns child's version first
const vector<int>* procs = child->GetProcs("Move");
assert((*procs)[0] == 201);  // Child's version
```

### String Table Operations

```cpp
// Add strings
int helloId = tree.AddString("hello");
int worldId = tree.AddString("world");

// Access strings
std::string hello = tree.StringTable[helloId];  // "hello"

// Deduplication
int hello2Id = tree.AddString("hello");
assert(helloId == hello2Id);  // Same ID!
```

### Resource Tracking

```cpp
// Track icon files, sounds, etc.
tree.Resources.insert("icons/mob.dmi");
tree.Resources.insert("sound/explosion.ogg");

// Later: package all resources with compiled output
for (const auto& resource : tree.Resources) {
    // Copy resource to output package
}
```

## Usage Patterns

### Basic Compilation Flow

```cpp
// 1. Create tree
DMObjectTree tree(&compiler);

// 2. Parse source files, creating types as encountered
DMObject* mob = tree.GetOrCreateDMObject(mobPath);
DMObject* player = tree.GetOrCreateDMObject(playerPath);

// 3. Add variables
DMVariable nameVar(std::nullopt, "name", false, false, false, false);
mob->Variables["name"] = nameVar;

// 4. Add procs (when DMProc is implemented)
mob->AddProc(procId, "Login");

// 5. After compilation: tree contains complete type system
//    Use for bytecode generation, type checking, etc.
```

### Type Hierarchy Traversal

```cpp
// Walk up the parent chain
DMObject* current = player;
while (current != nullptr) {
    std::cout << current->Path.ToString() << std::endl;
    current = current->Parent;
}
// Output: /mob/player, /mob, /datum, /
```

### Finding All Subtypes

```cpp
// Find all children of /mob
std::vector<DMObject*> mobChildren;
for (auto& obj : tree.AllObjects) {
    DMObject* parent = obj->Parent;
    while (parent != nullptr) {
        if (parent->Path == mobPath) {
            mobChildren.push_back(obj.get());
            break;
        }
        parent = parent->Parent;
    }
}
```

### Variable Resolution

```cpp
// Check all variable locations
const DMVariable* GetCompleteVariable(DMObject* obj, const string& name) {
    // 1. Check overrides first
    auto it = obj->VariableOverrides.find(name);
    if (it != obj->VariableOverrides.end()) {
        return &it->second;
    }
    
    // 2. Check local variables
    auto it2 = obj->Variables.find(name);
    if (it2 != obj->Variables.end()) {
        return &it2->second;
    }
    
    // 3. Check parent chain
    if (obj->Parent != nullptr) {
        return GetCompleteVariable(obj->Parent, name);
    }
    
    return nullptr;
}
```

## Integration with Compiler Pipeline

### Phase 1: Parsing

During parsing, create types as they're encountered:

```cpp
// Encounter: /mob/player/New()
DreamPath playerPath(DreamPath::PathType::Absolute, {"mob", "player"});
DMObject* player = tree.GetOrCreateDMObject(playerPath);

// Add the New proc
player->AddProc(newProcId, "New");
```

### Phase 2: Type Checking

Use the tree to validate types:

```cpp
// Check if assignment is valid
bool IsValidAssignment(DreamPath varType, DreamPath valueType) {
    DMObject* value = nullptr;
    if (!tree.TryGetDMObject(valueType, &value)) {
        return false;  // Type doesn't exist
    }
    
    // Walk up value's hierarchy to see if it matches varType
    while (value != nullptr) {
        if (value->Path == varType) {
            return true;  // Valid assignment
        }
        value = value->Parent;
    }
    return false;
}
```

### Phase 3: Bytecode Generation

Use the tree to generate type tables:

```cpp
// Generate type table for runtime
for (int i = 0; i < tree.AllObjects.size(); i++) {
    DMObject* obj = tree.AllObjects[i].get();
    
    // Emit type entry
    bytecode.WriteInt(obj->Id);
    bytecode.WriteString(obj->Path.ToString());
    bytecode.WriteInt(obj->Parent ? obj->Parent->Id : -1);
    
    // Emit variable table
    bytecode.WriteInt(obj->Variables.size());
    for (const auto& [name, var] : obj->Variables) {
        bytecode.WriteString(name);
        // ... emit variable info
    }
    
    // Emit proc table
    // ... etc.
}
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| GetOrCreateDMObject | O(depth) | Creates parent chain if needed |
| TryGetDMObject | O(1) | Hash table lookup |
| TryGetTypeId | O(1) | Hash table lookup |
| AddString | O(1) average | Hash table for deduplication |
| GetVariable | O(depth) | Walks up parent chain |
| HasProc | O(depth) | Walks up parent chain |
| AddProc | O(1) | Direct insertion |

Where `depth` is the nesting level of the type path (e.g., `/mob/player/admin` has depth 3).

## Thread Safety

**Not thread-safe.** DMObjectTree is designed for single-threaded compilation. If multi-threaded compilation is needed, use one DMObjectTree per thread or add synchronization.

## Memory Management

- `AllObjects` owns all DMObject instances via `unique_ptr`
- DMObject stores raw pointers to parents (lifetime managed by AllObjects)
- When DMObjectTree is destroyed, all objects are automatically cleaned up
- No manual memory management required

## Best Practices

### ✅ DO

- Create DMObjectTree once at the start of compilation
- Use `GetOrCreateDMObject()` during parsing to build the hierarchy
- Use `TryGetDMObject()` for lookups when existence is uncertain
- Intern all string literals via `AddString()`
- Track all resources used via `Resources` set

### ❌ DON'T

- Don't create multiple DMObjectTree instances for the same compilation
- Don't store DMObject pointers beyond the lifetime of the tree
- Don't modify the type hierarchy after type checking phase
- Don't assume a type exists without checking

## Future Enhancements

When `DMProc` class is implemented:

```cpp
// This will become available:
std::vector<std::unique_ptr<DMProc>> AllProcs;

// Usage:
DMProc* proc = tree.AllProcs[procId].get();
```

When AST integration is complete:

```cpp
// DMVariable::Value will become:
std::unique_ptr<DMASTExpression> Value;

// Usage:
DMVariable& var = obj->Variables["health"];
if (var.Value) {
    // Evaluate initial value expression
}
```

## See Also

- `DMObject.h` - Type definition API
- `DMVariable.h` - Variable declaration API  
- `DreamPath.h` - Path handling and manipulation
- `test_objecttree.cpp` - Complete usage examples and test cases
