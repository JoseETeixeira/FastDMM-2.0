#include "DMCompiler.h"
#include "DMObjectTree.h"
#include <iostream>
#include <memory>

using namespace DMCompiler;

int main() {
    // Create compiler
    DMCompilerSettings settings;
    settings.Verbose = false;
    settings.SuppressUnimplemented = true;
    settings.NoStandard = true;
    
    auto compiler = std::make_unique<DMCompiler::DMCompiler>(settings);
    
    // Compile test file
    if (!compiler->Compile("test_var_block.dm")) {
        std::cerr << "Compilation failed" << std::endl;
        return 1;
    }
    
    // Get object tree
    auto* objectTree = compiler->GetObjectTree();
    
    // Get /mob object
    DMObject* mobObj = nullptr;
    if (!objectTree->TryGetDMObject(DreamPath::FromString("/mob"), &mobObj)) {
        std::cerr << "Failed to get /mob object" << std::endl;
        return 1;
    }
    
    // Check if variables are accessible
    std::vector<std::string> expectedVars = {"HP", "techs", "myBeam", "text", "beamState", "beamDam", "num", "beamSpeed", "beamDist", "m"};
    
    for (const auto& varName : expectedVars) {
        if (!mobObj->HasVariable(varName)) {
            std::cerr << "Variable '" << varName << "' not found in /mob" << std::endl;
            return 1;
        }
        
        auto* var = mobObj->GetVariable(varName);
        if (!var) {
            std::cerr << "GetVariable returned null for '" << varName << "'" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Variable '" << varName << "' is accessible" << std::endl;
    }
    
    std::cout << "\nAll variables are accessible through GetVariable()!" << std::endl;
    return 0;
}
