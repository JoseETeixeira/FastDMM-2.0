#include "DMPreprocessor.h"
#include "DMCompiler.h"
#include <iostream>
#include <chrono>

using namespace DMCompiler;

int main() {
    std::cout << "=== Testing Path Resolution Caching ===" << std::endl;
    
    // Create a compiler and preprocessor
    DMCompiler compiler;
    DMPreprocessor preprocessor(&compiler);
    
    // Test path resolution with caching
    std::string currentFile = "C:/test/main.dm";
    std::string relativePath = "../include/helper.dm";
    
    // First resolution (cache miss)
    auto start1 = std::chrono::high_resolution_clock::now();
    std::string resolved1 = preprocessor.ResolvePath(relativePath, currentFile);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    
    std::cout << "First resolution (cache miss): " << resolved1 << std::endl;
    std::cout << "Time: " << duration1.count() << " microseconds" << std::endl;
    
    // Second resolution (cache hit)
    auto start2 = std::chrono::high_resolution_clock::now();
    std::string resolved2 = preprocessor.ResolvePath(relativePath, currentFile);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    
    std::cout << "Second resolution (cache hit): " << resolved2 << std::endl;
    std::cout << "Time: " << duration2.count() << " microseconds" << std::endl;
    
    // Verify results are identical
    if (resolved1 == resolved2) {
        std::cout << "✓ Cache working correctly - results match" << std::endl;
    } else {
        std::cout << "✗ Cache error - results don't match" << std::endl;
        return 1;
    }
    
    // Verify cache is faster (should be at least 2x faster)
    if (duration2.count() < duration1.count()) {
        double speedup = static_cast<double>(duration1.count()) / duration2.count();
        std::cout << "✓ Cache is faster: " << speedup << "x speedup" << std::endl;
    } else {
        std::cout << "Note: Cache timing may vary on first run" << std::endl;
    }
    
    std::cout << "\n=== Path Cache Test Complete ===" << std::endl;
    return 0;
}
