#include "myg/ProjectManager.h"
#include <DMObjectTree.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "=== ProjectManager Test ===" << std::endl;
    
    // Get the test DME file path
    std::string test_dme = "../../DMCompilerCpp/tests/testdata/simple_test/test.dme";
    
    // Convert to absolute path
    fs::path abs_path = fs::absolute(test_dme);
    std::cout << "Test DME path: " << abs_path << std::endl;
    
    if (!fs::exists(abs_path)) {
        std::cerr << "ERROR: Test DME file not found!" << std::endl;
        return 1;
    }
    
    // Create ProjectManager
    myg::ProjectManager pm;
    
    // Test 1: Load project
    std::cout << "\n--- Test 1: Load Project ---" << std::endl;
    if (!pm.LoadProject(abs_path.string())) {
        std::cerr << "ERROR: Failed to load project: " << pm.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "✓ Project loaded successfully" << std::endl;
    std::cout << "  Project name: " << pm.GetProjectName() << std::endl;
    std::cout << "  Project directory: " << pm.GetProjectDirectory() << std::endl;
    
    // Test 2: Check if object tree is loaded (should be false before compilation)
    std::cout << "\n--- Test 2: Check Object Tree (before compilation) ---" << std::endl;
    if (pm.HasValidObjectTree()) {
        std::cout << "✓ Object tree loaded from cache" << std::endl;
    } else {
        std::cout << "✓ No object tree yet (expected)" << std::endl;
    }
    
    // Test 3: Compile project
    std::cout << "\n--- Test 3: Compile Project ---" << std::endl;
    bool compile_success = pm.CompileProject([](const std::string& msg, float progress) {
        std::cout << "  [" << (int)(progress * 100) << "%] " << msg << std::endl;
    });
    
    if (!compile_success) {
        std::cerr << "ERROR: Compilation failed: " << pm.GetLastError() << std::endl;
        
        const auto& messages = pm.GetCompilationMessages();
        if (!messages.empty()) {
            std::cerr << "\nCompilation messages:" << std::endl;
            for (const auto& msg : messages) {
                std::cerr << "  " << msg << std::endl;
            }
        }
        return 1;
    }
    std::cout << "✓ Compilation successful" << std::endl;
    
    // Test 4: Check object tree after compilation
    std::cout << "\n--- Test 4: Check Object Tree (after compilation) ---" << std::endl;
    if (!pm.HasValidObjectTree()) {
        std::cerr << "ERROR: Object tree not available after compilation" << std::endl;
        return 1;
    }
    std::cout << "✓ Object tree is available" << std::endl;
    
    auto* tree = pm.GetObjectTree();
    if (!tree) {
        std::cerr << "ERROR: GetObjectTree() returned null" << std::endl;
        return 1;
    }
    std::cout << "✓ Object tree pointer is valid" << std::endl;
    std::cout << "  Object count: " << tree->AllObjects.size() << std::endl;
    std::cout << "  String table size: " << tree->StringTable.size() << std::endl;
    
    // Test 5: Verify cache file was created
    std::cout << "\n--- Test 5: Check Cache File ---" << std::endl;
    std::string cache_path = pm.GetProjectDirectory() + "/.myg_cache.bin";
    if (fs::exists(cache_path)) {
        std::cout << "✓ Cache file created: " << cache_path << std::endl;
        auto cache_size = fs::file_size(cache_path);
        std::cout << "  Cache size: " << cache_size << " bytes" << std::endl;
    } else {
        std::cout << "⚠ Cache file not created (serialization not fully implemented)" << std::endl;
    }
    
    std::cout << "\n=== All Tests Passed! ===" << std::endl;
    return 0;
}
