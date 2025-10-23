#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <memory>
#include "Token.h"
#include "Location.h"

namespace DMCompiler {

class DMCompiler;
class DMLexer;

/// <summary>
/// Represents a preprocessor macro
/// </summary>
class DMMacro {
public:
    virtual ~DMMacro() = default;
    virtual std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) = 0;
    virtual bool HasParameters() const { return false; }
};

/// <summary>
/// Simple text replacement macro
/// </summary>
class DMMacroText : public DMMacro {
public:
    std::vector<Token> Tokens;
    
    explicit DMMacroText(const std::vector<Token>& tokens) : Tokens(tokens) {}
    
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override {
        return Tokens;
    }
};

/// <summary>
/// Function-like macro with parameters
/// </summary>
class DMMacroFunction : public DMMacro {
public:
    std::vector<std::string> Parameters;
    std::vector<Token> Tokens;
    
    DMMacroFunction(const std::vector<std::string>& params, const std::vector<Token>& tokens)
        : Parameters(params), Tokens(tokens) {}
    
    bool HasParameters() const override { return true; }
    
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override;
};

/// <summary>
/// Built-in __LINE__ macro
/// </summary>
class DMMacroLine : public DMMacro {
public:
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override;
};

/// <summary>
/// Built-in __FILE__ macro
/// </summary>
class DMMacroFile : public DMMacro {
public:
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override;
};

/// <summary>
/// Built-in DM_VERSION macro
/// </summary>
class DMMacroVersion : public DMMacro {
public:
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override;
};

/// <summary>
/// Built-in DM_BUILD macro
/// </summary>
class DMMacroBuild : public DMMacro {
public:
    std::vector<Token> Expand(const std::vector<Token>& arguments, const Location& location) override;
};

/// <summary>
/// DM Preprocessor - handles #include, #define, #if, etc.
/// </summary>
class DMPreprocessor {
public:
    explicit DMPreprocessor(DMCompiler* compiler = nullptr);
    ~DMPreprocessor();

    // Main preprocessing entry point
    std::vector<Token> Preprocess(const std::string& filePath);
    
    // Include tracking
    std::vector<std::string> GetIncludedMaps() const { return IncludedMaps_; }
    std::string GetIncludedInterface() const { return IncludedInterface_; }
    
    // Define management
    void Define(const std::string& name, const std::string& value);
    void Undefine(const std::string& name);
    bool IsDefined(const std::string& name) const;
    
    // Macro expansion for expression evaluation
    std::vector<Token> ExpandMacroForExpression(const std::string& name) const;

private:
    DMCompiler* Compiler_;
    
    std::vector<std::string> IncludedMaps_;
    std::string IncludedInterface_;
    
    // Lexer stack for nested includes
    std::stack<std::unique_ptr<DMLexer>> LexerStack_;
    std::stack<std::string> IncludeDirectoryStack_; // Tracks directory for each lexer
    
    // Token buffering
    std::stack<Token> UnprocessedTokens_;
    std::stack<Token> BufferedWhitespace_;
    
    // Macro definitions
    std::unordered_map<std::string, std::unique_ptr<DMMacro>> Defines_;
    
    // Include tracking
    std::unordered_set<std::string> IncludedFiles_;
    
    // Conditional compilation state
    std::stack<bool> LastIfEvaluations_;
    bool CanUseDirective_;
    bool CurrentLineContainsNonWhitespace_;
    
    // Token processing
    Token GetNextToken();
    void PushToken(const Token& token);
    void PushTokens(const std::vector<Token>& tokens);
    
    // Directive handlers
    void HandleIncludeDirective(const Token& token, std::vector<Token>& result);
    void HandleDefineDirective(const Token& token);
    void HandleUndefineDirective(const Token& token);
    void HandleIfDirective(const Token& token);
    void HandleIfDefDirective(const Token& token);
    void HandleIfNDefDirective(const Token& token);
    void HandleElifDirective(const Token& token);
    void HandleElseDirective(const Token& token);
    void HandleEndIfDirective(const Token& token);
    void HandleErrorDirective(const Token& token);
    void HandleWarningDirective(const Token& token);
    void HandlePragmaDirective(const Token& token);
    
    // Macro handling
    bool TryExpandMacro(const Token& token);
    std::vector<Token> ReadMacroArguments();
    
    // Conditional evaluation
    bool EvaluateCondition(const std::vector<Token>& tokens);
    void SkipIfBody(bool skipElse);
    
    // File inclusion
    bool IncludeFile(const std::string& path, const Location& includeLocation);
    std::vector<Token> PreprocessFile(const std::string& path, const Location& includeLocation);
    std::string ResolvePath(const std::string& path, const std::string& currentFile);
    
    // Helpers
    Token ConsumeToken();
    std::vector<Token> ReadLineTokens();
    bool IsDirective(TokenType type) const;
};

} // namespace DMCompiler
