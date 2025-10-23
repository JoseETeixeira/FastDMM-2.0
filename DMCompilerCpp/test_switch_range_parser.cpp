#include "DMParser.h"
#include "DMCompiler.h"
#include "DMLexer.h"
#include "DMASTExpression.h"
#include "DMASTStatement.h"
#include <iostream>
#include <sstream>

int main() {
    std::string code = R"(
/proc/test()
    var/x = 5
    switch(x)
        if(1 to 3)
            return "low"
)";
    
    DMCompiler::DMCompiler compiler;
    std::istringstream input(code);
    DMCompiler::DMLexer lexer(&compiler, input, "test.dm");
    DMCompiler::DMParser parser(&compiler, &lexer);
    
    try {
        auto file = parser.ParseFile();
        
        // Check if we got a proc definition
        if (file && !file->Statements.empty()) {
            auto* procDef = dynamic_cast<DMCompiler::DMASTObjectProcDefinition*>(file->Statements[0].get());
            if (procDef && procDef->Body) {
                // Look for switch statement
                for (auto& stmt : procDef->Body->Statements) {
                    if (auto* switchStmt = dynamic_cast<DMCompiler::DMASTProcStatementSwitch*>(stmt.get())) {
                        std::cout << "Found switch statement with " << switchStmt->Cases.size() << " cases\n";
                        
                        for (size_t i = 0; i < switchStmt->Cases.size(); i++) {
                            auto& switchCase = switchStmt->Cases[i];
                            std::cout << "Case " << i << " has " << switchCase.Values.size() << " values\n";
                            
                            for (size_t j = 0; j < switchCase.Values.size(); j++) {
                                auto& value = switchCase.Values[j];
                                if (auto* rangeExpr = dynamic_cast<DMCompiler::DMASTSwitchCaseRange*>(value.get())) {
                                    std::cout << "  Value " << j << " is a RANGE expression!\n";
                                } else {
                                    std::cout << "  Value " << j << " is NOT a range expression\n";
                                }
                            }
                        }
                        
                        return 0;
                    }
                }
                std::cout << "No switch statement found\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
