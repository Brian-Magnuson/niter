#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "../src/checker/environment.h"
#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"

void setup(const std::string& source_code, const std::string& file_name, bool set_printing_enabled) {
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

void cleanup() {
    Environment& env = Environment::inst();
    env.reset();
    ErrorLogger& logger = ErrorLogger::inst();
    logger.reset();
}

TEST_CASE("Checker if stmt", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true {
            x = 1;
        }
        return x;
    }
)";

    setup(source_code, "test_files/if_stmt.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Checker if stmt 2", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            x = 1
        return x;
    }
)";

    setup(source_code, "test_files/if_stmt_2.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Checker if stmt 3", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            x = 1
        else
            x = 2
        return x;
    }
)";

    setup(source_code, "test_files/if_stmt_3.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Checker if stmt 4", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            return 0
        else
            return 1
    }
)";

    setup(source_code, "test_files/if_else_stmt.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Checker if stmt 5", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            x = 1
        else
            x = true
    }
)";

    setup(source_code, "test_files/if_else_stmt_2.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 1);
    CHECK(logger.get_errors().at(0) == E_INCOMPATIBLE_TYPES);

    cleanup();
}

TEST_CASE("Checker if stmt 6", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            return x
        else
            return true
    }
)";

    setup(source_code, "test_files/if_else_stmt_3.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 1);
    CHECK(logger.get_errors().at(0) == E_INCONSISTENT_RETURN_TYPES);

    cleanup();
}

TEST_CASE("Checker if stmt 7", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if x
            x = 0
        else
            x = 1
    }
)";

    setup(source_code, "test_files/if_else_stmt_4.nit", false);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 1);
    CHECK(logger.get_errors().at(0) == E_CONDITIONAL_WITHOUT_BOOL);

    cleanup();
}

TEST_CASE("Checker if stmt 8", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            x = 1
        else if false
            x = 2

        return 0
    }
)";

    setup(source_code, "test_files/if_else_if_stmt.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}

TEST_CASE("Checker if stmt 9", "[checker]") {

    std::string source_code = R"(
    fun main(): i32 {
        var x: i32;
        if true
            x = 1
        else if false
            x = 2
        else
            x = 3

        return 0
    }
)";

    setup(source_code, "test_files/if_else_if_stmt_2.nit", true);

    ErrorLogger& logger = ErrorLogger::inst();
    REQUIRE(logger.get_errors().size() == 0);

    cleanup();
}
