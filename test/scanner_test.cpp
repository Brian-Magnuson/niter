#include "../src/logger/logger.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "catch/catch_amalgamated.hpp"
#include <any>
#include <string>

// MARK: Main tests

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

    // std::stringstream ss;
    // logger.set_ostream(ss);
    logger.set_printing_enabled(false);
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

    logger.reset();
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

    // std::stringstream ss;
    // logger.set_ostream(ss);
    logger.set_printing_enabled(false);
    logger.log_error(token, E_CONFIG, "Test error message");
    logger.log_error(token2, E_TEST_ERROR, "Test error message 2");

    REQUIRE(logger.get_errors().size() == 2);
    CHECK(logger.get_errors().at(0) == E_CONFIG);
    CHECK(logger.get_errors().at(1) == E_TEST_ERROR);

    logger.reset();
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

TEST_CASE("Scanner bool and nil", "[scanner]") {
    std::string source_code = "true false nil";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/bool_nil_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 4);
    CHECK(tokens.at(0).tok_type == TOK_BOOL);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<bool>(tokens.at(0).literal) == true);
    CHECK(tokens.at(1).tok_type == TOK_BOOL);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::any_cast<bool>(tokens.at(1).literal) == false);
    CHECK(tokens.at(2).tok_type == TOK_NIL);
    CHECK(tokens.at(3).tok_type == TOK_EOF);
}

TEST_CASE("Scanner integers", "[scanner]") {
    std::string source_code = "5 0xab 0xAB 0o42 0b11001110 100_000_000 042";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/integers_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 8);
    CHECK(tokens.at(0).tok_type == TOK_INT);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(0).literal) == 5);
    CHECK(tokens.at(1).tok_type == TOK_INT);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(1).literal) == 0xab);
    CHECK(tokens.at(2).tok_type == TOK_INT);
    REQUIRE(tokens.at(2).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(2).literal) == 0xAB);
    CHECK(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 042);
    CHECK(tokens.at(4).tok_type == TOK_INT);
    REQUIRE(tokens.at(4).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(4).literal) == 0b11001110);
    CHECK(tokens.at(5).tok_type == TOK_INT);
    REQUIRE(tokens.at(5).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(5).literal) == 100000000);
    CHECK(tokens.at(6).tok_type == TOK_INT);
    REQUIRE(tokens.at(6).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(6).literal) == 42);
    CHECK(tokens.at(7).tok_type == TOK_EOF);
}

TEST_CASE("Scanner floating point numbers", "[scanner]") {
    std::string source_code = "5.0 5. 0.5 .5 5e5 5e+5 5e-5 5.0e5 5.0e+5 5.0e-5 5E5";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/floating_point_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 12);
    CHECK(tokens.at(0).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(0).literal) == 5.0);
    CHECK(tokens.at(1).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(1).literal) == 5.0);
    CHECK(tokens.at(2).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(2).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(2).literal) == 0.5);
    CHECK(tokens.at(3).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(3).literal) == 0.5);
    CHECK(tokens.at(4).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(4).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(4).literal) == 5e5);
    CHECK(tokens.at(5).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(5).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(5).literal) == 5e5);
    CHECK(tokens.at(6).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(6).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(6).literal) == 5e-5);
    CHECK(tokens.at(7).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(7).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(7).literal) == 5.0e5);
    CHECK(tokens.at(8).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(8).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(8).literal) == 5.0e5);
    CHECK(tokens.at(9).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(9).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(9).literal) == 5.0e-5);
    CHECK(tokens.at(10).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(10).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(10).literal) == 5e5);
    CHECK(tokens.at(11).tok_type == TOK_EOF);
}

TEST_CASE("Scanner float inf and NaN", "[scanner]") {
    std::string source_code = "inf NaN";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/inf_nan_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 3);
    CHECK(tokens.at(0).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<double>(tokens.at(0).literal) == INFINITY);
    CHECK(tokens.at(1).tok_type == TOK_FLOAT);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::isnan(std::any_cast<double>(tokens.at(1).literal)));
    CHECK(tokens.at(2).tok_type == TOK_EOF);
}

TEST_CASE("Scanner characters", "[scanner]") {
    std::string source_code = R"('a' 'b' '\\' '\n' ' ' '\'')";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/characters_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 7);
    CHECK(tokens.at(0).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(0).literal) == 'a');
    CHECK(tokens.at(1).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(1).literal) == 'b');
    CHECK(tokens.at(2).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(2).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(2).literal) == '\\');
    CHECK(tokens.at(3).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(3).literal) == '\n');
    CHECK(tokens.at(4).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(4).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(4).literal) == ' ');
    CHECK(tokens.at(5).tok_type == TOK_CHAR);
    REQUIRE(tokens.at(5).literal.has_value());
    REQUIRE(std::any_cast<char>(tokens.at(5).literal) == '\'');
    CHECK(tokens.at(6).tok_type == TOK_EOF);
}

