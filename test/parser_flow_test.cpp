#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include "../src/logger/logger.h"
#include "../src/parser/ast_printer.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "../src/utility/stmt.h"

TEST_CASE("Parser if stmt", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true { x = 1; }\n");
    auto file_name = std::make_shared<std::string>("test_files/if_stmt.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if stmt 2", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true x = 1\n");
    auto file_name = std::make_shared<std::string>("test_files/if_stmt_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if stmt 3", "[parser]") {
    auto source_code = std::make_shared<std::string>(R"(
if true
    x = 1

if false {
    x = 2
}
)");
    auto file_name = std::make_shared<std::string>("test_files/if_stmt_3.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 2) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");
}

TEST_CASE("Parser if else stmt", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true { x = 1; } else { x = 2; }\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_stmt.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else stmt 2", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true x = 1; else x = 2\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_stmt_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else stmt 3", "[parser]") {
    auto source_code = std::make_shared<std::string>(R"(
if true
    x = 1
else
    x = 2

if false {
    x = 1
} else {
    x = 2
}
)");
    auto file_name = std::make_shared<std::string>("test_files/if_else_stmt_3.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (= x 2) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if stmt", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true { x = 1; } else if false { x = 2; }\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_stmt.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if stmt 2", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true x = 1; else if false x = 2\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_stmt_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if stmt 3", "[parser]") {
    auto source_code = std::make_shared<std::string>(R"(
if true
    x = 1
else if false
    x = 2

if false {
    x = 1
} else if true {
    x = 2
}
)");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_stmt_3.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (stmt:if true { (= x 2) }) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if stmt 4", "[parser]") {
    auto source_code = std::make_shared<std::string>(R"(
if true
    x = 1
else if false
    x = 2
if true
    x = 3

)");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_stmt_4.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if true { (= x 3) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if else stmt", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true { x = 1; } else if false { x = 2; } else { x = 3; }\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_else_stmt.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if else stmt 2", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true x = 1; else if false x = 2; else x = 3\n");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_else_stmt_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}

TEST_CASE("Parser if else if else stmt 3", "[parser]") {
    auto source_code = std::make_shared<std::string>(R"(
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
)");
    auto file_name = std::make_shared<std::string>("test_files/if_else_if_else_stmt_3.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1) } else { (stmt:if false { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[1]) == "(stmt:if false { (= x 1) } else { (stmt:if true { (= x 2) } else { (= x 3) }) })");
    CHECK(printer.print(stmts[2]) == "(stmt:eof)");
}
