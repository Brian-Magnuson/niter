#include "checker/global_checker.h"
#include "checker/local_checker.h"
#include "codegen/code_generator.h"
#include "codegen/emitter.h"
#include "codegen/optimizer.h"
#include "logger/logger.h"
#include "parser/parser.h"
#include "scanner/scanner.h"
#include <iostream>
#include <memory>
#include <string>

int main() {
    // FIXME: Change this to read from a file
    std::string source = R"(
fun main(): i32 {
    puts "Hello, World!\n";
    return 0;
}
)";
    auto src_code = std::make_shared<std::string>(source);
    auto file_name = std::make_shared<std::string>("main.nit");

    ErrorLogger& logger = ErrorLogger::inst();

    Scanner scanner;
    scanner.scan_file(file_name, src_code);

    if (logger.get_errors().size() > 0) {
        std::cerr << "Errors found during scanning. Exiting...\n";
        return 1;
    }

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    if (logger.get_errors().size() > 0) {
        std::cerr << "Errors found during parsing. Exiting...\n";
        return 2;
    }

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    if (logger.get_errors().size() > 0) {
        std::cerr << "Errors found during type checking. Exiting...\n";
        return 3;
    }

    LocalChecker local_checker;
    local_checker.type_check(stmts);

    if (logger.get_errors().size() > 0) {
        std::cerr << "Errors found during type checking. Exiting...\n";
        return 4;
    }

    CodeGenerator codegen;
    auto ir_module = codegen.generate(stmts, true);

    if (logger.get_errors().size() > 0 || ir_module == nullptr) {
        std::cerr << "Errors found during code generation. Exiting...\n";
        return 5;
    }

    Optimizer optimizer;
    optimizer.optimize(ir_module);

    Emitter emitter;
    emitter.emit(ir_module, "./bin/test_output.o");

    return 0;
}