TEST_CASE("Scanner strings", "[scanner]") {
    // std::string source_code = "\"Hello, world!\" \"\" \"\\\"\"";
    std::string source_code = R"("Hello, world!" "" "\"")";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/strings_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 4);
    CHECK(tokens.at(0).tok_type == TOK_STR);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<std::string>(tokens.at(0).literal) == "Hello, world!");
    CHECK(tokens.at(1).tok_type == TOK_STR);
    REQUIRE(tokens.at(1).literal.has_value());
    REQUIRE(std::any_cast<std::string>(tokens.at(1).literal) == "");
    CHECK(tokens.at(2).tok_type == TOK_STR);
    REQUIRE(tokens.at(2).literal.has_value());
    REQUIRE(std::any_cast<std::string>(tokens.at(2).literal) == "\"");
    CHECK(tokens.at(3).tok_type == TOK_EOF);
}

TEST_CASE("Scanner escape sequences", "[scanner]") {
    // std::string source_code = "\"\\n\\t\\r\\b\\f\\\"\\\\\"";
    std::string source_code = R"("\n\t\r\b\f\"\\")";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/escape_sequences_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 2);
    CHECK(tokens.at(0).tok_type == TOK_STR);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<std::string>(tokens.at(0).literal) == "\n\t\r\b\f\"\\");
    CHECK(tokens.at(1).tok_type == TOK_EOF);
}

TEST_CASE("Scanner multi line strings", "[scanner]") {
    std::string source_code = "\"\"\"Hello, world!\nThis is a multi-line string!\"\"\"";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/multi_line_strings_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 2);
    CHECK(tokens.at(0).tok_type == TOK_STR);
    REQUIRE(tokens.at(0).literal.has_value());
    REQUIRE(std::any_cast<std::string>(tokens.at(0).literal) == "Hello, world!\nThis is a multi-line string!");
    CHECK(tokens.at(1).tok_type == TOK_EOF);
}

TEST_CASE("Scanner comments", "[scanner]") {
    std::string source_code = "var x = 5 // This is a comment\nvar y = 10";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/comments_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 10);
    CHECK(tokens.at(0).tok_type == KW_VAR);
    CHECK(tokens.at(1).tok_type == TOK_IDENT);
    CHECK(tokens.at(1).lexeme == "x");
    CHECK(tokens.at(2).tok_type == TOK_EQ);
    CHECK(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 5);
    CHECK(tokens.at(4).tok_type == TOK_NEWLINE);
    CHECK(tokens.at(5).tok_type == KW_VAR);
    CHECK(tokens.at(6).tok_type == TOK_IDENT);
    CHECK(tokens.at(6).lexeme == "y");
    CHECK(tokens.at(7).tok_type == TOK_EQ);
    CHECK(tokens.at(8).tok_type == TOK_INT);
    REQUIRE(tokens.at(8).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(8).literal) == 10);
    CHECK(tokens.at(9).tok_type == TOK_EOF);
}

TEST_CASE("Scanner multi line comments", "[scanner]") {
    std::string source_code = "var x = 5 /* This is a multi-line comment\nIt spans multiple lines */ var y = 10";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/multi_line_comments_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 9);
    CHECK(tokens.at(0).tok_type == KW_VAR);
    CHECK(tokens.at(1).tok_type == TOK_IDENT);
    CHECK(tokens.at(1).lexeme == "x");
    CHECK(tokens.at(2).tok_type == TOK_EQ);
    CHECK(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 5);
    CHECK(tokens.at(4).tok_type == KW_VAR);
    CHECK(tokens.at(5).tok_type == TOK_IDENT);
    CHECK(tokens.at(5).lexeme == "y");
    CHECK(tokens.at(6).tok_type == TOK_EQ);
    CHECK(tokens.at(7).tok_type == TOK_INT);
    REQUIRE(tokens.at(7).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(7).literal) == 10);
    CHECK(tokens.at(8).tok_type == TOK_EOF);
}

