#include "../src/logger/logger.h"
#include "../src/parser/ast_printer.h"
#include "../src/parser/decl.h"
#include "../src/parser/expr.h"
#include "../src/parser/parser.h"
#include "../src/parser/stmt.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "catch/catch_amalgamated.hpp"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

// MARK: Main tests

TEST_CASE("Parser var decls", "[parser]") {
    std::string source_code = "var x = 5; var y: i32 = 10;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/var_decls_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x auto 5)");
    CHECK(printer.print(stmts.at(1)) == "(decl:var y i32 10)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser var decls 2", "[parser]") {
    std::string source_code = "var x: i32; var y: CustomType; var z: std::stack;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/var_decls_test_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x i32)");
    CHECK(printer.print(stmts.at(1)) == "(decl:var y CustomType)");
    CHECK(printer.print(stmts.at(2)) == "(decl:var z std::stack)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser advanced type annotations", "[parser]") {
    std::string source_code = "var x: std::pair<std::pair<i32, i32>, i32>;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/advanced_type_annotations_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x std::pair<std::pair<i32, i32>, i32>)");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser rptr type", "[parser]") {
    std::string source_code = "var x: i32*;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/rptr_type_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x i32*)");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser array type", "[parser]") {
    std::string source_code = "var x: i32[];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/array_type_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x i32[])");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser tuple type", "[parser]") {
    std::string source_code = "var x: (i32, i32); var y: ();";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/tuple_type_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x (i32, i32))");
    CHECK(printer.print(stmts.at(1)) == "(decl:var y ())");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser fptr type", "[parser]") {
    std::string source_code = "var x: fun(i32) => i64; var y: fun() => i32; var z: fun() => void;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_type_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(decl:var x fun(i32) => i64)");
    CHECK(printer.print(stmts.at(1)) == "(decl:var y fun() => i32)");
    CHECK(printer.print(stmts.at(2)) == "(decl:var z fun() => void)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser constants", "[parser]") {
    std::string source_code = "const x = 5; const y: i32 = 10;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/const_decls_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();
    std::shared_ptr<Stmt> stmt = stmts.at(0);

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmt) == "(decl:const x auto 5)");
    CHECK(printer.print(stmts.at(1)) == "(decl:const y i32 10)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser fun decls", "[parser]") {
    std::string source_code = "fun foo() {}";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_decls_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:fun foo fun() => void { })");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser fun decls 2", "[parser]") {
    std::string source_code = "fun foo() { return; }";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_decls_test_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();
    std::shared_ptr<Stmt> stmt = stmts.at(0);

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmt) == "(decl:fun foo fun() => void { (stmt:return) })");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser fun decls 3", "[parser]") {
    std::string source_code = "fun foo(): i32 { return 5; }";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_decls_test_3.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();
    std::shared_ptr<Stmt> stmt = stmts.at(0);

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmt) == "(decl:fun foo fun() => i32 { (stmt:return 5) })");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser fun decls 4", "[parser]") {
    std::string source_code = "fun foo(a: i32): i32 { return a; }";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_decls_test_4.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();
    std::shared_ptr<Stmt> stmt = stmts.at(0);

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:fun foo fun(i32) => i32 (decl:const a i32) { (stmt:return a) })");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser fun decls 5", "[parser]") {
    std::string source_code = "fun foo(a: i32): i32 { var b = a; return a + b; }";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/fun_decls_test_5.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(decl:fun foo fun(i32) => i32 (decl:const a i32) { (decl:var b auto a) (stmt:return (+ a b)) })");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser print stmts", "[parser]") {
    std::string source_code = "puts 5; puts \"Hello, world!\";";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/print_stmts_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(stmt:print 5)");
    CHECK(printer.print(stmts.at(1)) == "(stmt:print \"Hello, world!\")");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

// MARK: Error tests

TEST_CASE("Logger no lparen in fun decl", "[logger]") {
    std::string source_code = "fun foo {}";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/no_lparen_in_fun_decl_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_LPAREN_IN_FUN_DECL);

    logger.reset();
}

TEST_CASE("Logger no lbrace in fun decl", "[logger]") {
    std::string source_code = "fun foo()";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/no_lbrace_in_fun_decl_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_LBRACE_IN_FUN_DECL);

    logger.reset();
}