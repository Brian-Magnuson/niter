#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "../src/checker/environment.h"
#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"

static void setup(const std::string& source_code, const std::string& file_name, bool set_printing_enabled) {
    auto source_code_ptr = std::make_shared<std::string>(source_code);
    auto file_name_ptr = std::make_shared<std::string>(file_name);

    ErrorLogger::inst().set_printing_enabled(set_printing_enabled);

    Scanner scanner;
    scanner.scan_file(file_name_ptr, source_code_ptr);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());
    GlobalChecker global_checker;
    global_checker.type_check(stmts);
    LocalChecker local_checker;
    local_checker.type_check(stmts);
}

static void cleanup() {
    Environment& env = Environment::inst();
    env.reset();
    ErrorLogger& logger = ErrorLogger::inst();
    logger.reset();
}

// MARK: Local Checker Tests

TEST_CASE("Local checker tuples", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    return 0; 
}
)";
    setup(source_code, "test_files/tuples.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Local checker incompatible tuples", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, 1);
    return 0; 
}
)";
    setup(source_code, "test_files/incompatible_tuples.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INCOMPATIBLE_TYPES);

    cleanup();
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
    setup(source_code, "test_files/tuple_indexing.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Local checker tuple indexing out of bounds", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var a: (i32, bool) = (1, true);
    a[2];
    return 0; 
}
)";
    setup(source_code, "test_files/tuple_indexing_out_of_bounds.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_TUPLE_INDEX_OUT_OF_RANGE);

    cleanup();
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
    setup(source_code, "test_files/tuple_no_literal_index.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_LITERAL_INDEX_ON_TUPLE);

    cleanup();
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
    setup(source_code, "test_files/new_struct.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
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
    setup(source_code, "test_files/new_struct_wrong_field.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_STRUCT_MEMBER);

    cleanup();
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
    setup(source_code, "test_files/new_struct_missing_field.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_MISSING_FIELD_IN_OBJ);

    cleanup();
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
    setup(source_code, "test_files/struct_fields_optional.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
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
    setup(source_code, "test_files/struct_field_type_mismatch.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INCOMPATIBLE_TYPES);

    cleanup();
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
    setup(source_code, "test_files/struct_usage.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
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
    setup(source_code, "test_files/struct_mutation.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
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
    setup(source_code, "test_files/struct_mutation_wrong_field.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_STRUCT_MEMBER);

    cleanup();
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
    setup(source_code, "test_files/struct_mutation_wrong_type.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INCOMPATIBLE_TYPES);

    cleanup();
}

TEST_CASE("Local checker inconsistent array types", "[checker]") {
    std::string source_code = "fun main(): i32 { var arr = [1, 2, true]; return 0; }";

    setup(source_code, "test_files/inconsistent_array_types.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INCONSISTENT_ARRAY_TYPES);

    cleanup();
}

TEST_CASE("Local checker indeterminate array type", "[checker]") {
    std::string source_code = "fun main(): i32 { var arr = []; return 0; }";

    setup(source_code, "test_files/indeterminate_arr_type.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INDETERMINATE_ARRAY_TYPE);

    cleanup();
}

TEST_CASE("Local checker array size bad cast", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var arr: [i32; *] = [1, 2, 3];
    var arr2: [i32; 2] = arr;
    return 0;
}
)";
    setup(source_code, "test_files/array_size_bad_cast.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_ARRAY_SIZE_UNKNOWN);

    cleanup();
}

TEST_CASE("Local checker sized array without initializer", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var arr: [i32; 2];
    return 0;
}
)";
    setup(source_code, "test_files/sized_array_no_initializer.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_SIZED_ARRAY_WITHOUT_INITIALIZER);

    cleanup();
}

TEST_CASE("Local checker array generator", "[checker]") {
    std::string source_code = R"(
fun main(): i32 {
    var arr: [i32; 5] = [0; 5];
    return 0;
}
)";
    setup(source_code, "test_files/array_generator.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Local checker new constructor", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32

    fun new(): Point {
        return :Point {x: 0, y: 0}
    }
}

fun main(): i32 {
    var a: Point = Point::new()
    return 0
}
)";
    setup(source_code, "test_files/new_constructor.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Local checker instance method", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32

    fun add_parts(this: Point*): i32 {
        return this->x + this->y
    }
}

fun main(): i32 {
    var a: Point = :Point {x: 0, y: 0}
    a.add_parts()
    return 0
}
)";
    setup(source_code, "test_files/instance_method.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Local checker mutating fun immutable param", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32

    // This function should not be allowed to mutate the Point object
    fun reset_x(this: Point*): i32 {
        this->x = 0
    }
}

fun main(): i32 {
    return 0
}
)";
    setup(source_code, "test_files/mutating_function_immutable_param.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_ASSIGN_TO_CONST);

    cleanup();
}

TEST_CASE("Local checker mutating fun mutable param", "[checker]") {
    std::string source_code = R"(
struct Point {
    var x: i32
    var y: i32

    fun reset_x(var this: Point*) {
        this->x = 0
    }
}

fun main(): i32 {
    return 0
}
)";
    setup(source_code, "test_files/mutating_function_mutable_param.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}
