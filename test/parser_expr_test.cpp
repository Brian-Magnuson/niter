#include "../src/logger/logger.h"
#include "../src/parser/ast_printer.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "../src/utility/decl.h"
#include "../src/utility/expr.h"
#include "../src/utility/stmt.h"
#include "catch/catch_amalgamated.hpp"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

// MARK: Main tests

TEST_CASE("Parser", "[parser]") {
    std::string source_code = "x = 5\n";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/parser_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    std::string expected = "(= x 5)";
    REQUIRE(printer.print(stmts.at(0)) == expected);
}

TEST_CASE("Parser multiple expression stmts", "[parser]") {
    std::string source_code = "x = 5\ny = 10; z = 15\n";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/multiple_expr_stmts_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(= x 5)");
    CHECK(printer.print(stmts.at(1)) == "(= y 10)");
    CHECK(printer.print(stmts.at(2)) == "(= z 15)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser literal exprs", "[parser]") {
    std::string source_code = "5; 5.5; true; false; nil; 'a'; \"Hello, world!\";";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/literal_exprs_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 8);
    CHECK(printer.print(stmts.at(0)) == "5");
    CHECK(printer.print(stmts.at(1)) == "5.5000");
    CHECK(printer.print(stmts.at(2)) == "true");
    CHECK(printer.print(stmts.at(3)) == "false");
    CHECK(printer.print(stmts.at(4)) == "nil");
    CHECK(printer.print(stmts.at(5)) == "'a'");
    CHECK(printer.print(stmts.at(6)) == "\"Hello, world!\"");
    CHECK(printer.print(stmts.at(7)) == "(stmt:eof)");
}

TEST_CASE("Parser arrays", "[parser]") {
    std::string source_code = "[]; [1]; [1,2]; [1,2,];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/arrays_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 5);
    CHECK(printer.print(stmts.at(0)) == "(array)");
    CHECK(printer.print(stmts.at(1)) == "(array 1)");
    CHECK(printer.print(stmts.at(2)) == "(array 1 2)");
    CHECK(printer.print(stmts.at(3)) == "(array 1 2)");
    CHECK(printer.print(stmts.at(4)) == "(stmt:eof)");
}

TEST_CASE("Parser nested arrays", "[parser]") {
    std::string source_code = "[[1],]; [[1,2],]; [[1],[2,],];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/nested_arrays_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(array (array 1))");
    CHECK(printer.print(stmts.at(1)) == "(array (array 1 2))");
    CHECK(printer.print(stmts.at(2)) == "(array (array 1) (array 2))");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser tuples", "[parser]") {
    std::string source_code = "(); (1); (1,); (1,2); (1,2,);";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/tuples_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 6);
    CHECK(printer.print(stmts.at(0)) == "(tuple)");
    CHECK(printer.print(stmts.at(1)) == "1");
    CHECK(printer.print(stmts.at(2)) == "(tuple 1)");
    CHECK(printer.print(stmts.at(3)) == "(tuple 1 2)");
    CHECK(printer.print(stmts.at(4)) == "(tuple 1 2)");
    CHECK(printer.print(stmts.at(5)) == "(stmt:eof)");
}

TEST_CASE("Parser nested tuples", "[parser]") {
    std::string source_code = "((1,),); ((1,2),); ((1,),(2,));";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/nested_tuples_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(tuple (tuple 1))");
    CHECK(printer.print(stmts.at(1)) == "(tuple (tuple 1 2))");
    CHECK(printer.print(stmts.at(2)) == "(tuple (tuple 1) (tuple 2))");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser nested tuples and arrays", "[parser]") {
    std::string source_code = "([1]); [(1)]; ([1,2],); [(1,2)]; [()];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/nested_tuples_and_arrays_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 6);
    CHECK(printer.print(stmts.at(0)) == "(array 1)");
    CHECK(printer.print(stmts.at(1)) == "(array 1)");
    CHECK(printer.print(stmts.at(2)) == "(tuple (array 1 2))");
    CHECK(printer.print(stmts.at(3)) == "(array (tuple 1 2))");
    CHECK(printer.print(stmts.at(4)) == "(array (tuple))");
    CHECK(printer.print(stmts.at(5)) == "(stmt:eof)");
}

