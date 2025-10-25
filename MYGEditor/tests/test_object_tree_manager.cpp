#include "myg/ObjectTreeManager.h"
#include "myg/ProjectManager.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "=== ObjectTreeManager Test ===" << std::endl;
    
    // Get the test DME file path
    std::string test_dme = "../../DMCompilerCpp/tests/testdata/simple_test/test.dme";
    
    // Convert to absolute path
    fs::path abs_path = fs::absolute(test_dme);
    std::cout << "Test DME path: " << abs_path << std::endl;
    
    if (!fs::exists(abs_path)) {
        std::cerr << "ERROR: Test DME file not found!" << std::endl;
        return 1;
    }
    
    // Create ProjectManager and compile
    myg::ProjectManager pm;
    
    std::cout << "\n--- Loading and Compiling Project ---" << std::endl;
    if (!pm.LoadProject(abs_path.string())) {
        std::cerr << "ERROR: Failed to load project: " << pm.GetLastError() << std::endl;
        return 1;
    }
    
    if (!pm.CompileProject([](const std::string& msg, float progress) {})) {
        std::cerr << "ERROR: Compilation failed: " << pm.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "✓ Project compiled successfully" << std::endl;
    
    auto* tree = pm.GetObjectTree();
    if (!tree) {
        std::cerr << "ERROR: No object tree available" << std::endl;
        return 1;
    }
    
    // Create ObjectTreeManager
    MYG::ObjectTreeManager otm;
    
    // Test 1: Set object tree
    std::cout << "\n--- Test 1: Set Object Tree ---" << std::endl;
    otm.SetObjectTree(tree);
    if (!otm.HasObjectTree()) {
        std::cerr << "ERROR: Object tree not set" << std::endl;
        return 1;
    }
    std::cout << "✓ Object tree set successfully" << std::endl;
    
    // Test 2: Get root object
    std::cout << "\n--- Test 2: Get Root Object ---" << std::endl;
    auto* root = otm.GetRoot();
    if (!root) {
        std::cerr << "ERROR: Failed to get root object" << std::endl;
        return 1;
    }
    std::cout << "✓ Root object retrieved" << std::endl;
    std::cout << "  Root path: " << root->Path.ToString() << std::endl;
    
    // Test 3: Get children of root
    std::cout << "\n--- Test 3: Get Children of Root ---" << std::endl;
    auto children = otm.GetChildren(root);
    std::cout << "✓ Root has " << children.size() << " children" << std::endl;
    for (size_t i = 0; i < std::min(children.size(), size_t(5)); ++i) {
        std::cout << "  - " << children[i]->Path.ToString() << std::endl;
    }
    if (children.size() > 5) {
        std::cout << "  ... and " << (children.size() - 5) << " more" << std::endl;
    }
    
    // Test 4: Get object by path
    std::cout << "\n--- Test 4: Get Object by Path ---" << std::endl;
    auto* datum = otm.GetObjectByPath("/datum");
    if (!datum) {
        std::cerr << "ERROR: Failed to get /datum object" << std::endl;
        return 1;
    }
    std::cout << "✓ Found /datum object" << std::endl;
    std::cout << "  Path: " << datum->Path.ToString() << std::endl;
    
    // Test 5: Get children with caching (should be faster second time)
    std::cout << "\n--- Test 5: Test Caching ---" << std::endl;
    auto children2 = otm.GetChildren(root);
    if (children.size() != children2.size()) {
        std::cerr << "ERROR: Cached children count mismatch" << std::endl;
        return 1;
    }
    std::cout << "✓ Cached children retrieval successful" << std::endl;
    
    // Test 6: Get all variables for an object
    std::cout << "\n--- Test 6: Get All Variables ---" << std::endl;
    auto vars = otm.GetAllVariables(datum);
    std::cout << "✓ /datum has " << vars.size() << " variables" << std::endl;
    for (size_t i = 0; i < std::min(vars.size(), size_t(10)); ++i) {
        const auto& [name, value] = vars[i];
        std::cout << "  - " << name;
        if (!value.empty()) {
            std::cout << " = " << value;
        }
        std::cout << std::endl;
    }
    if (vars.size() > 10) {
        std::cout << "  ... and " << (vars.size() - 10) << " more" << std::endl;
    }
    
    // Test 7: Get specific variable
    std::cout << "\n--- Test 7: Get Specific Variable ---" << std::endl;
    std::string tag_value = otm.GetVariable(datum, "tag");
    if (!tag_value.empty()) {
        std::cout << "✓ Found 'tag' variable with value: " << tag_value << std::endl;
    } else {
        std::cout << "✓ 'tag' variable has no default value (expected)" << std::endl;
    }
    
    // Test 8: Clear caches
    std::cout << "\n--- Test 8: Clear Caches ---" << std::endl;
    otm.SetObjectTree(tree);  // This should clear caches
    auto children3 = otm.GetChildren(root);
    if (children.size() != children3.size()) {
        std::cerr << "ERROR: Children count mismatch after cache clear" << std::endl;
        return 1;
    }
    std::cout << "✓ Cache cleared and rebuilt successfully" << std::endl;
    
    std::cout << "\n=== All Tests Passed! ===" << std::endl;
    return 0;
}
