# Goto Forward Reference Resolution

## Overview

This document describes the implementation of forward reference resolution for `goto` statements in the DMCompilerCpp project.

## Problem

In DM (Dream Maker) language, `goto` statements can reference labels that are defined later in the code (forward references):

```dm
proc/test()
    goto end_label    // Forward reference - label not yet defined
    world << "This is skipped"
end_label:
    world << "Jumped to end"
```

The compiler needs to handle both:

- **Backward references**: `goto` jumps to a label defined earlier
- **Forward references**: `goto` jumps to a label defined later

## Solution

### Architecture

The solution uses a two-pass approach:

1. **First Pass (Compilation)**:

   - When a `goto` statement is encountered, check if the label exists
   - If not, create a placeholder label and track it as a forward reference
   - Store the placeholder in `NamedLabels_` so subsequent gotos to the same label reuse it

2. **Second Pass (Finalization)**:
   - After all statements are compiled, call `Finalize()`
   - Check that all forward references have been resolved (labels were defined)
   - Report errors for any undefined labels

### Data Structures

#### ForwardReference Structure

```cpp
struct ForwardReference {
    std::string LabelName;      // The name of the label being referenced
    size_t BytecodePosition;    // Position in bytecode where the jump is
    std::string PlaceholderLabel; // The placeholder label that was created
};
```

#### Tracking Collections

- `NamedLabels_`: Maps label names to their generated label IDs
- `ForwardReferences_`: Vector of unresolved forward references

### Implementation Details

#### CompileGoto()

```cpp
bool DMStatementCompiler::CompileGoto(DMASTProcStatementGoto* stmt) {
    std::string labelName = stmt->Label->Identifier;

    auto it = NamedLabels_.find(labelName);
    if (it == NamedLabels_.end()) {
        // Forward reference - create placeholder
        std::string placeholderLabel = NewLabel();
        EmitJump(placeholderLabel);

        // Store for reuse by subsequent gotos
        NamedLabels_[labelName] = placeholderLabel;

        // Track for validation
        ForwardReferences_.push_back({labelName, Writer_->GetPosition(), placeholderLabel});
    } else {
        // Backward reference or subsequent forward reference
        EmitJump(it->second);
    }

    return true;
}
```

#### CompileLabel()

```cpp
bool DMStatementCompiler::CompileLabel(DMASTProcStatementLabel* stmt) {
    std::string generatedLabel;

    // Check if there's a forward reference to this label
    bool hasForwardRef = false;
    for (const auto& forwardRef : ForwardReferences_) {
        if (forwardRef.LabelName == stmt->Name) {
            // Reuse the placeholder label
            generatedLabel = forwardRef.PlaceholderLabel;
            hasForwardRef = true;
            break;
        }
    }

    if (!hasForwardRef) {
        generatedLabel = NewLabel();
    }

    NamedLabels_[stmt->Name] = generatedLabel;
    EmitLabel(generatedLabel);

    return true;
}
```

#### Finalize()

```cpp
bool DMStatementCompiler::Finalize() {
    bool allResolved = true;

    for (const auto& forwardRef : ForwardReferences_) {
        auto it = NamedLabels_.find(forwardRef.LabelName);

        if (it == NamedLabels_.end()) {
            // Label was never defined - error
            Compiler_->ForcedError(
                Location::Internal,
                "Undefined label '" + forwardRef.LabelName + "' referenced by goto statement"
            );
            allResolved = false;
        }
    }

    ForwardReferences_.clear();
    return allResolved;
}
```

## Usage

When compiling a proc body:

```cpp
DMStatementCompiler stmtCompiler(&compiler, &proc, &writer, &exprCompiler);

// Compile all statements
for (auto& stmt : procBody->Statements) {
    stmtCompiler.CompileStatement(stmt.get());
}

// Resolve forward references
if (!stmtCompiler.Finalize()) {
    // Handle compilation error
}

// Finalize bytecode writer to resolve jumps
writer.Finalize();
```

## Test Coverage

The implementation includes comprehensive tests in `test_goto_forward_ref.cpp`:

1. **TestGotoForwardReference**: Basic forward reference
2. **TestGotoBackwardReference**: Backward reference (label before goto)
3. **TestGotoUndefinedLabel**: Error detection for undefined labels
4. **TestMultipleGotosToSameLabel**: Multiple gotos to the same forward label

All tests pass successfully.

## Future Enhancements

- Add source location tracking for better error messages
- Support for computed goto (if DM supports it)
- Optimization: detect unreachable code after goto

## Related Files

- `DMCompilerCpp/include/DMStatementCompiler.h`
- `DMCompilerCpp/src/DMStatementCompiler.cpp`
- `DMCompilerCpp/tests/test_goto_forward_ref.cpp`
