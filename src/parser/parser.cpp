#include "parser.h"
#include "../logger/logger.h"
#include <exception>

Token& Parser::peek() {
    return tokens[current];
}

Token& Parser::previous() {
    return tokens[current - 1];
}

bool Parser::check(const std::vector<TokenType>& types) {
    if (is_at_end()) {
        return false;
    }
    for (TokenType type : types) {
        if (peek().tok_type == type) {
            return true;
        }
    }
    return false;
}

bool Parser::is_at_end() {
    return peek().tok_type == TOK_EOF;
}

Token& Parser::advance() {
    if (!is_at_end()) {
        current++;
    }
    Token& prev = previous();
    if (!grouping_tokens.empty() && prev.tok_type == grouping_tokens.top()) {
        grouping_tokens.pop();
    }
    if (grouping_tokens.size() > 0 && peek().tok_type == TOK_NEWLINE) {
        current++;
    }
    return prev;
}

bool Parser::match(const std::vector<TokenType>& types) {
    if (check(types)) {
        advance();
        return true;
    }
    return false;
}

Token& Parser::consume(TokenType tok_type, ErrorCode error_code, const std::string& message) {
    if (check({tok_type})) {
        return advance();
    }
    ErrorLogger::inst().log_error(peek(), error_code, message);
    throw ParserException();
}

