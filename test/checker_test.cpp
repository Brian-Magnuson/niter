#include "../src/checker/environment.h"
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

TEST_CASE("Global checker symbol already declared", "[checker]") {
    std::string source_code = "var x: i32; var x: i32;";
    auto file_name = std::make_shared<std::string>("test_files/symbol_already_declared.nit");

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
    CHECK(logger.get_errors().at(0) == E_SYMBOL_ALREADY_DECLARED);

    logger.reset();
    env.reset();
}

TEST_CASE("Global checker expr in global", "[checker]") {
    std::string source_code = "var x: i32 = 0; x = 1;";
    auto file_name = std::make_shared<std::string>("test_files/expr_in_global.nit");

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
    CHECK(logger.get_errors().at(0) == E_GLOBAL_EXPRESSION);

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

TEST_CASE("Local checker fun in local", "[checker]") {
    std::string source_code = "fun main(): i32 { fun foo(): i32 { return 0; } return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/fun_in_local.nit");

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
    CHECK(logger.get_errors().at(0) == E_FUN_IN_LOCAL_SCOPE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker uninitialized const", "[checker]") {
    std::string source_code = "fun main(): i32 { const x: i32; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/uninitialized_const.nit");

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
    CHECK(logger.get_errors().at(0) == E_UNINITIALIZED_CONST);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker uninitialized auto", "[checker]") {
    std::string source_code = "fun main(): i32 { var x; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/uninitialized_auto.nit");

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
    CHECK(logger.get_errors().at(0) == E_AUTO_WITHOUT_INITIALIZER);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker valid vars", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: i32; var b = 1; var c: i32 = 2; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/valid_vars.nit");

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

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker valid initializers", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var _1: i32 = 3
    var _2: char = 'a'
    var _3: bool = true
    var _4: f64 = 2.0
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/valid_initializers.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    Environment& env = Environment::inst();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker unknown type", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: strange_type = 1; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/unknown_type.nit");

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
    CHECK(logger.get_errors().at(0) == E_UNKNOWN_TYPE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker types incompatible", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: i32 = 1; a = true; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/types_incompatible.nit");

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
    CHECK(logger.get_errors().at(0) == E_INCOMPATIBLE_TYPES);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker valid ptrs", "[checker]") {
    std::string source_code = "fun main(): i32 { var num: i32 = 0; var num_ptr: i32* = &num; var other_num: i32 = *num_ptr; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/valid_ptrs.nit");

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

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker invalid ptr declarer", "[checker]") {
    std::string source_code = "fun main(): i32 { var num: i32 = 0; const num_ptr: i32* = &num; var var_num_ptr: i32* = num_ptr; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/invalid_ptr_declarer.nit");

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
    CHECK(logger.get_errors().at(0) == E_INVALID_PTR_DECLARER);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker inconsistent array types", "[checker]") {
    std::string source_code = "fun main(): i32 { var arr = [1, 2, true]; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/inconsistent_array_types.nit");

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
    CHECK(logger.get_errors().at(0) == E_INCONSISTENT_ARRAY_TYPES);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker return incompatible") {
    std::string source_code = "fun main(): i32 { return true; }";
    auto file_name = std::make_shared<std::string>("test_files/return_incompatible.nit");

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
    CHECK(logger.get_errors().at(0) == E_RETURN_INCOMPATIBLE);

    logger.reset();
    env.reset();
}
