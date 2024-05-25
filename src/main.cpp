#include "checker/global_checker.h"
#include "checker/local_checker.h"
#include "codegen/code_generator.h"
#include "codegen/emitter.h"
#include "codegen/optimizer.h"
#include "logger/logger.h"
#include "parser/parser.h"
#include "scanner/scanner.h"
#include "llvm/IR/Module.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct CompilerState {
    std::vector<std::shared_ptr<std::string>> file_names;
    std::vector<std::shared_ptr<std::string>> src_codes;
    std::vector<std::shared_ptr<Token>> tokens;
    std::vector<std::shared_ptr<Stmt>> stmts;
    std::shared_ptr<llvm::Module> ir_module;
};

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

    CompilerState state;
    state.file_names.push_back(file_name);
    state.src_codes.push_back(src_code);

    std::vector<std::function<bool(CompilerState&)>> stages = {
        [](CompilerState& state) {
            Scanner scanner;
            for (size_t i = 0; i < state.file_names.size(); i++) {
                scanner.scan_file(state.file_names[i], state.src_codes[i]);
            }
            state.tokens = scanner.get_tokens();
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [](CompilerState& state) {
            Parser parser;
            state.stmts = parser.parse(state.tokens);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [](CompilerState& state) {
            GlobalChecker global_checker;
            global_checker.type_check(state.stmts);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [](CompilerState& state) {
            LocalChecker local_checker;
            local_checker.type_check(state.stmts);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [](CompilerState& state) {
            CodeGenerator codegen;
            state.ir_module = codegen.generate(state.stmts, true);
            return ErrorLogger::inst().get_errors().size() == 0 && state.ir_module != nullptr;
        },
        [](CompilerState& state) {
            Optimizer optimizer;
            optimizer.optimize(state.ir_module);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [](CompilerState& state) {
            Emitter emitter;
            emitter.emit(state.ir_module, "./debug/bin/test_output.o");
            return ErrorLogger::inst().get_errors().size() == 0;
        }
    };

    for (auto& stage : stages) {
        if (!stage(state)) {
            std::cerr << "Compiled with errors. Exiting..." << std::endl;
            return 1;
        }
    }

    return 0;
}
