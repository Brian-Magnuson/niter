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

TEST_CASE("Global checker extern fun named main", "[checker]") {
    std::string source_code = "extern fun main();";
    auto file_name = std::make_shared<std::string>("test_files/extern_fun_named_main.nit");

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

TEST_CASE("Global checker valid extern fun", "[checker]") {
    std::string source_code = "extern fun foo(i32): i32;";
    auto file_name = std::make_shared<std::string>("test_files/valid_extern_fun.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    Environment& env = Environment::inst();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

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
    std::string source_code = R"(
fun main(): i32 { 
    var a: i32 = 1; 
    a = true; 
    return 0;
}
)";
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

TEST_CASE("Local checker invalid ptr declarer 2", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    const a = 1
    var b: i32* = &a
    return 0
}
)";

    auto file_name = std::make_shared<std::string>("test_files/invalid_ptr_declarer_2.nit");

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

    REQUIRE(logger.get_errors().size() == 1);
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

TEST_CASE("Local checker dup param names", "[checker]") {
    std::string source_code = "fun add(a: i32, a: i32): i32 { return a + a; }";
    auto file_name = std::make_shared<std::string>("test_files/dup_param_names.nit");

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
    CHECK(logger.get_errors().at(0) == E_DUPLICATE_PARAM_NAME);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker return incompatible", "[checker]") {
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

TEST_CASE("Local checker return in void fun", "[checker]") {
    std::string source_code = "fun do_nothing() { return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/return_in_void_fun.nit");

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

TEST_CASE("Local checker no ident after dot") {
    std::string source_code = "fun main(): i32 { var a: i32; a.true; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/no_ident_after_dot.nit");

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
    CHECK(logger.get_errors().at(0) == E_NO_IDENT_AFTER_DOT);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker arrow on non-pointer", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: i32; a->\"hello\"; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/no_ident_after_arrow.nit");

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
    CHECK(logger.get_errors().at(0) == E_DEREFERENCE_NON_POINTER);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker no ident after arrow", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: i32*; a->\"hello\"; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/no_ident_after_arrow.nit");

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
    CHECK(logger.get_errors().at(0) == E_NO_IDENT_AFTER_DOT);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker access on non struct", "[checker]") {
    std::string source_code = "fun main(): i32 { main.b; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/access_on_non_struct.nit");

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
    CHECK(logger.get_errors().at(0) == E_ACCESS_ON_NON_STRUCT);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker call on non fun", "[checker]") {
    std::string source_code = "fun main(): i32 { var a: i32; a(); return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/call_on_non_fun.nit");

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
    CHECK(logger.get_errors().at(0) == E_CALL_ON_NON_FUN);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker invalid arity", "[checker]") {
    std::string source_code = R"(
fun add(a: i32, b: i32): i32 {
    return a + b
}

fun main(): i32 {
    add(1, 2, 3)
    return 0
}
)";

    auto file_name = std::make_shared<std::string>("test_files/invalid_arity.nit");

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
    CHECK(logger.get_errors().at(0) == E_INVALID_ARITY);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker invalid param type", "[checker]") {
    std::string source_code = R"(
fun add(a: i32, b: i32): i32 {
    return a + b
}

fun main(): i32 {
    add(1, true)
    return 0
}
)";

    auto file_name = std::make_shared<std::string>("test_files/invalid_param_type.nit");

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

TEST_CASE("Local checker valid call", "[checker]") {
    std::string source_code = R"(
fun add(a: i32, b: i32): i32 {
    return a + b
}

fun main(): i32 {
    add(1, 2)
    return 0
}
)";

    auto file_name = std::make_shared<std::string>("test_files/valid_call.nit");

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

TEST_CASE("Local checker valid extern fun call", "[checker]") {
    std::string source_code = R"(
extern fun add(i32, i32): i32

fun main(): i32 {
    add(1, 2)
    return 0
}
)";

    auto file_name = std::make_shared<std::string>("test_files/valid_extern_fun_call.nit");

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

TEST_CASE("Local checker address of non lvalue", "[checker]") {
    std::string source_code = "fun main(): i32 { &1; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/address_of_non_lvalue.nit");

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
    CHECK(logger.get_errors().at(0) == E_ADDRESS_OF_NON_LVALUE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker assign to non lvalue", "[checker]") {
    std::string source_code = "fun main(): i32 { 1 = 2; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/assign_to_non_lvalue.nit");

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
    CHECK(logger.get_errors().at(0) == E_ASSIGN_TO_NON_LVALUE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker assign to const", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    const my_constant = 1;
    my_constant = 2;
    return 0;
}
)";
    auto file_name = std::make_shared<std::string>("test_files/assign_to_const.nit");

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
    CHECK(logger.get_errors().at(0) == E_ASSIGN_TO_CONST);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker deref and assign", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: i32 = 1;
    var b: i32* = &a;
    *b = 2;
    return 0;
}
)";
    auto file_name = std::make_shared<std::string>("test_files/deref_and_assign.nit");

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

TEST_CASE("Local checker undeclared variable", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    a = 1;
    return 0;
}
)";
    auto file_name = std::make_shared<std::string>("test_files/undeclared_variable.nit");

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
    CHECK(logger.get_errors().at(0) == E_UNKNOWN_VAR);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker invalid cast", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: i32 = 1;
    var b: i32* = (a as i32*);
    return 0;
}
)";
    auto file_name = std::make_shared<std::string>("test_files/invalid_cast.nit");

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
    REQUIRE(logger.get_errors().size() == 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_CAST);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker valid cast", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: i32 = 1;
    var b: f64 = (a as f64);
    return 0;
}
)";
    auto file_name = std::make_shared<std::string>("test_files/valid_cast.nit");

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
