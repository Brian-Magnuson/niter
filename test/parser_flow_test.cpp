#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "../src/logger/logger.h"
#include "../src/parser/ast_printer.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "../src/utility/stmt.h"

static std::vector<std::shared_ptr<Stmt>> run_parser(const std::string& source_code, const std::string& file_name) {
    auto source_code_ptr = std::make_shared<std::string>(source_code);
    auto file_name_ptr = std::make_shared<std::string>(file_name);

    Scanner scanner;
    scanner.scan_file(file_name_ptr, source_code_ptr);
    Parser parser;
    return parser.parse(scanner.get_tokens());
}

static void cleanup() {
    ErrorLogger& logger = ErrorLogger::inst();
    logger.reset();
}

TEST_CASE("Parser if stmt", "[parser]") {
    std::string source_code = "if true { x = 1; }\n";

    auto stmts = run_parser(source_code, "test_files/if_stmt.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if stmt 2", "[parser]") {
    std::string source_code = "if true x = 1\n";

    auto stmts = run_parser(source_code, "test_files/if_stmt_2.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if stmt 3", "[parser]") {
    std::string source_code = R"(
if true
    x = 1

if false {
    x = 2
}
)";

    auto stmts = run_parser(source_code, "test_files/if_stmt_3.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 2) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else stmt", "[parser]") {
    std::string source_code = "if true { x = 1; } else { x = 2; }\n";

    auto stmts = run_parser(source_code, "test_files/if_else_stmt.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else stmt 2", "[parser]") {
    std::string source_code = "if true x = 1; else x = 2\n";

    auto stmts = run_parser(source_code, "test_files/if_else_stmt_2.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else stmt 3", "[parser]") {
    std::string source_code = R"(
if true
    x = 1
else
    x = 2

if false {
    x = 1
} else {
    x = 2
}
)";
    auto stmts = run_parser(source_code, "test_files/if_else_stmt_3.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if stmt", "[parser]") {
    std::string source_code = "if true { x = 1; } else if false { x = 2; }\n";

    auto stmts = run_parser(source_code, "test_files/if_else_if_stmt.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if stmt 2", "[parser]") {
    std::string source_code = "if true x = 1; else if false x = 2\n";

    auto stmts = run_parser(source_code, "test_files/if_else_if_stmt_2.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if stmt 3", "[parser]") {
    std::string source_code = R"(
if true
    x = 1
else if false
    x = 2

if false {
    x = 1
} else if true {
    x = 2
}
)";
    auto stmts = run_parser(source_code, "test_files/if_else_if_stmt_3.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (stmt:if true { (= x 2) }) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if stmt 4", "[parser]") {
    std::string source_code = R"(
if true
    x = 1
else if false
    x = 2
if true
    x = 3

)";
    auto stmts = run_parser(source_code, "test_files/if_else_if_stmt_4.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if true { (= x 3) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if else stmt", "[parser]") {
    std::string source_code = "if true { x = 1; } else if false { x = 2; } else { x = 3; }\n";

    auto stmts = run_parser(source_code, "test_files/if_else_if_else_stmt.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if else stmt 2", "[parser]") {
    std::string source_code = "if true x = 1; else if false x = 2; else x = 3\n";

    auto stmts = run_parser(source_code, "test_files/if_else_if_else_stmt_2.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser if else if else stmt 3", "[parser]") {
    std::string source_code = R"(
if true
    x = 1
else if false
    x = 2
else
    x = 3

if false {
    x = 1
} else if true {
    x = 2
} else {
    x = 3
}
)";
    auto stmts = run_parser(source_code, "test_files/if_else_if_else_stmt_3.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (stmt:if true { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser while stmt", "[parser]") {
    std::string source_code = "while true { x = 1; }\n";

    auto stmts = run_parser(source_code, "test_files/while_stmt.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:while true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser while stmt 2", "[parser]") {
    std::string source_code = "while true x = 1\n";

    auto stmts = run_parser(source_code, "test_files/while_stmt_2.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:while true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");

    cleanup();
}

TEST_CASE("Parser while stmt 3", "[parser]") {
    std::string source_code = R"(
while true
    x = 1

while false {
    x = 2
}
)";

    auto stmts = run_parser(source_code, "test_files/while_stmt_3.nit");

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:while true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:while false { (= x 2) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");

    cleanup();
}