TEST_CASE("Parser call exprs", "[parser]") {
    std::string source_code = "foo(); foo(1); foo(1,2); foo(1,2,);";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/call_exprs_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 5);
    CHECK(printer.print(stmts.at(0)) == "(call foo)");
    CHECK(printer.print(stmts.at(1)) == "(call foo 1)");
    CHECK(printer.print(stmts.at(2)) == "(call foo 1 2)");
    CHECK(printer.print(stmts.at(3)) == "(call foo 1 2)");
    CHECK(printer.print(stmts.at(4)) == "(stmt:eof)");
}

TEST_CASE("Parser chained calls", "[parser]") {
    std::string source_code = "foo()(); bar(1)(2,3);";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/chained_calls_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(call (call foo))");
    CHECK(printer.print(stmts.at(1)) == "(call (call bar 1) 2 3)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser access exprs", "[parser]") {
    std::string source_code = "foo.bar; foo->bar; foo[1];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/access_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(. foo bar)");
    CHECK(printer.print(stmts.at(1)) == "(. (* foo) bar)");
    CHECK(printer.print(stmts.at(2)) == "([] foo 1)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser chained access exprs", "[parser]") {
    std::string source_code = "foo.bar.baz; foo->bar->baz; foo[1][2];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/chained_access_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(. (. foo bar) baz)");
    CHECK(printer.print(stmts.at(1)) == "(. (* (. (* foo) bar)) baz)");
    CHECK(printer.print(stmts.at(2)) == "([] ([] foo 1) 2)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser chained access with grouping", "[parser]") {
    std::string source_code = "foo[foo[1]];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/chained_access_with_grouping_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "([] foo ([] foo 1))");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser chained access exprs 2", "[parser]") {
    std::string source_code = "one.two->three[four].five[six]->seven;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/chained_access_exprs_2_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(. (* ([] (. ([] (. (* (. one two)) three) four) five) six)) seven)");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");
}

TEST_CASE("Parser unary exprs", "[parser]") {
    std::string source_code = "-5; !true; *foo; &bar; -&foo; !*bar;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unary_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 7);
    CHECK(printer.print(stmts.at(0)) == "(- 5)");
    CHECK(printer.print(stmts.at(1)) == "(! true)");
    CHECK(printer.print(stmts.at(2)) == "(* foo)");
    CHECK(printer.print(stmts.at(3)) == "(& bar)");
    CHECK(printer.print(stmts.at(4)) == "(- (& foo))");
    CHECK(printer.print(stmts.at(5)) == "(! (* bar))");
    CHECK(printer.print(stmts.at(6)) == "(stmt:eof)");
}

TEST_CASE("Parser binary exprs", "[parser]") {
    std::string source_code = "1 + 2; 3 - 4; 5 * 6; 7 / 8; 9 % 10; 11 ^ 12;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/binary_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 7);
    CHECK(printer.print(stmts.at(0)) == "(+ 1 2)");
    CHECK(printer.print(stmts.at(1)) == "(- 3 4)");
    CHECK(printer.print(stmts.at(2)) == "(* 5 6)");
    CHECK(printer.print(stmts.at(3)) == "(/ 7 8)");
    CHECK(printer.print(stmts.at(4)) == "(% 9 10)");
    CHECK(printer.print(stmts.at(5)) == "(^ 11 12)");
    CHECK(printer.print(stmts.at(6)) == "(stmt:eof)");
}

TEST_CASE("Parser order of operations", "[parser]") {
    std::string source_code = "1 + 2 * 3; 1 * 2 ^ 3; 1 / 2 + 3; 1 / (2 + 3);";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/order_of_operations_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 5);
    CHECK(printer.print(stmts.at(0)) == "(+ 1 (* 2 3))");
    CHECK(printer.print(stmts.at(1)) == "(* 1 (^ 2 3))");
    CHECK(printer.print(stmts.at(2)) == "(+ (/ 1 2) 3)");
    CHECK(printer.print(stmts.at(3)) == "(/ 1 (+ 2 3))");
    CHECK(printer.print(stmts.at(4)) == "(stmt:eof)");
}

