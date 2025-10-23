#include "DMLexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace DMCompiler;

int main() {
    std::string source = R"(/mob
    var/test = 1
)";
    
    DMLexer lexer("test", source);
    
    while (true) {
        Token token = lexer.GetNextToken();
        
        std::cout << "Token: ";
        switch (token.Type) {
            case TokenType::Slash: std::cout << "Slash"; break;
            case TokenType::Identifier: std::cout << "Identifier(" << token.Text << ")"; break;
            case TokenType::Newline: std::cout << "Newline"; break;
            case TokenType::Indent: std::cout << "INDENT"; break;
            case TokenType::Dedent: std::cout << "DEDENT"; break;
            case TokenType::Var: std::cout << "Var"; break;
            case TokenType::Assign: std::cout << "Assign"; break;
            case TokenType::Number: std::cout << "Number(" << token.Text << ")"; break;
            case TokenType::EndOfFile: std::cout << "EOF"; break;
            default: std::cout << "Other(" << static_cast<int>(token.Type) << ")"; break;
        }
        std::cout << std::endl;
        
        if (token.Type == TokenType::EndOfFile) {
            break;
        }
    }
    
    return 0;
}
