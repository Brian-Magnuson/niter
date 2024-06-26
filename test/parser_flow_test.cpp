#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include "../src/logger/logger.h"
#include "../src/parser/ast_printer.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "../src/scanner/token.h"
#include "../src/utility/decl.h"
#include "../src/utility/expr.h"
#include "../src/utility/stmt.h"

TEST_CASE("Parser if stmt", "[parser]") {
    auto source_code = std::make_shared<std::string>("if true { x = 1; }");
    auto file_name = std::make_shared<std::string>("test_files/if_stmt.nit");

    Scanner scanner;
    scanner.scan_file(file_name, source_code);
    Parser parser;
    std::vector<std::shared_ptr<Stmt>> stmts = parser.parse(scanner.get_tokens());

    AstPrinter printer;
    REQUIRE(stmts.size() == 2);
    CHECK(printer.print(stmts[0]) == "(stmt:if true { (= x 1)})");
    CHECK(printer.print(stmts[1]) == "(stmt:eof)");
}