void Parser::synchronize() {
    grouping_tokens = std::stack<TokenType>();
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

// MARK: Statements

std::shared_ptr<Stmt> Parser::statement() {
    try {
        if (check({KW_VAR, KW_CONST, KW_FUN})) {
            return declaration_statement();
        }
        // if (match({KW_IF})) {
        //     return if_statement();
        // }
        // if (match({KW_WHILE})) {
        //     return while_statement();
        // }
        // if (match({KW_LOOP})) {
        //     return loop_statement();
        // }
        if (match({KW_RETURN})) {
            return return_statement();
        }
        if (match({KW_PUTS})) {
            return print_statement();
        }
        return expression_statement();

    } catch (const ParserException& e) {
        synchronize();
        return nullptr;
    }
}

std::shared_ptr<Stmt> Parser::declaration_statement() {
    std::shared_ptr<Decl> decl;
    if (match({KW_VAR, KW_CONST})) {
        decl = var_decl();
        if (!match({TOK_NEWLINE, TOK_SEMICOLON})) {
            ErrorLogger::inst().log_error(peek(), E_MISSING_STMT_END, "Expected newline or ';' after declaration.");
            throw ParserException();
        }
    } else if (match({KW_FUN})) {
        decl = fun_decl();
    } else {
        // This function is called in statement where it is verified that the current token signifies a declaration.
        // Therefore, this should be unreachable.
        ErrorLogger::inst().log_error(peek(), E_UNREACHABLE, "Unreachable code reached in declaration statement.");
        throw ParserException();
    }
    return std::make_shared<Stmt::Declaration>(decl);
}

std::shared_ptr<Stmt> Parser::expression_statement() {
    std::shared_ptr<Expr> expr = expression();
    if (!check({TOK_EOF}) && !match({TOK_NEWLINE, TOK_SEMICOLON})) {
        ErrorLogger::inst().log_error(peek(), E_MISSING_STMT_END, "Expected newline or ';' after expression.");
    }
    return std::make_shared<Stmt::Expression>(expr);
}

std::shared_ptr<Stmt> Parser::print_statement() {
    std::shared_ptr<Expr> value = expression();
    if (!match({TOK_NEWLINE, TOK_SEMICOLON})) {
        ErrorLogger::inst().log_error(peek(), E_MISSING_STMT_END, "Expected newline or ';' after expression.");
    }
    return std::make_shared<Stmt::Print>(value);
}

std::shared_ptr<Stmt> Parser::return_statement() {
    Token keyword = previous();
    std::shared_ptr<Expr> value = nullptr;
    if (!check({TOK_NEWLINE, TOK_SEMICOLON})) {
        value = expression();
    }
    if (!match({TOK_NEWLINE, TOK_SEMICOLON})) {
        ErrorLogger::inst().log_error(peek(), E_MISSING_STMT_END, "Expected newline or ';' after return statement.");
    }
    return std::make_shared<Stmt::Return>(keyword, value);
}

// MARK: Declarations

std::shared_ptr<Decl> Parser::var_decl() {
    TokenType declarer = previous().tok_type;
    if (declarer != KW_VAR && declarer != KW_CONST) {
        // This can only happen if var_decl was called outside of declaration_statement, i.e. in another decl function.
        // If neither var nor const was specified, then assume const.
        declarer = KW_CONST;
    }
    Token name = consume(TOK_IDENT, E_UNNAMED_VAR, "Expected identifier in declaration.");

    std::shared_ptr<Expr::Identifier> type_annotation = nullptr;
    if (match({TOK_COLON})) {
        type_annotation = std::dynamic_pointer_cast<Expr::Identifier>(primary_expr());
        if (type_annotation == nullptr) {
            ErrorLogger::inst().log_error(peek(), E_INVALID_TYPE_ANNOTATION, "Invalid type annotation.");
            throw ParserException();
        }
    } else {
        // Inject an 'auto' token for type inference later
        Location location = name.location;
        // It will be treated as having the same location as the name
        type_annotation = std::make_shared<Expr::Identifier>(Token{
            TOK_IDENT,
            "auto",
            std::any(),
            location
        });
    }
    std::shared_ptr<Expr> initializer = nullptr;
    if (match({TOK_EQ})) {
        initializer = expression();
    }
    return std::make_shared<Decl::Var>(declarer, name, type_annotation, initializer);
}

std::shared_ptr<Decl> Parser::fun_decl() {
    // One thing at a time...
    // First, grab the declarer
    TokenType declarer = previous().tok_type;
    // Next, grab the identifier
    Token name = consume(TOK_IDENT, E_UNNAMED_FUN, "Expected identifier in function declaration.");
    // Next, the parameters
    std::vector<std::shared_ptr<Decl::Var>> parameters;
    consume(TOK_LEFT_PAREN, E_NO_LPAREN_IN_FUN_DECL, "Expected '(' after function name.");
    grouping_tokens.push(TOK_RIGHT_PAREN);
    if (!check({TOK_RIGHT_PAREN})) {
        auto variable = var_decl();
        if (variable == nullptr) {
            ErrorLogger::inst().log_error(peek(), E_IMPOSSIBLE, "var_decl did not return a variable in function declaration.");
            throw ParserException();
        }
        parameters.push_back(std::dynamic_pointer_cast<Decl::Var>(variable));
        while (match({TOK_COMMA})) {
            if (check({TOK_RIGHT_PAREN})) {
                break;
            }
            variable = var_decl();
            if (variable == nullptr) {
                ErrorLogger::inst().log_error(peek(), E_IMPOSSIBLE, "var_decl did not return a variable in function declaration.");
                throw ParserException();
            }
            parameters.push_back(std::dynamic_pointer_cast<Decl::Var>(variable));
        }
    }
    consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_PARAMS, "Expected ')' after function parameters.");

    std::shared_ptr<Expr::Identifier> return_type = nullptr;
    if (match({TOK_COLON})) {
        return_type = std::dynamic_pointer_cast<Expr::Identifier>(primary_expr());
        if (return_type == nullptr) {
            ErrorLogger::inst().log_error(peek(), E_INVALID_TYPE_ANNOTATION, "Invalid type annotation.");
            throw ParserException();
        }
    } else {
        // Inject a 'void' token for type inference later
        Location location = previous().location;
        // It will be treated as having the same location as the previous token
        return_type = std::make_shared<Expr::Identifier>(Token{
            TOK_IDENT,
            "void",
            std::any(),
            location
        });
    }

    std::vector<std::shared_ptr<Stmt>> body;
    consume(TOK_LEFT_BRACE, E_NO_LBRACE_IN_FUN_DECL, "Expected '{' before function body.");
    while (!check({TOK_RIGHT_BRACE})) {
        body.push_back(statement());
    }
    consume(TOK_RIGHT_BRACE, E_UNMATCHED_BRACE_IN_FUN_DECL, "Expected '}' after function body.");
    return std::make_shared<Decl::Fun>(declarer, name, parameters, return_type, body);
}

// MARK: Expressions

std::shared_ptr<Expr> Parser::expression() {
    return assign_expr();
}

