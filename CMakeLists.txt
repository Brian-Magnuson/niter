# CMake configuration
cmake_minimum_required(VERSION 3.19)
project(niter)
include(FetchContent)
include(CTest)

# Compiler configuration
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)

# Libraries

# # LLVM
find_package(LLVM 18.1.0 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
set(LLVM_ENABLE_ZSTD OFF)
llvm_map_components_to_libnames(llvm_libs core orcjit native interpreter)
message(STATUS "LLVM libraries: ${llvm_libs}")

# # Catch2
find_package(Catch2 3.4.0 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Fetching Catch2 from GitHub.")
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
else()
    message(STATUS "Found Catch2 ${Catch2_VERSION}")
endif()

include(Catch)

# Source files
set(LOGGER_SRC src/logger/logger.cpp)
set(SCANNER_SRC src/scanner/scanner.cpp src/scanner/token.cpp)
set(PARSER_SRC src/parser/parser.cpp src/parser/ast_printer.cpp)
set(CHECKER_SRC src/checker/environment.cpp src/checker/global_checker.cpp src/checker/local_checker.cpp)
set(CODEGEN_SRC src/codegen/code_generator.cpp src/codegen/optimizer.cpp src/codegen/emitter.cpp)
set(COMPILER_SRC src/compiler/compiler.cpp)
set(UTILITY_SRC src/utility/core.cpp src/utility/expr.cpp src/utility/node.cpp)
set(MAIN_SRC src/main.cpp)
set(SOURCES
    ${LOGGER_SRC}
    ${SCANNER_SRC}
    ${PARSER_SRC}
    ${CHECKER_SRC}
    ${CODEGEN_SRC}
    ${COMPILER_SRC}
    ${UTILITY_SRC}
)
set(TEST_SOURCES
    test/checker_fun_test.cpp
    test/checker_struct_test.cpp
    test/checker_flow_test.cpp
    test/compiler_test.cpp
    test/parser_decl_test.cpp
    test/parser_expr_test.cpp
    test/parser_flow_test.cpp
    test/scanner_test.cpp
)

# Main executable
add_executable(niterc ${SOURCES} ${MAIN_SRC})
target_link_libraries(niterc ${llvm_libs})

# Tests executable
add_executable(tests ${SOURCES} ${TEST_SOURCES})
target_link_libraries(tests Catch2::Catch2WithMain ${llvm_libs})
enable_testing()
catch_discover_tests(tests)
