#include "../src/checker/global_checker.h"
#include "../src/checker/local_checker.h"
#include "../src/logger/logger.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "catch/catch_amalgamated.hpp"
#include <memory>
#include <string>

TEST_CASE("Checker", "[checker]") {
    std::string source_code = "fun main() { return; }";
    std::shared_ptr file_name = std::make_shared<std::string>("test");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    auto stmts = parser.parse();

    GlobalChecker global_checker;
    global_checker.type_check(stmts);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_MAIN_SIGNATURE);

    logger.reset();
}
