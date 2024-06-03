#include "pipeline/pipeline.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "Usage: niterc [-c] [-o output] [-dump-ir output] <source files>" << std::endl;
        return 2;
    }

    CompilerState state;

    for (int i = 1; i < argc; i++) {
        auto str = std::make_shared<std::string>(argv[i]);

        if (str->at(0) == '-') {
            if (*str == "-o") {
                if (state.target_destination != nullptr) {
                    std::cerr << "Multiple output files specified" << std::endl;
                    return 2;
                } else if (i + 1 < argc) {
                    i++;
                    auto output = std::make_shared<std::string>(argv[i]);
                    state.target_destination = output;
                } else {
                    std::cerr << "Expected output file after -o" << std::endl;
                    return 2;
                }
            } else if (*str == "-c") {
                state.run_linker = false;
            } else if (*str == "-dump-ir") {
                if (state.ir_target_destination != "") {
                    std::cerr << "Multiple IR output files specified" << std::endl;
                    return 2;
                } else if (i + 1 < argc) {
                    i++;
                    auto output = std::make_shared<std::string>(argv[i]);
                    state.ir_target_destination = *output;
                } else {
                    std::cerr << "Expected IR output file after -dump-ir" << std::endl;
                    return 2;
                }
            } else {
                std::cerr << "Unknown option: " << str << std::endl;
                return 2;
            }

            continue;
        }

        std::ifstream file(*str);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << *str << std::endl;
            return 3;
        }
        auto src = std::make_shared<std::string>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        state.file_names.push_back(str);
        state.src_codes.push_back(src);
    }

    if (state.file_names.empty()) {
        std::cerr << "No input files specified" << std::endl;
        return 2;
    }

    if (state.target_destination == nullptr) {
        if (state.run_linker) {
            state.target_destination = std::make_shared<std::string>("out");
        } else {
            state.target_destination = std::make_shared<std::string>("out.o");
        }
    }

    return compile(state);
}
