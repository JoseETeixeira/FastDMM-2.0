# Preprocessor #include Processing

## Overview

The DMCompilerCpp preprocessor now supports recursive #include directive processing, allowing files to include other files which can in turn include more files. This is essential for properly loading the DMStandard library and its type definitions.

## How #include Directives Are Processed

When the preprocessor encounters an `#include` directive:

1. The file path is extracted from the directive (e.g., `#include "Types\Atoms\Obj.dm"`)
2. The path is resolved relative to the file containing the #include directive
3. The included file is recursively preprocessed
4. The tokens from the included file are inserted at the location of the #include directive
5. Processing continues with the parent file

## Include Path Resolution Order

When resolving an #include path, the preprocessor:

1. Checks if the path is absolute - if so, uses it directly
2. Otherwise, resolves the path relative to the directory of the file containing the #include directive
3. Normalizes path separators (both forward slashes and backslashes are supported)
4. Converts the path to an absolute path for tracking

## Circular Include Prevention

The preprocessor tracks all included files using their absolute paths. If a file has already been included, subsequent #include directives for that file are skipped to prevent infinite recursion.

## Path Separator Handling

The preprocessor supports both forward slashes (`/`) and backslashes (`\`) in #include paths:

- `#include "Types/Atoms/Obj.dm"` - Unix-style
- `#include "Types\Atoms\Obj.dm"` - Windows-style

Both formats work correctly on all platforms.

## String Escape Sequence Handling

The lexer properly handles backslashes in string literals:

- Recognized escape sequences: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`
- Unrecognized escape sequences: The backslash is preserved (e.g., `\T` becomes `\T`, not `T`)

This ensures that Windows file paths with backslashes work correctly in #include directives.

## Example Usage

### Simple Include

```dm
// main.dm
proc/main()
    return 1

#include "helper.dm"
```

### Nested Includes

```dm
// _Standard.dm
#include "Defines.dm"
#include "Types\Atoms\_Atom.dm"
#include "Types\Atoms\Obj.dm"

// Types\Atoms\Obj.dm
/obj
    parent_type = /atom/movable
    layer = OBJ_LAYER
```

### DMStandard Library

The DMStandard library uses #include directives extensively:

```dm
// DMStandard/_Standard.dm
#include "Defines.dm"
#include "_Globals.dm"
#include "Types\AList.dm"
#include "Types\Client.dm"
#include "Types\Datum.dm"
// ... many more includes
#include "Types\Atoms\_Atom.dm"
#include "Types\Atoms\Area.dm"
#include "Types\Atoms\Mob.dm"
#include "Types\Atoms\Movable.dm"
#include "Types\Atoms\Obj.dm"
#include "Types\Atoms\Turf.dm"
```

## Implementation Details

### Key Functions

- `DMPreprocessor::Preprocess(path)` - Main entry point, calls PreprocessFile
- `DMPreprocessor::PreprocessFile(path, location)` - Recursively preprocesses a file
- `DMPreprocessor::HandleIncludeDirective(token, result)` - Handles #include directives
- `DMPreprocessor::ResolvePath(path, currentFile)` - Resolves relative paths

### Token Flow

1. Lexer generates `TokenType::DM_Preproc_Include` for `#include` directives
2. Preprocessor recognizes the token and calls `HandleIncludeDirective`
3. `HandleIncludeDirective` extracts the file path and calls `PreprocessFile`
4. `PreprocessFile` returns tokens which are inserted into the parent file's token stream

## Troubleshooting

### File Not Found Errors

If you see "File not found" errors:

1. Check that the file path is correct relative to the including file
2. Verify that the file exists in the expected location
3. Check for typos in the file path
4. Ensure path separators are correct for your platform

### Circular Include Warnings

If you see "Skipping already included file" messages (in verbose mode):

- This is normal behavior to prevent infinite recursion
- The file has already been included earlier in the compilation
- No action is needed

### Token Count Verification

To verify that #include directives are being processed:

1. Run the compiler with `--verbose` flag
2. Check the "DMStandard tokens" count - should be ~9500+ tokens with all includes
3. Compare to the count without includes (~3800 tokens)

## Performance Considerations

- Each file is only preprocessed once (tracked by absolute path)
- Included files are processed immediately when encountered
- No caching of preprocessed tokens (each compilation starts fresh)
- Typical DMStandard preprocessing takes <100ms
