// #include "../src/pipeline/pipeline.h"
// #include <catch2/catch_test_macros.hpp>
// #include <filesystem>
// #include <string>
// #include <utility>

// /**
//  * @brief Runs the compiler on the given source code.
//  * This function will create a temporary executable and text file.
//  *
//  * @param file_name The name of the file to be compiled. The compiler won't actually read from this file, but error messages will use this name.
//  * @param source_code The source code to be compiled.
//  * @return std::pair<int, std::string> A pair containing the exit code of the executable and the output of the executable.
//  * If the compilation failed, the exit code will be -1.
//  */
// std::pair<int, std::string> run_compiler(const std::string& file_name, const std::string& source_code) {
//     std::filesystem::create_directory("./test/temp");

//     CompilerState state;
//     state.file_names.push_back(std::make_shared<std::string>(file_name));
//     state.src_codes.push_back(std::make_shared<std::string>(source_code));
//     state.target_destination = std::make_shared<std::string>("./test/temp/test_exe");
//     int result = compile(state);
//     if (result == 1) {
//         return {-1, ""};
//     }
//     int exit_code = system("./test/temp/test_exe > ./test/temp/test_output.txt");
//     std::ifstream output_file("./test/temp/test_output.txt");
//     std::string output((std::istreambuf_iterator<char>(output_file)), std::istreambuf_iterator<char>());
//     return {exit_code, output};
// }

// TEST_CASE("Compiler hello world", "[compiler]") {
//     std::string file_name = "test_files/hello_world.nit";
//     std::string source_code = R"(
// extern variadic fun printf(char*): i32;

// fun main(): i32 {
//     printf("Hello, world!\n");
//     return 0;
// }
// )";
//     auto output = run_compiler(file_name, source_code);
//     REQUIRE(output.first == 0);
//     REQUIRE(output.second == "Hello, world!\n");
// }
