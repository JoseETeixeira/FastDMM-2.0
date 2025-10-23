#include <iostream>
#include "include/DMLexer.h"
#include "include/Token.h"

int main() {
    DMCompiler::DMLexer lexer("test.dm", "42");
    
    DMCompiler::Token tok1 = lexer.NextToken();
    std::cout << "Token 1: Type=" << static_cast<int>(tok1.Type) << " Text='" << tok1.Text << "'" << std::endl;
    
    DMCompiler::Token tok2 = lexer.NextToken();
    std::cout << "Token 2: Type=" << static_cast<int>(tok2.Type) << " Text='" << tok2.Text << "'" << std::endl;
    
    return 0;
}
