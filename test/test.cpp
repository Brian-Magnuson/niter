#include "../src/logger/logger.h"
#include "../src/scanner/token.h"
#include "catch/catch_amalgamated.hpp"
#include <string>

TEST_CASE("Sanity check", "[sanity]") {
    REQUIRE(1 == 1);
}

TEST_CASE("Logger", "[logger]") {
    ErrorLogger logger;
    std::string file_name = "test_files/error_test.nit";
    std::string source_code = "var x = 5";
    Location location = {
        std::make_shared<std::string>(file_name),
        1, // line
        0, // column
        3, // length
        0, // line_index
        std::make_shared<std::string>(source_code)
    };
    Token token(TOK_IDENT, "x", 5, location);

    std::stringstream ss;
    logger.set_ostream(ss);
    logger.log_error(token, E_CONFIG, "Test error message");

    std::string expected = file_name + ":1:0\n\033[31mError 1: \033[0m1000 Test error message\n    1 | var x = 5\n        \033[31m^~~\033[0m\n\n";
    /*
    This is what that string looks like:
    test_files/error_test.nit:1:0
    Error 1: Test error message
        1 | var x = 5
            ^~~

    */

    REQUIRE(ss.str() == expected);
    // logger.set_ostream(std::cerr);
    // logger.log_error(token, E_CONFIG, "Test error message");
}
