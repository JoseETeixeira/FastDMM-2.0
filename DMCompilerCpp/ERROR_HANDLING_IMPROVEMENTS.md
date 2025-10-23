# Error Handling and Reporting Improvements

This document summarizes the improvements made to error handling and reporting in DMCompilerCpp as part of Task 17.

## Overview

Enhanced error messages throughout the compiler to provide better context, location information, and distinguish between different types of failures. All error messages now include:

- Precise location information (file, line, column)
- Context about where the error occurred (proc name, object path)
- Specific details about what was expected vs. what was found
- Clear distinction between different lookup types (local/field/global for variables)

## Changes Made

### 1. Expression Compiler (DMExpressionCompiler.cpp)

#### Type Resolution Errors

- **Before**: `Type not found: /path/to/type`
- **After**: `Type path '/path/to/type' could not be resolved in proc /obj/item/proc_name`
- Includes full context about where the type was referenced

#### Variable Resolution Errors

- **Before**: `Unknown identifier 'varname'`
- **After**: `Unknown identifier 'varname' at file.dm:10:5 in proc /obj/item/proc_name (not found as local variable, object field, or global variable)`
- Clearly distinguishes the three lookup scopes that were checked
- Includes precise location and proc context

#### Variable Assignment Errors

- **Before**: `Variable 'varname' not found in current scope`
- **After**: `Variable 'varname' not found for assignment in proc /obj/item/proc_name (not found as local variable, object field, or global variable)`
- Specifies this is an assignment context
- Lists all scopes that were checked

#### Proc Resolution Errors

- **Before**: `Unknown global proc 'procname()'`
- **After**: `Unknown proc 'procname()' at file.dm:15:10 in proc /obj/item/caller (not found as global proc or member proc)`
- Includes location and calling context
- Clarifies both global and member proc lookups were attempted

#### Built-in Function Errors

Enhanced error messages for all built-in functions with consistent format:

- `pick()`: "pick() requires at least 1 argument at file.dm:20:5 in proc /obj/item/test"
- `locate()`: "locate() requires 1, 2, or 3 arguments (found 0) at file.dm:21:5 in proc /obj/item/test"
- `input()`: "input() takes at most 4 positional arguments (found 5) at file.dm:22:5 in proc /obj/item/test"
- `rgb()`: "rgb() requires 3-5 arguments (found 2) at file.dm:23:5 in proc /obj/item/test"
- `prob()`: "prob() requires exactly 1 argument (found 0) at file.dm:24:5 in proc /obj/item/test"
- `istype()`: "istype() requires exactly 2 arguments (found 1) at file.dm:25:5 in proc /obj/item/test"

All built-in function errors now include:

- Function name
- Expected argument count
- Actual argument count (when applicable)
- Precise location
- Proc context

### 2. Statement Compiler (DMStatementCompiler.cpp)

#### Unknown Statement Type

- **Before**: `Unknown statement type`
- **After**: `Unknown statement type at file.dm:30:5 in proc /obj/item/proc_name`
- Includes location and proc context

#### Control Flow Errors

- **Before**: `break statement outside of loop`
- **After**: `break statement outside of loop at file.dm:35:4 in proc /obj/item/proc_name`
- Includes precise location and proc context

- **Before**: `continue statement outside of loop`
- **After**: `continue statement outside of loop at file.dm:40:4 in proc /obj/item/proc_name`
- Includes precise location and proc context

#### Variable Declaration Errors

- **Before**: `Invalid variable declaration`
- **After**: `Invalid variable declaration at file.dm:45:4 in proc /obj/item/proc_name`
- Includes location and proc context

- **Before**: `Variable 'varname' already declared`
- **After**: `Variable 'varname' already declared at file.dm:46:4 in proc /obj/item/proc_name`
- Includes location and proc context

### 3. Preprocessor (DMPreprocessor.cpp)

#### Macro Expansion Errors

- **Before**: Silent failure (empty vector returned)
- **After**: `Error at file.dm:5:10: Macro expansion failed - expected 2 arguments but got 1`
- Reports argument count mismatches with location

#### Directive Errors

##### #if Directive

- **Before**: `Error: Expression expected for #if at file.dm:10:0`
- **After**: `Error at file.dm:10:0: #if directive requires an expression`
- More concise and clear

##### #ifdef Directive

- **Before**: `Error: Expected identifier after #ifdef at file.dm:15:0`
- **After**: `Error at file.dm:15:0: #ifdef directive requires a macro name (identifier), got token type 83`
- Specifies what was expected and what was received

##### #ifndef Directive

- **Before**: `Error: Expected identifier after #ifndef at file.dm:20:0`
- **After**: `Error at file.dm:20:0: #ifndef directive requires a macro name (identifier), got token type 83`
- Specifies what was expected and what was received

##### #undef Directive

- **Before**: `Error: Expected identifier after #undef at file.dm:25:0`
- **After**: `Error at file.dm:25:0: #undef directive requires a macro name (identifier), got token type 3`
- Specifies what was expected and what was received

##### #elif Directive

- **Before**: `Error: Unexpected #elif at file.dm:30:0`
- **After**: `Error at file.dm:30:0: #elif directive without matching #if, #ifdef, or #ifndef`
- Clarifies what directives can precede #elif

##### #else Directive

- **Before**: `Error: Unexpected #else at file.dm:35:0`
- **After**: `Error at file.dm:35:0: #else directive without matching #if, #ifdef, or #ifndef`
- Clarifies what directives can precede #else

##### #endif Directive

- **Before**: `Error: Unexpected #endif at file.dm:40:0`
- **After**: `Error at file.dm:40:0: #endif directive without matching #if, #ifdef, or #ifndef`
- Clarifies what directives can precede #endif

##### #include Directive

- **Before**: `Error: Invalid include path at file.dm:45:0 - expected string or identifier, got token type 5`
- **After**: `Error at file.dm:45:0: #include directive requires a file path (string or identifier), got token type 5`
- More concise and clear about requirements

## Error Message Format Standards

All error messages now follow these standards:

1. **Location First**: Always start with location information when available

   - Format: `file.dm:line:column` or `at file.dm:line:column`

2. **Context Information**: Include proc and object context when available

   - Format: `in proc /path/to/object/proc_name`

3. **Specific Details**: Provide specific information about what failed

   - What was expected
   - What was found
   - What scopes were checked (for lookups)

4. **Actionable Information**: Help the developer understand how to fix the issue
   - List all places that were checked
   - Specify exact requirements (argument counts, types, etc.)

## Testing

Created test files to verify improved error messages:

- `testdata/error_handling_test.dm` - Tests variable, proc, and statement errors
- `testdata/error_handling_test2.dm` - Tests type path resolution errors
- `testdata/preprocessor_error_test.dm` - Tests preprocessor directive errors

All tests produce clear, informative error messages with proper context.

## Requirements Satisfied

This implementation satisfies all requirements from Task 17:

- ✅ 7.1: Type resolution failures include location and path name
- ✅ 7.2: Variable resolution failures distinguish between lookup types (local/field/global)
- ✅ 7.3: Proc resolution failures include helpful context
- ✅ 7.4: Macro expansion failures include location info
- ✅ 7.5: Preprocessor expression evaluation failures include error handling

## Future Improvements

Potential areas for further enhancement:

1. Add "did you mean?" suggestions for typos in identifiers
2. Include more context about available alternatives (e.g., list similar variable names)
3. Add color coding to error messages for better readability
4. Group related errors together
5. Add error codes for programmatic error handling
