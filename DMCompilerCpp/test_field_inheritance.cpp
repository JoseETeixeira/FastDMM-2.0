#include "DMObjectTree.h"
#include "DMObject.h"
#include "DMVariable.h"
#include <iostream>

using namespace DMCompiler;

int main() {
    DMObjectTree tree;
    
    // Create /atom with 'loc' variable
    DMObject* atom = tree.CreateType(DreamPath("/atom"), nullptr);
    DMVariable locVar;
    locVar.Name = "loc";
    atom->Variables["loc"] = locVar;
    
    // Create /atom/movable inheriting from /atom
    DMObject* movable = tree.CreateType(DreamPath("/atom/movable"), atom);
    
    // Create /mob inheriting from /atom/movable
    DMObject* mob = tree.CreateType(DreamPath("/mob"), movable);
    
    // Add 'techs' variable to /mob
    DMVariable techsVar;
    techsVar.Name = "techs";
    mob->Variables["techs"] = techsVar;
    
    // Test 1: Can /mob find 'techs' (direct variable)?
    const DMVariable* foundTechs = mob->GetVariable("techs");
    std::cout << "Test 1 - /mob finding 'techs': " 
              << (foundTechs ? "PASS" : "FAIL") << std::endl;
    
    // Test 2: Can /mob find 'loc' (inherited from /atom)?
    const DMVariable* foundLoc = mob->GetVariable("loc");
    std::cout << "Test 2 - /mob finding 'loc' (inherited): " 
              << (foundLoc ? "PASS" : "FAIL") << std::endl;
    
    // Test 3: Can /atom/movable find 'loc' (inherited from parent)?
    const DMVariable* foundLocMovable = movable->GetVariable("loc");
    std::cout << "Test 3 - /atom/movable finding 'loc' (inherited): " 
              << (foundLocMovable ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
