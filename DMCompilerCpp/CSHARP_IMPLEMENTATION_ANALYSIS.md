# C# Implementation Analysis - Complete Reference

## Architecture Overview

The C# DM compiler follows a clean multi-stage pipeline:

1. **Preprocessing** (DMPreprocessor) - Handles #include, #define, #if, etc.
2. **Lexing** (DMLexer) - Converts preprocessor tokens to DM tokens
3. **Parsing** (DMParser) - Builds Abstract Syntax Tree (AST)
4. **Building** (DMObjectTree, DMProcBuilder, DMExpressionBuilder) - Compiles AST to bytecode

## Stage 1: Preprocessor (DMPreprocessor.cs)

### Responsibilities
- Process #include directives (recursively)
- Expand #define macros
- Evaluate #if/#ifdef/#ifndef directives
- Handle string interpolation

### Key Components

#### Macro System
```csharp
private readonly Dictionary<string, DMMacro> _defines;
```

- Stores all #define macros
- Built-in macros: `__LINE__`, `__FILE__`, `DM_VERSION`, `DM_BUILD`
- User defines added via #define directive

#### Macro Expansion
```csharp
private bool TryMacro(Token token) {
    if (!_defines.TryGetValue(token.Text, out DMMacro? macro))
        return false;
    
    List<Token>? expandedTokens = macro.Expand(...);
    // Push expanded tokens back for re-processing
    for (int i = expandedTokens.Count - 1; i >= 0; i--) {
        PushToken(expandedTokens[i]);
    }
    return true;
}
```

**Critical Insight**: Macros are expanded DURING preprocessing, so identifiers like `NORTH` become `1` before the lexer ever sees them!

### Output
- Stream of preprocessor tokens (`DM_Preproc_*` types)
- All #define macros already expanded
- All #include files inlined
- All #if blocks resolved

## Stage 2: Lexer (DMLexer.cs)

### Responsibilities
- Convert preprocessor tokens to DM tokens
- Recognize keywords (null, break, if, etc.)
- Handle indentation (convert to INDENT/DEDENT tokens)
- Combine multi-token identifiers (escaped identifiers)

### Keywords Dictionary
```csharp
private static readonly Dictionary<string, TokenType> Keywords = new(25) {
    { "null", TokenType.DM_Null },
    { "break", TokenType.DM_Break },
    { "continue", TokenType.DM_Continue },
    { "if", TokenType.DM_If },
    { "else", TokenType.DM_Else },
    { "for", TokenType.DM_For },
    { "switch", TokenType.DM_Switch },
    { "while", TokenType.DM_While },
    { "do", TokenType.DM_Do },
    { "var", TokenType.DM_Var },
    { "proc", TokenType.DM_Proc },
    { "new", TokenType.DM_New },
    { "del", TokenType.DM_Del },
    { "return", TokenType.DM_Return },
    { "in", TokenType.DM_In },
    { "to", TokenType.DM_To },
    { "as", TokenType.DM_As },
    { "set", TokenType.DM_Set },
    { "call", TokenType.DM_Call },
    { "call_ext", TokenType.DM_Call},
    { "spawn", TokenType.DM_Spawn },
    { "goto", TokenType.DM_Goto },
    { "step", TokenType.DM_Step },
    { "try", TokenType.DM_Try },
    { "catch", TokenType.DM_Catch },
    { "throw", TokenType.DM_Throw }
};
```

**Critical Insight**: Keywords like `null` are resolved in the LEXER, not the parser!

### Identifier Processing
```csharp
case TokenType.DM_Preproc_Identifier: {
    // Combine multiple tokens for escaped identifiers
    do {
        TokenTextBuilder.Append(GetCurrent().Text);
    } while (ValidIdentifierComponents.Contains(Advance().Type));
    
    var identifierText = TokenTextBuilder.ToString();
    
    // Look up in keywords dictionary
    var tokenType = Keywords.GetValueOrDefault(identifierText, TokenType.DM_Identifier);
    
    token = CreateToken(tokenType, identifierText, firstTokenLocation);
    break;
}
```

**Critical Insight**: After preprocessor expansion and keyword lookup, only "true" identifiers remain (variables, types, procs).

### Output
- Stream of DM tokens (`DM_*` types)
- Keywords converted to specific token types (DM_Null, DM_Break, etc.)
- Identifiers that aren't keywords remain as DM_Identifier
- Numbers parsed to DM_Integer or DM_Float with values

## Stage 3: Parser (DMParser.cs)

