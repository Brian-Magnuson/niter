#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>

#include "../src/checker/environment.h"
#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/codegen/code_generator.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"

static std::unique_ptr<llvm::Module> setup(const std::string& source_code, const std::string& file_name, bool set_printing_enabled) {
    auto source_code_ptr = std::make_shared<std::string>(source_code);
    auto file_name_ptr = std::make_shared<std::string>(file_name);

    ErrorLogger::inst().set_printing_enabled(set_printing_enabled);

    Scanner scanner;
    scanner.scan_file(file_name_ptr, source_code_ptr);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);
    CodeGenerator code_generator;
    return code_generator.generate(stmts);
}

/**
 * @brief Finds the function with the given name in the module and runs it.
 *
 * @param module A unique pointer to the module to run (requires ownership).
 * @param fun_name The name of the function to run.
 * @param args A vector of arguments to pass to the function, defaults to an empty vector.
 * @return std::pair<llvm::GenericValue, bool> A pair containing the result of the function, if it is valid, and a boolean indicating if the llvm::GenericValue is valid.
 */
static std::pair<llvm::GenericValue, bool> run_code(std::unique_ptr<llvm::Module> ir_module, const std::string& fun_name, std::vector<llvm::GenericValue> args = {}) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::string error_str;
    llvm::ExecutionEngine* engine = llvm::EngineBuilder(std::move(ir_module))
                                        .setEngineKind(llvm::EngineKind::Interpreter)
                                        .setErrorStr(&error_str)
                                        .create();

    if (!engine) {
        std::cerr << "Failed to create ExecutionEngine: " << error_str << std::endl;
        return {llvm::GenericValue(), false};
    }
    llvm::Function* target_function = engine->FindFunctionNamed(fun_name);
    if (!target_function) {
        return {llvm::GenericValue(), false};
    };
    llvm::GenericValue result = engine->runFunction(target_function, args);
    return {result, true};
}

static void cleanup() {
    Environment& env = Environment::inst();
    env.reset();
    ErrorLogger& logger = ErrorLogger::inst();
    logger.reset();
}

TEST_CASE("Compiler", "[compiler]") {

    std::string source_code = R"(
        fun main(): i32 {
            var x: i32
            x = 1
            return x
        }
    )";

    auto ir_module = setup(source_code, "test_files/compiler.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 1);

    cleanup();
}

TEST_CASE("Compiler add", "[compiler]") {

    std::string source_code = R"(
        fun main(): i32 {
            return 1 + 2
        }
    )";

    auto ir_module = setup(source_code, "test_files/compiler_add.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 3);

    cleanup();
}

TEST_CASE("Compiler add 2", "[compiler]") {

    std::string source_code = R"(
        fun add(): i32 {
            return 1 + 2 + 3
        }
    )";

    auto ir_module = setup(source_code, "test_files/compiler_add_2.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "__add");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 6);

    cleanup();
}

TEST_CASE("Compiler tuple", "[compiler]") {

    std::string source_code = R"(
        fun main(): i32 {
            var x: (i32, i32)
            x = (1, 2)
            return x[0] + x[1]
        }
    )";

    auto ir_module = setup(source_code, "test_files/compiler_tuple.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 3);

    cleanup();
}

TEST_CASE("Compiler tuple 2", "[compiler]") {

    std::string source_code = R"(
        fun main(): i32 {
            var x: (i32, i32, i32, i32)
            x = (1, 2, 3, 4)
            return x[0] + x[1] + x[2] + x[3]
        }
    )";

    auto ir_module = setup(source_code, "test_files/compiler_tuple_2.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 10);

    cleanup();
}

TEST_CASE("Compiler if stmt", "[compiler]") {

    std::string source_code = R"(
            fun main(): i32 {
                var x: i32
                x = 1
                if x == 1 {
                    return 1
                }
                return 0
            }
        )";

    auto ir_module = setup(source_code, "test_files/compiler_if_stmt.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 1);

    cleanup();
}

TEST_CASE("Compiler if stmt 2", "[compiler]") {

    std::string source_code = R"(
            fun main(): i32 {
                var x: i32
                x = 2
                if x == 1 {
                    return 1
                }
                return 0
            }
        )";

    auto ir_module = setup(source_code, "test_files/compiler_if_stmt_2.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 0);

    cleanup();
}

TEST_CASE("Compiler if stmt 3", "[compiler]") {

    std::string source_code = R"(
            fun main(): i32 {
                var x: i32
                x = 1
                if x == 1 {
                    return 1
                } else {
                    return 0
                }
            }
        )";

    auto ir_module = setup(source_code, "test_files/compiler_if_stmt_3.nit", true);

    auto [result, ok] = run_code(std::move(ir_module), "main");
    REQUIRE(ok);
    REQUIRE(result.IntVal.getSExtValue() == 1);

    cleanup();
}

// FIXME: This test is failing because LLVM can't load the Point type correctly in the interpreter.
// Figure out how to fix this.
// TEST_CASE("Compiler struct", "[compiler]") {

//     std::string source_code = R"(
//         struct Point {
//             var x: i32
//             var y: i32
//             var z: i32

//             fun new(): Point {
//                 return :Point { x: 1, y: 2, z: 3 }
//             }
//         }
//     )";

//     auto ir_module = setup(source_code, "test_files/compiler_struct.nit", true);

//     auto [result, ok] = run_code(std::move(ir_module), "__Point__new");
//     REQUIRE(ok);
//     // REQUIRE(result.AggregateVal[0].IntVal.getSExtValue() == 1);
//     // REQUIRE(result.AggregateVal[1].IntVal.getSExtValue() == 2);
//     // REQUIRE(result.AggregateVal[2].IntVal.getSExtValue() == 3);

//     cleanup();
// }
