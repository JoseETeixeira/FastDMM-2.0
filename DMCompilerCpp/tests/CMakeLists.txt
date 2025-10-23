cmake_minimum_required(VERSION 3.15)

# Test executable
add_executable(dm_compiler_tests
    test_lexer.cpp
    test_preprocessor.cpp
    test_parser.cpp
    test_compiler.cpp
    test_objecttree.cpp
    test_proc_simple.cpp
    test_bytecode.cpp
    test_expression_compiler.cpp
    test_statement_compiler.cpp
    test_dmmparser.cpp
    test_goto_forward_ref.cpp
    example_compilation_tests.cpp
    test_main.cpp
)

target_link_libraries(dm_compiler_tests PRIVATE DMCompilerLib)

# Standalone DMValueType test
add_executable(test_dmvaluetype
    test_dmvaluetype.cpp
)

target_link_libraries(test_dmvaluetype PRIVATE DMCompilerLib)

# Add test data directory
file(COPY testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY test_files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Register tests with CTest
add_test(NAME LexerTests COMMAND dm_compiler_tests [lexer])
add_test(NAME PreprocessorTests COMMAND dm_compiler_tests [preprocessor])
add_test(NAME ParserTests COMMAND dm_compiler_tests [parser])
add_test(NAME CompilerTests COMMAND dm_compiler_tests [compiler])
add_test(NAME ObjectTreeTests COMMAND dm_compiler_tests [objecttree])
add_test(NAME ProcTests COMMAND dm_compiler_tests [proc])
add_test(NAME BytecodeTests COMMAND dm_compiler_tests [bytecode])
add_test(NAME ExprCompilerTests COMMAND dm_compiler_tests [exprcompiler])
add_test(NAME DMMParserTests COMMAND dm_compiler_tests [dmmparser])
add_test(NAME GotoForwardRefTests COMMAND dm_compiler_tests [gotoforward])
add_test(NAME ExampleCompilationTests COMMAND dm_compiler_tests [examples])