### Responsibilities
- Build Abstract Syntax Tree (AST) from tokens
- Validate syntax
- Create expression/statement nodes

### Primary Expression Parsing
```csharp
private DMASTExpression? ExpressionPrimary(bool allowParentheses = true) {
    // 1. Parenthesized expressions
    if (allowParentheses && Check(TokenType.DM_LeftParenthesis)) { ... }
    
    // 2. Var declarations (var/path)
    if (token.Type == TokenType.DM_Var && _allowVarDeclExpression) { ... }
    
    // 3. Constants (integers, floats, strings, null, resources)
    if (Constant() is { } constant)
        return constant;
    
    // 4. Paths (/mob/player, /obj/item)
    if (Path(true) is { } path) {
        return new DMASTConstantPath(loc, path);
    }
    
    // 5. Identifiers (variables, types, procs)
    if (Identifier() is { } identifier)
        return identifier;
    
    // 6. Callables (proc references)
    if ((DMASTExpression?)Callable() is { } callable)
        return callable;
    
    // 7. Scope identifiers (::global)
    if (Check(TokenType.DM_DoubleColon))
        return ParseScopeIdentifier(null);
    
    // 8. call()() expressions
    if (Check(TokenType.DM_Call)) { ... }
    
    return null;
}
```

### Constant Parsing
```csharp
protected DMASTExpression? Constant() {
    Token constantToken = Current();
    
    switch (constantToken.Type) {
        case TokenType.DM_Integer:
            Advance();
            return new DMASTConstantInteger(constantToken.Location, constantToken.ValueAsInt());
        
        case TokenType.DM_Float:
            Advance();
            return new DMASTConstantFloat(constantToken.Location, constantToken.ValueAsFloat());
        
        case TokenType.DM_Resource:
            Advance();
            return new DMASTConstantResource(constantToken.Location, constantToken.ValueAsString());
        
        case TokenType.DM_Null:  // <-- KEYWORD RECOGNIZED HERE
            Advance();
            return new DMASTConstantNull(constantToken.Location);
        
        case TokenType.DM_RawString:
            Advance();
            return new DMASTConstantString(constantToken.Location, constantToken.ValueAsString());
        
        case TokenType.DM_ConstantString:
        case TokenType.DM_StringBegin:
            return ExpressionFromString();
        
        default:
            return null;
    }
}
```

**Critical Insight**: `null` is handled as a constant because the lexer already converted it from DM_Identifier to DM_Null!

### Identifier Parsing
```csharp
private DMASTIdentifier? Identifier() {
    if (!Check(IdentifierTypes))
        return null;
    
    return new DMASTIdentifier(PreviousLocation, Previous().Text);
}
```

**Critical Insight**: Parser just creates identifier nodes - no resolution happens here!

### Output
- Abstract Syntax Tree (AST)
- DMASTFile → DMASTBlockInner → DMASTStatements → DMASTExpressions
- Expressions include: DMASTIdentifier, DMASTConstantInteger, DMASTConstantPath, etc.

## Stage 4: Expression Building (DMExpressionBuilder.cs)

### Responsibilities
- Convert AST expressions to DMExpression objects
- Resolve identifiers to variables/fields/globals/specials
- Emit warnings for unknown identifiers

### Identifier Resolution Order (BuildIdentifier)
```csharp
private DMExpression BuildIdentifier(DMASTIdentifier identifier, DreamPath? inferredPath = null) {
    var name = identifier.Identifier;
    
    // 1. Local variables
    var localVar = ctx.Proc?.GetLocalVariable(name);
    if (localVar is not null)
        return new Local(identifier.Location, localVar);
    
    // 2. Instance fields
    var field = ctx.Type.GetVariable(name);
    if (field != null && (scopeMode == Normal || field.IsConst))
        return new Field(identifier.Location, field, field.ValType);
    
    // 3. Global variables
    var globalId = ctx.Proc?.GetGlobalVariableId(name) ?? ctx.Type.GetGlobalVariableId(name);
    if (globalId != null) {
        var globalVar = ObjectTree.Globals[globalId.Value];
        var global = new GlobalField(identifier.Location, globalVar.Type, globalId.Value, globalVar.ValType);
        
        // Soft reserved keywords DO NOT override globals
        if (name is not ("usr" or "src" or "args" or "world" or "global" or "callee" or "caller"))
            return global;
    }
    
    // 4. Special identifiers
    switch (name) {
        case "src": return new Src(identifier.Location, ctx.Type.Path);
        case "usr": return new Usr(identifier.Location);
        case "args": return new Args(identifier.Location);
        case "callee": return new Callee(identifier.Location);
        case "caller": return new Caller(identifier.Location);
        case "world": return new World(identifier.Location);
        case "__TYPE__": return new ProcOwnerType(identifier.Location, ctx.Type);
        case "__IMPLIED_TYPE__": return BuildPath(identifier.Location, inferredPath.Value);
        case "__PROC__": return new ConstantProcReference(identifier.Location, ...);
        case "global": return new Global(identifier.Location);
        
        // 5. Unknown identifier
        default:
            return UnknownIdentifier(identifier.Location, name);
    }
}
```

