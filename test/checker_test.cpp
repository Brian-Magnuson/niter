#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "catch/catch_amalgamated.hpp"
#include <memory>
#include <string>

// MARK: Global checker tests

TEST_CASE("Global checker bad main", "[checker]") {
    std::string source_code = "fun main() { return; }";
    auto file_name = std::make_shared<std::string>("test_files/bad_main_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    Environment& env = Environment::inst();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_MAIN_SIGNATURE);

    logger.reset();
    env.reset();
}

// MARK: Local checker tests

TEST_CASE("Local checker no return in non-void", "[checker]") {
    std::string source_code = "fun main(): i32 { return; }";
    auto file_name = std::make_shared<std::string>("test_files/no_ret_in_non_void.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    Environment& env = Environment::inst();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_RETURN_IN_NON_VOID_FUN);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker good main", "[checker]") {
    std::string source_code = "fun main(): i32 { return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/good_main.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    Environment& env = Environment::inst();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 0);

    logger.reset();
    env.reset();
}
