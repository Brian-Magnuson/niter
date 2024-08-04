#include "compiler.h"

#include "../checker/global_checker.h"
#include "../checker/local_checker.h"
#include "../codegen/code_generator.h"
#include "../codegen/emitter.h"
#include "../codegen/optimizer.h"
#include "../logger/logger.h"
#include <fstream>
#include <stdexcept>

void Compiler::add_file(const std::string& file_name, const std::string& src_code) {
    file_names.push_back(std::make_shared<std::string>(file_name));
    src_codes.push_back(std::make_shared<std::string>(src_code));
}

void Compiler::add_file(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_name);
    }
    std::string src_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) {
        throw std::runtime_error("Error reading file: " + file_name);
    }
    add_file(file_name, src_code);
}

int Compiler::compile() {

    if (this->target_destination == nullptr) {
        if (this->run_linker) {
            this->target_destination = std::make_shared<std::string>("out");
        } else {
            this->target_destination = std::make_shared<std::string>("out.o");
        }
    }

    std::vector<std::function<bool()>> stages = {
        [this]() {
            Scanner scanner;
            for (size_t i = 0; i < this->file_names.size(); i++) {
                scanner.scan_file(this->file_names[i], this->src_codes[i]);
            }
            this->tokens = scanner.get_tokens();
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [this]() {
            Parser parser;
            this->stmts = parser.parse(this->tokens);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [this]() {
            GlobalChecker global_checker;
            global_checker.type_check(this->stmts);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [this]() {
            LocalChecker local_checker;
            local_checker.type_check(this->stmts);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [this]() {
            CodeGenerator codegen;
            this->ir_module = codegen.generate(this->stmts, this->ir_target_destination);
            return ErrorLogger::inst().get_errors().size() == 0 && this->ir_module != nullptr;
        },
        [this]() {
            Optimizer optimizer;
            optimizer.optimize(this->ir_module);
            return ErrorLogger::inst().get_errors().size() == 0;
        },
        [this]() {
            Emitter emitter;
            auto target = *(this->target_destination);
            if (this->run_linker) {
                target += ".o";
            }
            emitter.emit(this->ir_module, target);
            return ErrorLogger::inst().get_errors().size() == 0;
        }
    };

    for (auto& stage : stages) {
        if (!stage()) {
            std::cerr << "Compiled with errors. Exiting..." << std::endl;
            return 1;
        }
    }

    if (this->run_linker) {
#ifdef _WIN32
        std::string cmd = "clang -o ";
#else
        std::string cmd = "clang -lc -lm -o ";
#endif
        cmd = cmd + *(this->target_destination) + " " + *(this->target_destination) + ".o";
        int status = system(cmd.c_str());
        if (status != 0) {
            std::cerr << "Linking failed with exit code " << status << std::endl;
            return 1;
        } else {
            // Delete the object file after linking
            std::string rm_cmd = "rm " + *(this->target_destination) + ".o";
            system(rm_cmd.c_str());
        }
    }

    return 0;
}
