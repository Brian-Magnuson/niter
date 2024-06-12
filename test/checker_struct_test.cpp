#include "../src/checker/environment.h"
#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "catch/catch_amalgamated.hpp"
#include <memory>
#include <string>

// MARK: Local Checker Tests

TEST_CASE("Local checker tuples", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    return 0; 
}
)";
    auto file_name = std::make_shared<std::string>("test_files/tuples.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker incompatible tuples", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, 1);
    return 0; 
}
)";
    auto file_name = std::make_shared<std::string>("test_files/incompatible_tuples.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
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

TEST_CASE("Local checker tuple indexing", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    var b: i32 = a[0];
    var c: bool = a[1];
    return 0; 
}
)";
    auto file_name = std::make_shared<std::string>("test_files/tuple_indexing.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker tuple indexing out of bounds", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    a[2];
    return 0; 
}
)";
    auto file_name = std::make_shared<std::string>("test_files/tuple_indexing_out_of_bounds.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_TUPLE_INDEX_OUT_OF_RANGE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker tuple without literal index", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    var i: i32 = 0;
    a[i];
    return 0; 
}
)";
    auto file_name = std::make_shared<std::string>("test_files/tuple_no_literal_index.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_LITERAL_INDEX_ON_TUPLE);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker new struct", "[checker]") {
    std::string source_code = R"(

struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: 2}
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/new_struct.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker new struct with wrong field", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, z: 2}
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/new_struct_wrong_field.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_STRUCT_MEMBER);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker new struct missing field", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
    var z: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1}
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/new_struct_missing_field.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_MISSING_FIELD_IN_OBJ);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker struct fields optional", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32 = 0
    var y = 0
    var z: i32
}

fun main(): i32 {
    var a: Point = :Point {z: 1}
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_fields_optional.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker struct field type mismatch", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: true}
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_field_type_mismatch.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
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

TEST_CASE("Local checker struct usage", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: 2}
    var b: i32 = a.x
    var c: i32 = a.y
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_usage.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker struct mutation", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: 2}
    a.x = 3
    a.y = 4
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_mutation.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() == 0);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker struct mutation with wrong field", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: 2}
    a.z = 3
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_mutation_wrong_field.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_STRUCT_MEMBER);

    logger.reset();
    env.reset();
}

TEST_CASE("Local checker struct mutation with wrong type", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32
}

fun main(): i32 {
    var a: Point = :Point {x: 1, y: 2}
    a.x = true
    return 0
}
)";
    auto file_name = std::make_shared<std::string>("test_files/struct_mutation_wrong_type.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
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

TEST_CASE("Local checker inconsistent array types", "[checker]") {
    std::string source_code = "fun main(): i32 { var arr = [1, 2, true]; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/inconsistent_array_types.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());

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

TEST_CASE("Local checker indeterminate array type", "[checker]") {
    std::string source_code = "fun main(): i32 { var arr = []; return 0; }";
    auto file_name = std::make_shared<std::string>("test_files/indeterminate_arr_type.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);
    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    auto stmts = parser.parse(scanner.get_tokens());
    Environment& env = Environment::inst();
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INDETERMINATE_ARRAY_TYPE);

    logger.reset();
    env.reset();
}