std::shared_ptr<Expr> Parser::assign_expr() {
    std::shared_ptr<Expr> expr = or_expr();

    // Note: This isn't an interpreter. We can save l-value checking for
    // the type checker.
    // 5 = 10 is syntactically valid, but semantically invalid.
    if (match({TOK_EQ})) {
        Token op = previous();
        std::shared_ptr<Expr> right = or_expr();
        expr = std::make_shared<Expr::Assign>(expr, op, right);
    }

    return expr;
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
    std::shared_ptr<Expr> expr = power_expr();

    while (match({TOK_STAR, TOK_SLASH, TOK_PERCENT})) {
        Token op = previous();
        std::shared_ptr<Expr> right = power_expr();
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

std::shared_ptr<Expr> Parser::unary_expr() {
    if (match({TOK_BANG, TOK_MINUS, TOK_STAR, TOK_AMP})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary_expr();
        return std::make_shared<Expr::Unary>(op, right);
    }
    return access_expr();
}

std::shared_ptr<Expr> Parser::access_expr() {
    std::shared_ptr<Expr> expr = call_expr();
    while (match({TOK_DOT, TOK_ARROW, TOK_LEFT_SQUARE})) {
        Token op = previous();
        std::shared_ptr<Expr> right;
        if (op.tok_type == TOK_LEFT_SQUARE) {
            grouping_tokens.push(TOK_RIGHT_SQUARE);
            // If we have [] access, any expression is allowed
            right = expression();
            consume(TOK_RIGHT_SQUARE, E_UNMATCHED_LEFT_SQUARE, "Expected ']' after expression.");
        } else {
            right = call_expr();
        }
        expr = std::make_shared<Expr::Access>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::call_expr() {
    std::shared_ptr<Expr> expr = primary_expr();
    /*
    Valid forms:
    FUNC()
    FUNC(ARG1)
    FUNC(ARG1, ARG2)
    FUNC(ARG1,)
    FUNC(ARG1, ARG2,)
    */
    while (match({TOK_LEFT_PAREN})) {
        grouping_tokens.push(TOK_RIGHT_PAREN);
        Token& paren = previous();
        std::vector<std::shared_ptr<Expr>> arguments;
        // If there are no arguments, we can just return the call expression
        if (!check({TOK_RIGHT_PAREN})) {
            arguments.push_back(expression());
            while (match({TOK_COMMA})) {
                if (check({TOK_RIGHT_PAREN})) {
                    break;
                }
                arguments.push_back(expression());
                if (arguments.size() > 255) {
                    ErrorLogger::inst().log_error(peek(), E_TOO_MANY_ARGS, "Cannot have more than 255 arguments.");
                    throw ParserException();
                }
            }
        }
        consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_ARGS, "Expected ')' after arguments.");

        expr = std::make_shared<Expr::Call>(expr, paren, arguments);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::primary_expr() {
    // These are in separate cases in case we want to add extra information to the expression
    if (match({TOK_NIL})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_BOOL})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_INT})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_FLOAT})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_CHAR})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_STR})) {
        return std::make_shared<Expr::Literal>(previous());
    }
    if (match({TOK_IDENT})) {
        std::shared_ptr<Expr::Identifier> expr = std::make_shared<Expr::Identifier>(previous());

        while (match({TOK_COLON_COLON})) {
            Token name = consume(TOK_IDENT, E_NOT_AN_IDENTIFIER, "Expected identifier after '::'.");
            expr->tokens.push_back(name);
        }
        return expr;
    }
    if (match({TOK_LEFT_SQUARE})) {
        grouping_tokens.push(TOK_RIGHT_SQUARE);
        std::vector<std::shared_ptr<Expr>> elements;
        if (!check({TOK_RIGHT_SQUARE})) {
            elements.push_back(expression());
            while (match({TOK_COMMA})) {
                if (check({TOK_RIGHT_SQUARE})) {
                    break;
                }
                elements.push_back(expression());
            }
        }

        consume(TOK_RIGHT_SQUARE, E_UNMATCHED_LEFT_SQUARE, "Expected ']' after array.");
        return std::make_shared<Expr::Array>(elements);
    }

    if (match({TOK_LEFT_PAREN})) {
        grouping_tokens.push(TOK_RIGHT_PAREN);
        std::vector<std::shared_ptr<Expr>> expressions;
        if (match({TOK_RIGHT_PAREN})) {
            // Empty tuple
            return std::make_shared<Expr::Tuple>(expressions); // expressions is empty
        }
        // There must be at least one expression if it's a tuple, so "(,)" is invalid
        expressions.push_back(expression());
        if (check({TOK_COMMA})) {
            while (match({TOK_COMMA})) {
                // Trailing comma is allowed, so ",)" marks the end of the tuple
                if (check({TOK_RIGHT_PAREN})) {
                    break;
                }
                expressions.push_back(expression());
            }
            consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_TUPLE, "Expected ')' after tuple.");
            return std::make_shared<Expr::Tuple>(expressions);
        }
        consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_GROUPING, "Expected ')' after expression.");
        return expressions[0];
    }
    ErrorLogger::inst().log_error(peek(), E_NOT_AN_EXPRESSION, "Expected expression.");
    throw ParserException();
}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;

    while (current != tokens.size()) {
        while (!is_at_end()) {
            statements.push_back(statement());
        }
        current++;
        statements.push_back(std::make_shared<Stmt::EndOfFile>());
    }

    return statements;
}
