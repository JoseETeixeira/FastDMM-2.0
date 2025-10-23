#include <iostream>
#include "include/DMParser.h"
#include "include/DMLexer.h"
#include "include/DMCompiler.h"
#include "include/DMASTExpression.h"

int main() {
    DMCompiler::DMCompiler compiler;
    DMCompiler::DMLexer lexer("test.dm", "42");
    DMCompiler::DMParser parser(&compiler, &lexer);
    
    auto expr = parser.Expression();
    
    if (dynamic_cast<DMCompiler::DMASTConstantInteger*>(expr.get())) {
        std::cout << "Got DMASTConstantInteger - CORRECT!" << std::endl;
        return 0;
    } else if (dynamic_cast<DMCompiler::DMASTAssign*>(expr.get())) {
        std::cout << "Got DMASTAssign - BUG!" << std::endl;
        return 1;
    } else {
        std::cout << "Got unknown type" << std::endl;
        return 1;
    }
}
