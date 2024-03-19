#include "../src/logger/logger.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "catch/catch_amalgamated.hpp"
#include <string>

TEST_CASE("Sanity check", "[sanity]") {
    REQUIRE(1 == 1);
}

TEST_CASE("Logger", "[logger]") {
    ErrorLogger& logger = ErrorLogger::inst();
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

    REQUIRE(logger.get_errors().size() == 1);
    REQUIRE(logger.get_errors().at(0) == E_CONFIG);

    // std::string expected = file_name + ":1:0\n\033[31mError 1: \033[0m1000 Test error message\n    1 | var x = 5\n        \033[31m^~~\033[0m\n\n";
    // /*
    // This is what that string looks like:
    // test_files/error_test.nit:1:0
    // Error 1: Test error message
    //     1 | var x = 5
    //         ^~~

    // */

    // REQUIRE(ss.str() == expected);
    logger.set_ostream(std::cerr);
    logger.log_error(token, E_CONFIG, "Test error message");

    ErrorLogger::inst().clear_errors();
}

TEST_CASE("Log in order", "[logger]") {
    ErrorLogger& logger = ErrorLogger::inst();
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
    Location location2 = {
        std::make_shared<std::string>(file_name),
        1, // line
        0, // column
        3, // length
        0, // line_index
        std::make_shared<std::string>(source_code)
    };
    Token token2(TOK_IDENT, "x", 5, location2);

    std::stringstream ss;
    logger.set_ostream(ss);
    logger.log_error(token, E_CONFIG, "Test error message");
    logger.log_error(token2, E_TEST_ERROR, "Test error message 2");

    REQUIRE(logger.get_errors().size() == 2);
    REQUIRE(logger.get_errors().at(0) == E_CONFIG);
    REQUIRE(logger.get_errors().at(1) == E_TEST_ERROR);

    ErrorLogger::inst().clear_errors();
}

TEST_CASE("Scanner", "[scanner]") {
    // FIXME: This last test resulted in a segmentation fault.
    std::string source_code = "var x = 5";
    std::string file_name = "test_files/scanner_test.nit";
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);
    std::shared_ptr<std::string> filename = std::make_shared<std::string>(file_name);
    Scanner scanner;
    scanner.scan_file(filename, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens.at(0).tok_type == KW_VAR);
    REQUIRE(tokens.at(1).tok_type == TOK_IDENT);
    REQUIRE(tokens.at(2).tok_type == TOK_EQ);
    REQUIRE(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 5);
}
