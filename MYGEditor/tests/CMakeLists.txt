# Tests for MYG Editor

# ProjectManager test
add_executable(test_project_manager 
    test_project_manager.cpp
    ../src/ProjectManager.cpp
    ../src/ObjectTreeCache.cpp
)

target_include_directories(test_project_manager PRIVATE
    ../include
    ../../DMCompilerCpp/include
)

target_link_libraries(test_project_manager PRIVATE
    DMCompilerLib
)

# Add as a test
add_test(NAME ProjectManager_Test COMMAND test_project_manager)

# ObjectTreeManager test
add_executable(test_object_tree_manager 
    test_object_tree_manager.cpp
    ../src/ObjectTreeManager.cpp
    ../src/ProjectManager.cpp
    ../src/ObjectTreeCache.cpp
)

target_include_directories(test_object_tree_manager PRIVATE
    ../include
    ../../DMCompilerCpp/include
)

target_link_libraries(test_object_tree_manager PRIVATE
    DMCompilerLib
)

# Add as a test
add_test(NAME ObjectTreeManager_Test COMMAND test_object_tree_manager)

message(STATUS "Tests configured: test_project_manager, test_object_tree_manager")
