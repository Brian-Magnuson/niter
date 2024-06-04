#include "compiler/compiler.h"
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

    Compiler compiler;
    bool target_set = false;
    bool ir_target_set = false;
    bool run_linker_set = false;

    for (int i = 1; i < argc; i++) {
        auto str = std::make_shared<std::string>(argv[i]);

        if (str->at(0) == '-') {
            if (*str == "-o") {
                if (target_set) {
                    std::cerr << "Multiple output files specified" << std::endl;
                    return 2;
                } else if (i + 1 < argc) {
                    i++;
                    compiler.set_target_destination(argv[i]);
                    target_set = true;
                } else {
                    std::cerr << "Expected output file after -o" << std::endl;
                    return 2;
                }
            } else if (*str == "-c") {
                if (run_linker_set) {
                    std::cerr << "Multiple -c flags specified" << std::endl;
                    return 2;
                }
                compiler.set_run_linker(false);
                run_linker_set = true;
            } else if (*str == "-dump-ir") {
                if (ir_target_set) {
                    std::cerr << "Multiple IR output files specified" << std::endl;
                    return 2;
                } else if (i + 1 < argc) {
                    i++;
                    compiler.set_ir_target_destination(argv[i]);
                    ir_target_set = true;
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

        try {
            compiler.add_file(*str);
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return 3;
        }
    }

    if (!compiler.has_input()) {
        std::cerr << "No input files specified" << std::endl;
        return 2;
    }

    return compiler.compile();
}