TEST_CASE("Scanner identifiers", "[scanner]") {
    std::string source_code = "var var1 var_1 _var _var1 _var_1 _ _1 1 v";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/identifiers_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 11);
    CHECK(tokens.at(0).tok_type == KW_VAR);
    CHECK(tokens.at(1).tok_type == TOK_IDENT);
    CHECK(tokens.at(1).lexeme == "var1");
    CHECK(tokens.at(2).tok_type == TOK_IDENT);
    CHECK(tokens.at(2).lexeme == "var_1");
    CHECK(tokens.at(3).tok_type == TOK_IDENT);
    CHECK(tokens.at(3).lexeme == "_var");
    CHECK(tokens.at(4).tok_type == TOK_IDENT);
    CHECK(tokens.at(4).lexeme == "_var1");
    CHECK(tokens.at(5).tok_type == TOK_IDENT);
    CHECK(tokens.at(5).lexeme == "_var_1");
    CHECK(tokens.at(6).tok_type == TOK_IDENT);
    CHECK(tokens.at(6).lexeme == "_");
    CHECK(tokens.at(7).tok_type == TOK_IDENT);
    CHECK(tokens.at(7).lexeme == "_1");
    CHECK(tokens.at(8).tok_type == TOK_INT);
    REQUIRE(tokens.at(8).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(8).literal) == 1);
    CHECK(tokens.at(9).tok_type == TOK_IDENT);
    CHECK(tokens.at(9).lexeme == "v");
    CHECK(tokens.at(10).tok_type == TOK_EOF);
}

TEST_CASE("Escaping newlines", "[scanner]") {
    std::string source_code = "var a\\\n= 1";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/escaping_newlines_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    Scanner scanner;
    scanner.scan_file(file_name, source);
    auto tokens = scanner.get_tokens();

    REQUIRE(tokens.size() == 5);
    CHECK(tokens.at(0).tok_type == KW_VAR);
    CHECK(tokens.at(1).tok_type == TOK_IDENT);
    CHECK(tokens.at(1).lexeme == "a");
    CHECK(tokens.at(2).tok_type == TOK_EQ);
    CHECK(tokens.at(3).tok_type == TOK_INT);
    REQUIRE(tokens.at(3).literal.has_value());
    REQUIRE(std::any_cast<long long>(tokens.at(3).literal) == 1);
    CHECK(tokens.at(4).tok_type == TOK_EOF);
}

// MARK: Error tests

TEST_CASE("Logger no LF after backslash", "[logger]") {
    // std::string source_code = "var \\ var";
    std::string source_code = R"(var \ var)";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/no_lf_after_backslash_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_LF_AFTER_BACKSLASH);

    logger.reset();
}

TEST_CASE("Logger comment errors", "[logger]") {
    std::string source_code = "var /* unclosed comment";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unclosed_comment_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNCLOSED_COMMENT);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var */ closing unopened comment";
    file_name = std::make_shared<std::string>("test_files/closing_unopened_comment_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_CLOSING_UNOPENED_COMMENT);

    logger.reset();
}

TEST_CASE("Logger character errors", "[logger]") {
    std::string source_code = "var 'a";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unclosed_char_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNCLOSED_CHAR);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var '' empty char";
    file_name = std::make_shared<std::string>("test_files/empty_char_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_EMPTY_CHAR);

    logger.reset();
    logger.set_printing_enabled(false);

    // source_code = "var '\\z' illegal escape sequence";
    source_code = R"(var '\z' illegal escape sequence)";
    file_name = std::make_shared<std::string>("test_files/illegal_esc_seq_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_ILLEGAL_ESC_SEQ);

    logger.reset();
}

TEST_CASE("Logger string errors", "[logger]") {
    // std::string source_code = "var \"unclosed string";
    std::string source_code = R"(var "unclosed string)";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/unclosed_string_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNCLOSED_STRING);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var \"\"\" unclosed\nmulti-line\nstring\n";
    file_name = std::make_shared<std::string>("test_files/unclosed_multi_line_string_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_UNCLOSED_MULTI_LINE_STRING);

    logger.reset();
}

TEST_CASE("Logger number errors", "[logger]") {
    std::string source_code = "var 5.5.5";
    std::shared_ptr file_name = std::make_shared<std::string>("test_files/multiple_decimal_points_test.nit");
    std::shared_ptr<std::string> source = std::make_shared<std::string>(source_code);

    ErrorLogger& logger = ErrorLogger::inst();
    logger.set_printing_enabled(false);

    Scanner scanner;
    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_MULTIPLE_DECIMAL_POINTS);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var 0x5.5";
    file_name = std::make_shared<std::string>("test_files/non_decimal_float_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NON_DECIMAL_FLOAT);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var 5e";
    file_name = std::make_shared<std::string>("test_files/no_digits_in_exponent_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_NO_DIGITS_IN_EXPONENT);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var 12345678901234567890";
    file_name = std::make_shared<std::string>("test_files/int_too_large_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_INT_TOO_LARGE);

    logger.reset();
    logger.set_printing_enabled(false);

    source_code = "var 1E1000";
    file_name = std::make_shared<std::string>("test_files/float_too_large_test.nit");
    source = std::make_shared<std::string>(source_code);

    scanner.scan_file(file_name, source);

    REQUIRE(logger.get_errors().size() >= 1);
    CHECK(logger.get_errors().at(0) == E_FLOAT_TOO_LARGE);

    logger.reset();
}
