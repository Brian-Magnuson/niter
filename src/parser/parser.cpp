#include "parser.h"
#include "../logger/logger.h"

Token& Parser::peek() {
    return tokens[current];
}

Token& Parser::previous() {
    return tokens[current - 1];
}

bool Parser::check(TokenType tok_type) {
    if (is_at_end()) {
        return false;
    }
    return peek().tok_type == tok_type;
}

bool Parser::is_at_end() {
    return peek().tok_type == TOK_EOF;
}

Token& Parser::advance() {
    if (!is_at_end()) {
        current++;
    }
    return previous();
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token& Parser::consume(TokenType tok_type, ErrorCode error_code, const std::string& message) {
    if (check(tok_type)) {
        return advance();
    }
    ErrorLogger::inst().log_error(peek(), error_code, message);
    synchronize();
}

void Parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (previous().tok_type == TOK_NEWLINE) {
            return;
        }

        switch (peek().tok_type) {
        case KW_STRUCT:
        case KW_FUN:
        case KW_VAR:
        case KW_FOR:
        case KW_IF:
        case KW_WHILE:
        case KW_LOOP:
        case KW_RETURN:
            return;
        default:
            break;
        }

        advance();
    }
}

std::shared_ptr<Stmt> Parser::statement() {
    // if (match({KW_VAR})) {
    //     return var_declaration();
    // }
    // if (match({KW_FUN})) {
    //     return fun_declaration();
    // }
    // if (match({KW_IF})) {
    //     return if_statement();
    // }
    // if (match({KW_WHILE})) {
    //     return while_statement();
    // }
    // if (match({KW_LOOP})) {
    //     return loop_statement();
    // }
    // if (match({KW_RETURN})) {
    //     return return_statement();
    // }
    if (match({KW_PUTS})) {
        return print_statement();
    }

    return expression_statement();
}

std::shared_ptr<Stmt> Parser::print_statement() {
    // TODO: Implement print statement
    return nullptr;
}

std::shared_ptr<Stmt> Parser::expression_statement() {
    // TODO: Implement expression statement
    return nullptr;
}

std::shared_ptr<Expr> Parser::expression() {
    return assign_expr();
}

std::shared_ptr<Expr> Parser::assign_expr() {
    std::shared_ptr<Expr> expr = or_expr();

    if (match({TOK_EQ})) {
        Token equals = previous();
        std::shared_ptr<Expr> value = expression();
        // TODO: Implement assignment expression
        ErrorLogger::inst().log_error(equals, E_UNIMPLEMENTED, "Assignment expressions are not yet implemented.");
    }
}

std::shared_ptr<Expr> Parser::or_expr() {
    std::shared_ptr<Expr> expr = and_expr();

    while (match({KW_OR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = and_expr();
        expr = std::make_shared<Expr::Logical>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::and_expr() {
    std::shared_ptr<Expr> expr = equality_expr();

    while (match({KW_AND})) {
        Token op = previous();
        std::shared_ptr<Expr> right = equality_expr();
        expr = std::make_shared<Expr::Logical>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::equality_expr() {
    std::shared_ptr<Expr> expr = comparison_expr();

    while (match({TOK_EQ_EQ, TOK_BANG_EQ})) {
        Token op = previous();
        std::shared_ptr<Expr> right = comparison_expr();
        expr = std::make_shared<Expr::Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::comparison_expr() {
    std::shared_ptr<Expr> expr = term_expr();

    while (match({TOK_LT, TOK_LE, TOK_GT, TOK_GE})) {
        Token op = previous();
        std::shared_ptr<Expr> right = term_expr();
        expr = std::make_shared<Expr::Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::term_expr() {
    std::shared_ptr<Expr> expr = factor_expr();

    while (match({TOK_PLUS, TOK_MINUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = factor_expr();
        expr = std::make_shared<Expr::Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::factor_expr() {
    std::shared_ptr<Expr> expr = unary_expr();

    while (match({TOK_STAR, TOK_SLASH, TOK_PERCENT})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary_expr();
        expr = std::make_shared<Expr::Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::power_expr() {
    std::shared_ptr<Expr> expr = unary_expr();

    while (match({TOK_CARET})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary_expr();
        expr = std::make_shared<Expr::Binary>(expr, op, right);
    }
    return expr;
}

// TODO: Implement unary_expr

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;

    while (!is_at_end()) {
        statements.push_back(statement());
    }

    return statements;
}