### Unknown Identifier Handling
```csharp
private UnknownReference UnknownIdentifier(Location location, string identifier) =>
    UnknownReference(location, $"Unknown identifier \"{identifier}\"");
```

```csharp
internal sealed class UnknownReference(Location location, string message) : DMExpression(location) {
    public void EmitCompilerError(DMCompiler compiler) {
        compiler.Emit(WarningCode.ItemDoesntExist, Location, message);
    }
    
    public override void EmitPushValue(ExpressionContext ctx) {
        // Runtime error
        ctx.Proc.PushString("Encountered an unknown reference expression (compiler bug!)");
        ctx.Proc.Throw();
    }
}
```

**Critical Insight**: Unknown identifiers are NOT resolved as type paths! They become `UnknownReference` expressions that emit warnings later.

## Key Findings: Why C# Works

### 1. Preprocessor Expands All Macros ✅
- `NORTH`, `SOUTH`, `EAST`, `WEST` are `#define`d in DMStandard/Defines.dm
- Preprocessor expands them to integers (`1`, `2`, `4`, `8`) BEFORE lexing
- Lexer never sees them as identifiers

### 2. Keywords Resolved in Lexer ✅
- `null`, `break`, `if`, etc. converted to specific token types
- Parser never treats them as identifiers
- Parser `Constant()` function handles `TokenType.DM_Null` specially

### 3. No Type Path Resolution for Bare Identifiers ✅
- Identifiers like `mob` do NOT get resolved to `/mob` type paths
- If `mob` isn't a variable/field/global, it becomes `UnknownReference`
- **Type paths must be explicit**: Use `/mob` not `mob`

### 4. Clean Separation of Concerns ✅
- Preprocessor: Macro expansion, includes, conditional compilation
- Lexer: Tokenization, keyword recognition
- Parser: Syntax validation, AST construction
- Builder: Semantic analysis, identifier resolution, bytecode emission

## Critical Differences from C++ Implementation

| Aspect | C# Implementation | C++ Implementation | Impact |
|--------|------------------|-------------------|--------|
| Macro Expansion | Full expansion in preprocessor | **NOT WORKING** | Constants become identifiers ❌ |
| Keyword Recognition | Lexer converts to token types | **NOT IMPLEMENTED** | `null` treated as identifier ❌ |
| Identifier Resolution | Only vars/fields/globals | Added type path lookup | Different semantics ⚠️ |
| Unknown Identifiers | UnknownReference (deferred error) | Immediate warning + null | Different approach ⚠️ |
| Type Path Syntax | Must use `/mob` explicitly | Tried to infer from `mob` | Not DM-compliant ❌ |

## What About `true` and `false`?

In BYOND DM, `true` and `false` are **NOT keywords**! They are:
- Either #define'd as constants (e.g., `#define TRUE 1`)
- Or treated as unknown identifiers (warning emitted)

This is why the C# lexer doesn't have them in the Keywords dictionary!

## Conclusion: What Needs to Be Fixed in C++

### Critical Fixes (Must Do)
1. **Fix preprocessor macro expansion** - Make sure #define actually expands
2. **Add keyword recognition in lexer** - Convert `null` to DM_Null token type
3. **Remove type path lookup from identifier resolution** - Not how DM works!

### Architectural Alignment
1. Study C# implementation stage by stage
2. Ensure C++ follows same pipeline flow
3. Match token types and AST node types exactly
4. Use same identifier resolution order

### Expected Results After Fixes
- Direction constants (NORTH, etc.): **0 warnings** (expanded by preprocessor)
- Boolean literals (true, false): **0 warnings** (expanded by preprocessor or unknown)
- null keyword: **0 warnings** (recognized by lexer)
- Type references (mob, obj): **Warnings expected** (use `/mob`, `/obj` instead)
- Undefined identifiers: **Warnings expected** (truly unknown)

**Final count: ~10-15 legitimate warnings for undefined identifiers**