TEST_CASE("Parser comparison exprs", "[parser]") {
    std::string source_code = "1 == 2; 3 != 4; 5 < 6; 7 <= 8; 9 > 10; 11 >= 12;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/comparison_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 7);
    CHECK(printer.print(stmts.at(0)) == "(== 1 2)");
    CHECK(printer.print(stmts.at(1)) == "(!= 3 4)");
    CHECK(printer.print(stmts.at(2)) == "(< 5 6)");
    CHECK(printer.print(stmts.at(3)) == "(<= 7 8)");
    CHECK(printer.print(stmts.at(4)) == "(> 9 10)");
    CHECK(printer.print(stmts.at(5)) == "(>= 11 12)");
    CHECK(printer.print(stmts.at(6)) == "(stmt:eof)");
}

TEST_CASE("Parser comparison order of operations", "[parser]") {
    std::string source_code = "1 == 2 > 3; 1 < 2 == 3; 1 < 2 > 3;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/comparison_order_of_operations_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "(== 1 (> 2 3))");
    CHECK(printer.print(stmts.at(1)) == "(== (< 1 2) 3)");
    CHECK(printer.print(stmts.at(2)) == "(> (< 1 2) 3)");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser logical exprs", "[parser]") {
    std::string source_code = "true and false; true or false;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/logical_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(and true false)");
    CHECK(printer.print(stmts.at(1)) == "(or true false)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser logical order of operations", "[parser]") {
    std::string source_code = "true and false or true; true or false and true;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/logical_order_of_operations_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(or (and true false) true)");
    CHECK(printer.print(stmts.at(1)) == "(or true (and false true))");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser assign exprs", "[parser]") {
    std::string source_code = "x = 5; 1 = 2;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/assign_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(= x 5)");
    CHECK(printer.print(stmts.at(1)) == "(= 1 2)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser idents", "[parser]") {
    std::string source_code = "l1; l1::l2; l1::l2::l3;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/idents_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 4);
    CHECK(printer.print(stmts.at(0)) == "l1");
    CHECK(printer.print(stmts.at(1)) == "l1::l2");
    CHECK(printer.print(stmts.at(2)) == "l1::l2::l3");
    CHECK(printer.print(stmts.at(3)) == "(stmt:eof)");
}

TEST_CASE("Parser idents 2", "[parser]") {
    std::string source_code = "l1 + l2; l1::l2 + l3::l4;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/idents_test_2.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(+ l1 l2)");
    CHECK(printer.print(stmts.at(1)) == "(+ l1::l2 l3::l4)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

TEST_CASE("Parser cast exprs", "[parser]") {
    std::string source_code = "1 as i64; 2 as f64;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/cast_exprs_test.nit");

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 3);
    CHECK(printer.print(stmts.at(0)) == "(as 1 i64)");
    CHECK(printer.print(stmts.at(1)) == "(as 2 f64)");
    CHECK(printer.print(stmts.at(2)) == "(stmt:eof)");
}

// MARK: Error tests

TEST_CASE("Logger unmatched paren in grouping", "[logger]") {
    std::string source_code = "(1 + 2";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unmatched_paren_in_grouping_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNMATCHED_PAREN_IN_GROUPING);

    logger.reset();
}

TEST_CASE("Logger unmatched paren in call", "[logger]") {
    std::string source_code = "foo(1 + 2";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unmatched_paren_in_call_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNMATCHED_PAREN_IN_ARGS);

    logger.reset();
}

TEST_CASE("Logger unmatched bracket in array", "[logger]") {
    std::string source_code = "[1 + 2";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unmatched_bracket_in_array_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNMATCHED_LEFT_SQUARE);

    logger.reset();
}

TEST_CASE("Logger unmatched paren in tuple", "[logger]") {
    std::string source_code = "(1, 2";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unmatched_brace_in_tuple_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNMATCHED_PAREN_IN_TUPLE);

    logger.reset();
}

TEST_CASE("Logger missing stmt end", "[logger]") {
    std::string source_code = "1 + 2 3 + 4";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/missing_stmt_end_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_MISSING_STMT_END);

    logger.reset();
}

