#include "../src/logger/logger.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "catch/catch_amalgamated.hpp"
#include <string>

TEST_CASE("Sanity check", "[sanity]") {
    REQUIRE(1 == 1);
}

TEST_CASE("Logger", "[logger]") {
    ErrorLogger& logger = ErrorLogger::inst();
    std::string file_name = "test_files/error_test.nit";
    std::string source_code = "var x = 5";
    Location location = {
        std::make_shared<std::string>(file_name),
        1, // line
        0, // column
        3, // length
        0, // line_index
        std::make_shared<std::string>(source_code)
    };
    Token token(TOK_IDENT, "x", 5, location);

    std::stringstream ss;
    logger.set_ostream(ss);
    logger.log_error(token, E_CONFIG, "Test error message");

    REQUIRE(logger.get_errors().size() == 1);
    REQUIRE(logger.get_errors().at(0) == E_CONFIG);

    // std::string expected = file_name + ":1:0\n\033[31mError 1: \033[0m1000 Test error message\n    1 | var x = 5\n        \033[31m^~~\033[0m\n\n";
    // /*
    // This is what that string looks like:
    // test_files/error_test.nit:1:0
    // Error 1: Test error message
    //     1 | var x = 5
    //         ^~~

    // */

    // REQUIRE(ss.str() == expected);
    // logger.set_ostream(std::cerr);
    // logger.log_error(token, E_CONFIG, "Test error message");

    ErrorLogger::inst().clear_errors();
}

TEST_CASE("Log in order", "[logger]") {
    ErrorLogger& logger = ErrorLogger::inst();
    std::string file_name = "test_files/error_test.nit";
    std::string source_code = "var x = 5";
    Location location = {
        std::make_shared<std::string>(file_name),
        1, // line
        0, // column
        3, // length
        0, // line_index
        std::make_shared<std::string>(source_code)
    };
    Token token(TOK_IDENT, "x", 5, location);
    Location location2 = {
        std::make_shared<std::string>(file_name),
        1, // line
        0, // column
        3, // length
        0, // line_index
        std::make_shared<std::string>(source_code)
    };
    Token token2(TOK_IDENT, "x", 5, location2);

    std::stringstream ss;
    logger.set_ostream(ss);
    logger.log_error(token, E_CONFIG, "Test error message");
    logger.log_error(token2, E_TEST_ERROR, "Test error message 2");

    REQUIRE(logger.get_errors().size() == 2);
    CHECK(logger.get_errors().at(0) == E_CONFIG);
    CHECK(logger.get_errors().at(1) == E_TEST_ERROR);

    ErrorLogger::inst().clear_errors();
}

TEST_CASE("Scanner", "[scanner]") {
    std::string source_code = "var x = 5";
    std::string file_name = "test_files/scanner_test.nit";
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);
    std::shared_ptr<std::string> filename = std::make_shared<std::string>(file_name);
    Scanner scanner;
    scanner.scan_file(filename, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 5);
    CHECK(tokens.at(0).tok_type == KW_VAR);
    CHECK(tokens.at(1).tok_type == TOK_IDENT);
    CHECK(tokens.at(2).tok_type == TOK_EQ);
    CHECK(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 5);
    CHECK(tokens.at(4).tok_type == TOK_EOF);
}

