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

TEST_CASE("Codegen", "[codegen]") {

    std::string source_code = R"(
        fun main(): i32 {
            var x: i32;
            x = 1;
            return x;
        }
    )";

    auto ir_module = setup(source_code, "test_files/codegen.nit", true);

    auto result = run_code(std::move(ir_module), "main");
    REQUIRE(result.second);
    REQUIRE(result.first.IntVal.getSExtValue() == 1);

    cleanup();
}