TEST_CASE("Logger not an ident", "[logger]") {
    std::string source_code = "my_var::1;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/not_an_ident_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NOT_AN_IDENTIFIER);

    logger.reset();
}

TEST_CASE("Logger not an expression", "[logger]") {
    std::string source_code = "1 +;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/not_an_expression_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    parser.parse();

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NOT_AN_EXPRESSION);

    logger.reset();
}

TEST_CASE("Logger not an annotation", "[logger]") {
    std::string source_code = "1 as 2;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/not_an_annotation_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser;
    parser.parse(scanner.get_tokens());

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INVALID_TYPE_ANNOTATION);

    logger.reset();
}

TEST_CASE("Logger insignificant newlines", "[logger]") {
    // A case where the logger *shouldn't* report an error
    std::string source_code = "arr = [1,\n2];";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/insignificant_newlines_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(logger.get_errors().size() == 0);
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(= arr (array 1 2))");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");

    logger.reset();
}

TEST_CASE("Logger insignificant newlines 2", "[logger]") {
    std::string source_code = "tuple = (1,\n2);";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/insignificant_newlines_test_2.nit");

    ErrorLogger& logger = ErrorLogger::inst();

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(logger.get_errors().size() == 0);
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(= tuple (tuple 1 2))");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");

    logger.reset();
}

TEST_CASE("Logger insignificant newlines 3", "[logger]") {
    std::string source_code = "arr = [1,\n (2, \n3)]\n";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/insignificant_newlines_test_3.nit");

    ErrorLogger& logger = ErrorLogger::inst();

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;
    REQUIRE(logger.get_errors().size() == 0);
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts.at(0)) == "(= arr (array 1 (tuple 2 3)))");
    CHECK(printer.print(stmts.at(1)) == "(stmt:eof)");

    logger.reset();
}

TEST_CASE("Logger significant newlines", "[logger]") {
    std::string source_code = R"(
var a = 1

var b = 2
var c = 3

a = b + c

)";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/significant_newlines_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;

    REQUIRE(logger.get_errors().size() == 0);
    REQUIRE(stmts.size() == 5);
    CHECK(printer.print(stmts.at(0)) == "(decl:var a auto 1)");
    CHECK(printer.print(stmts.at(1)) == "(decl:var b auto 2)");
    CHECK(printer.print(stmts.at(2)) == "(decl:var c auto 3)");
    CHECK(printer.print(stmts.at(3)) == "(= a (+ b c))");
    CHECK(printer.print(stmts.at(4)) == "(stmt:eof)");

    logger.reset();
}

TEST_CASE("Logger significant newlines 2", "[logger]") {
    std::string source_code = R"(
var a = 1
var b = [
    1,
    2,
    3,
]

var c = b[a]
return c
)";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/significant_newlines_test_2.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(true);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));

    Parser parser(scanner.get_tokens());
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse();

    AstPrinter printer;

    REQUIRE(logger.get_errors().size() == 0);
    REQUIRE(stmts.size() == 5);
    CHECK(printer.print(stmts.at(0)) == "(decl:var a auto 1)");
    CHECK(printer.print(stmts.at(1)) == "(decl:var b auto (array 1 2 3))");
    CHECK(printer.print(stmts.at(2)) == "(decl:var c auto ([] b a))");
    CHECK(printer.print(stmts.at(3)) == "(stmt:return c)");
    CHECK(printer.print(stmts.at(4)) == "(stmt:eof)");

    logger.reset();
}

TEST_CASE("Logger no ident after dot", "[logger]") {
    std::string source_code = "foo.true";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/no_ident_after_dot_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    parser.parse(scanner.get_tokens());

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_IDENT_AFTER_DOT);

    logger.reset();
}

TEST_CASE("Logger no ident after arrow", "[logger]") {
    std::string source_code = "foo->true";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/no_ident_after_arrow_test.nit");

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, std::make_shared<std::string>(source_code));
    Parser parser;
    parser.parse(scanner.get_tokens());

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_IDENT_AFTER_DOT);

    logger.reset();
}