TEST_CASE("Scanner keywords", "[scanner]") {

    std::string source_code = "and or not if else loop while for in break continue return yield var const fun oper struct enum type interface using namespace static global self as typeof is alloc dealloc extern";

    std::shared_ptr file_name = std::make_shared<std::string>("test_files/keywords_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 33);
    CHECK(tokens.at(0).tok_type == KW_AND);
    CHECK(tokens.at(1).tok_type == KW_OR);
    CHECK(tokens.at(2).tok_type == KW_NOT);
    CHECK(tokens.at(3).tok_type == KW_IF);
    CHECK(tokens.at(4).tok_type == KW_ELSE);
    CHECK(tokens.at(5).tok_type == KW_LOOP);
    CHECK(tokens.at(6).tok_type == KW_WHILE);
    CHECK(tokens.at(7).tok_type == KW_FOR);
    CHECK(tokens.at(8).tok_type == KW_IN);
    CHECK(tokens.at(9).tok_type == KW_BREAK);
    CHECK(tokens.at(10).tok_type == KW_CONTINUE);
    CHECK(tokens.at(11).tok_type == KW_RETURN);
    CHECK(tokens.at(12).tok_type == KW_YIELD);
    CHECK(tokens.at(13).tok_type == KW_VAR);
    CHECK(tokens.at(14).tok_type == KW_CONST);
    CHECK(tokens.at(15).tok_type == KW_FUN);
    CHECK(tokens.at(16).tok_type == KW_OPER);
    CHECK(tokens.at(17).tok_type == KW_STRUCT);
    CHECK(tokens.at(18).tok_type == KW_ENUM);
    CHECK(tokens.at(19).tok_type == KW_TYPE);
    CHECK(tokens.at(20).tok_type == KW_INTERFACE);
    CHECK(tokens.at(21).tok_type == KW_USING);
    CHECK(tokens.at(22).tok_type == KW_NAMESPACE);
    CHECK(tokens.at(23).tok_type == KW_STATIC);
    CHECK(tokens.at(24).tok_type == KW_GLOBAL);
    CHECK(tokens.at(25).tok_type == KW_SELF);
    CHECK(tokens.at(26).tok_type == KW_AS);
    CHECK(tokens.at(27).tok_type == KW_TYPEOF);
    CHECK(tokens.at(28).tok_type == KW_IS);
    CHECK(tokens.at(29).tok_type == KW_ALLOC);
    CHECK(tokens.at(30).tok_type == KW_DEALLOC);
    CHECK(tokens.at(31).tok_type == KW_EXTERN);
    CHECK(tokens.at(32).tok_type == TOK_EOF);
}

TEST_CASE("Scanner operators 1", "[scanner]") {
    std::string source_code = "(){}[]+ += - -= * *= / /= % %= ^ ^=,;";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/operators_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 21);
    CHECK(tokens.at(0).tok_type == TOK_LEFT_PAREN);
    CHECK(tokens.at(1).tok_type == TOK_RIGHT_PAREN);
    CHECK(tokens.at(2).tok_type == TOK_LEFT_BRACE);
    CHECK(tokens.at(3).tok_type == TOK_RIGHT_BRACE);
    CHECK(tokens.at(4).tok_type == TOK_LEFT_SQUARE);
    CHECK(tokens.at(5).tok_type == TOK_RIGHT_SQUARE);
    CHECK(tokens.at(6).tok_type == TOK_PLUS);
    CHECK(tokens.at(7).tok_type == TOK_PLUS_EQ);
    CHECK(tokens.at(8).tok_type == TOK_MINUS);
    CHECK(tokens.at(9).tok_type == TOK_MINUS_EQ);
    CHECK(tokens.at(10).tok_type == TOK_STAR);
    CHECK(tokens.at(11).tok_type == TOK_STAR_EQ);
    CHECK(tokens.at(12).tok_type == TOK_SLASH);
    CHECK(tokens.at(13).tok_type == TOK_SLASH_EQ);
    CHECK(tokens.at(14).tok_type == TOK_PERCENT);
    CHECK(tokens.at(15).tok_type == TOK_PERCENT_EQ);
    CHECK(tokens.at(16).tok_type == TOK_CARET);
    CHECK(tokens.at(17).tok_type == TOK_CARET_EQ);
    CHECK(tokens.at(18).tok_type == TOK_COMMA);
    CHECK(tokens.at(19).tok_type == TOK_SEMICOLON);
    CHECK(tokens.at(20).tok_type == TOK_EOF);
}

TEST_CASE("Scanner operators 2", "[scanner]") {
    std::string source_code = "& && &= &&= | || |= ||= ! != = == > >= < <= . .. ... : :: -> =>";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/operators_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 24);
    CHECK(tokens.at(0).tok_type == TOK_AMP);
    CHECK(tokens.at(1).tok_type == TOK_AMP_AMP);
    CHECK(tokens.at(2).tok_type == TOK_AMP_EQ);
    CHECK(tokens.at(3).tok_type == TOK_AMP_AMP_EQ);
    CHECK(tokens.at(4).tok_type == TOK_BAR);
    CHECK(tokens.at(5).tok_type == TOK_BAR_BAR);
    CHECK(tokens.at(6).tok_type == TOK_BAR_EQ);
    CHECK(tokens.at(7).tok_type == TOK_BAR_BAR_EQ);
    CHECK(tokens.at(8).tok_type == TOK_BANG);
    CHECK(tokens.at(9).tok_type == TOK_BANG_EQ);
    CHECK(tokens.at(10).tok_type == TOK_EQ);
    CHECK(tokens.at(11).tok_type == TOK_EQ_EQ);
    CHECK(tokens.at(12).tok_type == TOK_GT);
    CHECK(tokens.at(13).tok_type == TOK_GE);
    CHECK(tokens.at(14).tok_type == TOK_LT);
    CHECK(tokens.at(15).tok_type == TOK_LE);
    CHECK(tokens.at(16).tok_type == TOK_DOT);
    CHECK(tokens.at(17).tok_type == TOK_DOT_DOT);
    CHECK(tokens.at(18).tok_type == TOK_TRIPLE_DOT);
    CHECK(tokens.at(19).tok_type == TOK_COLON);
    CHECK(tokens.at(20).tok_type == TOK_COLON_COLON);
    CHECK(tokens.at(21).tok_type == TOK_ARROW);
    CHECK(tokens.at(22).tok_type == TOK_DOUBLE_ARROW);
    CHECK(tokens.at(23).tok_type == TOK_EOF);
}
