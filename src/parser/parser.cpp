#include "parser.h"

#include "../logger/logger.h"
#include "../scanner/scanner.h"
#include "../utility/dictionary.h"
#include <exception>
#include <unordered_map>

Token& Parser::peek() {
    return *tokens[current];
}

Token& Parser::previous() {
    return *tokens[current - 1];
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
    ErrorLogger::inst().log_error(peek().location, error_code, message);
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
        if (check({KW_VAR, KW_CONST, KW_FUN, KW_EXTERN, KW_STRUCT})) {
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
            ErrorLogger::inst().log_error(peek().location, E_MISSING_STMT_END, "Expected newline or ';' after declaration.");
            throw ParserException();
        }
    } else if (match({KW_FUN})) {
        decl = fun_decl();
    } else if (match({KW_EXTERN})) {
        if (match({KW_FUN})) {
            decl = extern_fun_decl();
        } else if (match({KW_VARIADIC}) && match({KW_FUN})) {
            decl = extern_fun_decl(true);
        } else {
            ErrorLogger::inst().log_error(peek().location, E_NO_DECLARER_AFTER_EXTERN, "'extern' requires valid declarer. Expected 'fun'.");
            throw ParserException();
        }
        if (!match({TOK_NEWLINE, TOK_SEMICOLON})) {
            ErrorLogger::inst().log_error(peek().location, E_MISSING_STMT_END, "Expected newline or ';' after declaration.");
            throw ParserException();
        }
    } else if (match({KW_STRUCT})) {
        decl = struct_decl();
    } else {
        ErrorLogger::inst().log_error(peek().location, E_NOT_A_DECLARATION, "Expected a declaration.");
        throw ParserException();
    }
    return std::make_shared<Stmt::Declaration>(decl);
}

std::shared_ptr<Stmt> Parser::expression_statement() {
    std::shared_ptr<Expr> expr = expression();
    if (!check({TOK_EOF}) && !match({TOK_NEWLINE, TOK_SEMICOLON})) {
        ErrorLogger::inst().log_error(peek().location, E_MISSING_STMT_END, "Expected newline or ';' after expression.");
    }
    return std::make_shared<Stmt::Expression>(expr);
}

std::shared_ptr<Stmt> Parser::return_statement() {
    Token keyword = previous();
    std::shared_ptr<Expr> value = nullptr;
    if (!check({TOK_NEWLINE, TOK_SEMICOLON})) {
        value = expression();
    }
    if (!match({TOK_NEWLINE, TOK_SEMICOLON})) {
        ErrorLogger::inst().log_error(peek().location, E_MISSING_STMT_END, "Expected newline or ';' after return statement.");
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

    std::shared_ptr<Annotation> type_annotation = nullptr;
    if (match({TOK_COLON})) {
        type_annotation = annotation();
    } else {
        type_annotation = std::make_shared<Annotation::Segmented>("auto");
    }

    std::shared_ptr<Expr> initializer = nullptr;
    if (match({TOK_EQ})) {
        initializer = expression();
    }
    return std::make_shared<Decl::Var>(declarer, name, type_annotation, initializer);
}

std::shared_ptr<Decl> Parser::fun_decl() {
    // First, grab the declarer
    TokenType declarer = previous().tok_type;
    // Next, grab the identifier
    Token name = consume(TOK_IDENT, E_UNNAMED_FUN, "Expected identifier in function declaration.");

    // Start building the type annotation
    auto type_annotation = std::make_shared<Annotation::Function>(
        std::vector<std::pair<TokenType, std::shared_ptr<Annotation>>>(),
        std::make_shared<Annotation::Segmented>("void"),
        KW_CONST,
        false
    );

    // Next, the parameters
    std::vector<std::shared_ptr<Decl::Var>> parameters;
    grouping_tokens.push(TOK_RIGHT_PAREN);
    consume(TOK_LEFT_PAREN, E_NO_LPAREN_IN_FUN_DECL, "Expected '(' after function name.");
    if (!check({TOK_RIGHT_PAREN})) {
        do {
            match({KW_CONST, KW_VAR});
            auto variable = std::dynamic_pointer_cast<Decl::Var>(var_decl());
            if (variable == nullptr) {
                ErrorLogger::inst().log_error(peek().location, E_IMPOSSIBLE, "var_decl did not return a variable in function declaration.");
                throw ParserException();
            }
            // Parameters are not allowed to have a type annotation of `auto`
            if (variable->type_annotation->to_string() == "auto") {
                ErrorLogger::inst().log_error(variable->name.location, E_AUTO_IN_PARAM, "Parameters cannot have type 'auto'.");
                throw ParserException();
            }
            parameters.push_back(variable);
            type_annotation->params.push_back({variable->declarer, variable->type_annotation});
        } while (match({TOK_COMMA}) && !check({TOK_RIGHT_PAREN}));
    }
    consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_PARAMS, "Expected ')' after function parameters.");

    // Next, the return variable
    std::shared_ptr<Decl::Var> return_var = nullptr;
    if (match({TOK_COLON})) {
        Token return_token = Token{
            TOK_IDENT,
            "__return_val__",
            std::any(),
            previous().location,
        };

        bool var_found = match({KW_VAR});
        type_annotation->return_annotation = annotation();
        if (var_found) {
            type_annotation->return_declarer = KW_VAR;
        }
        return_var = std::make_shared<Decl::Var>(type_annotation->return_declarer, return_token, type_annotation->return_annotation, nullptr);
    }

    std::vector<std::shared_ptr<Stmt>> body;
    consume(TOK_LEFT_BRACE, E_NO_LBRACE_IN_FUN_DECL, "Expected '{' before function body.");
    while (match({TOK_NEWLINE}))
        ; // Skip over newlines
    while (!check({TOK_RIGHT_BRACE}) && !is_at_end()) {
        body.push_back(statement());
        while (match({TOK_NEWLINE}))
            ; // Skip over newlines
    }
    consume(TOK_RIGHT_BRACE, E_UNMATCHED_BRACE_IN_FUN_DECL, "Expected '}' after function body.");
    return std::make_shared<Decl::Fun>(declarer, name, parameters, return_var, type_annotation, body);
}

std::shared_ptr<Decl> Parser::extern_fun_decl(bool is_variadic) {
    TokenType declarer = KW_FUN;
    // Get the function name
    Token name = consume(TOK_IDENT, E_UNNAMED_FUN, "Expected identifier in function declaration.");

    // Start building the type annotation
    auto type_annotation = std::make_shared<Annotation::Function>(
        std::vector<std::pair<TokenType, std::shared_ptr<Annotation>>>(),
        std::make_shared<Annotation::Segmented>("void"),
        KW_CONST,
        is_variadic
    );

    // Next, the parameters
    // Unlike internal functions, extern functions do not use named parameters
    grouping_tokens.push(TOK_RIGHT_PAREN);
    consume(TOK_LEFT_PAREN, E_NO_LPAREN_IN_FUN_DECL, "Expected '(' after function name.");
    if (!check({TOK_RIGHT_PAREN})) {
        do {
            type_annotation->params.push_back({KW_CONST, annotation()});
        } while (match({TOK_COMMA}) && !check({TOK_RIGHT_PAREN}));
    }
    consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_PARAMS, "Expected ')' after function parameters.");

    // Next, the return type
    if (match({TOK_COLON})) {
        Token return_token = Token{
            TOK_IDENT,
            "__return_val__",
            std::any(),
            previous().location,
        };

        bool var_found = match({KW_VAR});
        type_annotation->return_annotation = annotation();
        if (var_found) {
            type_annotation->return_declarer = KW_VAR;
        }
    }
    // There is no return variable for extern functions

    return std::make_shared<Decl::ExternFun>(declarer, name, type_annotation);
}

std::shared_ptr<Decl> Parser::struct_decl() {
    // Should be KW_STRUCT
    TokenType declarer = previous().tok_type;
    // Get the struct name
    Token name = consume(TOK_IDENT, E_UNNAMED_STRUCT, "Expected identifier in struct declaration.");

    std::vector<std::shared_ptr<Decl>> declarations;

    consume(TOK_LEFT_BRACE, E_NO_LBRACE_IN_STRUCT_DECL, "Expected '{' before struct body.");
    while (match({TOK_NEWLINE}))
        ; // Skip over newlines
    while (!check({TOK_RIGHT_BRACE}) && !is_at_end()) {
        auto stmt = std::dynamic_pointer_cast<Stmt::Declaration>(declaration_statement());
        if (stmt == nullptr) {
            ErrorLogger::inst().log_error(peek().location, E_IMPOSSIBLE, "declaration_statement did not return a declaration in struct declaration.");
            throw ParserException();
        }
        // If the declaration is a variable, mark it as an instance member
        // TODO: Implement static members later.
        auto var_decl = std::dynamic_pointer_cast<Decl::Var>(stmt->declaration);
        if (var_decl != nullptr) {
            var_decl->is_instance_member = true;
        }

        declarations.push_back(stmt->declaration);
        while (match({TOK_NEWLINE}))
            ; // Skip over newlines
    }
    consume(TOK_RIGHT_BRACE, E_UNMATCHED_BRACE_IN_STRUCT_DECL, "Expected '}' after struct body.");

    return std::make_shared<Decl::Struct>(declarer, name, declarations);
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
    if (match({TOK_BANG, TOK_MINUS, TOK_AMP})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary_expr();
        return std::make_shared<Expr::Unary>(op, right);
    }
    if (match({TOK_STAR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary_expr();
        return std::make_shared<Expr::Dereference>(op, right);
    }
    return call_expr();
}

std::shared_ptr<Expr> Parser::call_expr() {
    std::shared_ptr<Expr> expr = access_index_expr();
    /*
    Valid forms:
    FUNC()
    FUNC(ARG1)
    FUNC(ARG1, ARG2)
    FUNC(ARG1,)
    FUNC(ARG1, ARG2,)
    */
    while (check({TOK_LEFT_PAREN})) {
        grouping_tokens.push(TOK_RIGHT_PAREN);
        advance();
        Token& paren = previous();
        std::vector<std::shared_ptr<Expr>> arguments;

        // If the left expression is an access expression, then it is a method call
        // Pass the address of the left expression as the first argument
        auto left_access = std::dynamic_pointer_cast<Expr::Access>(expr);
        if (left_access != nullptr) {
            arguments.push_back(std::make_shared<Expr::Unary>(Token(TOK_AMP, "&", nullptr, left_access->location), left_access->left));
        }

        // If there are no arguments, we can just return the call expression
        if (!check({TOK_RIGHT_PAREN})) {
            arguments.push_back(expression());
            while (match({TOK_COMMA})) {
                if (check({TOK_RIGHT_PAREN})) {
                    break;
                }
                arguments.push_back(expression());
                if (arguments.size() > 255) {
                    ErrorLogger::inst().log_error(peek().location, E_TOO_MANY_ARGS, "Cannot have more than 255 arguments.");
                    throw ParserException();
                }
            }
        }
        consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_ARGS, "Expected ')' after arguments.");

        expr = std::make_shared<Expr::Call>(expr, paren, arguments);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::access_index_expr() {
    std::shared_ptr<Expr> expr = cast_expr();
    while (true) {
        if (match({TOK_DOT})) {
            Token op = previous();
            // The right side of the dot operator must be an identifier
            Token name = consume(TOK_IDENT, E_NO_IDENT_AFTER_DOT, "Expected identifier after '.'.");

            // If expr is an LValue, then we can use the LAccess expression
            // Otherwise, we can use the Access expression
            auto left_lvalue = std::dynamic_pointer_cast<Expr::LValue>(expr);
            if (left_lvalue != nullptr) {
                expr = std::make_shared<Expr::LAccess>(left_lvalue, op, name);
            } else {
                expr = std::make_shared<Expr::Access>(expr, op, name);
            }
        } else if (match({TOK_ARROW})) {
            // The single arrow operator is syntactic sugar for dereferencing and then accessing
            Token op = previous();
            // The right side of the arrow operator must be an identifier
            Token name = consume(TOK_IDENT, E_NO_IDENT_AFTER_DOT, "Expected identifier after '->'.");
            auto deref_expr = std::make_shared<Expr::Dereference>(op, expr);
            // Arrow operator is equivalent to (*expr).name
            // Access expressions of this form are always LValues
            expr = std::make_shared<Expr::LAccess>(deref_expr, op, name);
        } else if (match({TOK_LEFT_SQUARE})) {
            Token op = previous();
            grouping_tokens.push(TOK_RIGHT_SQUARE);
            while (match({TOK_NEWLINE}))
                ; // Skip over newlines
            std::shared_ptr<Expr> right = expression();
            consume(TOK_RIGHT_SQUARE, E_UNMATCHED_LEFT_SQUARE, "Expected ']' after expression.");

            // If expr is an LValue, then we can use the LIndex expression
            // Otherwise, we can use the Index expression
            auto left_lvalue = std::dynamic_pointer_cast<Expr::LValue>(expr);
            if (left_lvalue != nullptr) {
                expr = std::make_shared<Expr::LIndex>(left_lvalue, op, right);
            } else {
                expr = std::make_shared<Expr::Index>(expr, op, right);
            }
        } else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<Expr> Parser::cast_expr() {
    std::shared_ptr<Expr> expr = primary_expr();
    if (match({KW_AS})) {
        Token op = previous();
        std::shared_ptr<Annotation> type_annotation = annotation();
        expr = std::make_shared<Expr::Cast>(expr, op, type_annotation);
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
    if (check({TOK_LEFT_SQUARE})) {
        Token& bracket = peek();
        grouping_tokens.push(TOK_RIGHT_SQUARE);
        advance();
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
        return std::make_shared<Expr::Array>(elements, bracket);
    }
    if (check({TOK_LEFT_PAREN})) {
        Token& paren = peek();
        grouping_tokens.push(TOK_RIGHT_PAREN);
        advance();
        std::vector<std::shared_ptr<Expr>> expressions;
        if (match({TOK_RIGHT_PAREN})) {
            // Empty tuple
            return std::make_shared<Expr::Tuple>(expressions, paren); // expressions is empty
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
            return std::make_shared<Expr::Tuple>(expressions, paren);
        }
        consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_GROUPING, "Expected ')' after expression.");
        return expressions[0];
    }
    if (match({TOK_COLON})) {
        return object_expr();
    }

    ErrorLogger::inst().log_error(peek().location, E_NOT_AN_EXPRESSION, "Expected expression.");
    if (Scanner::keywords.find(peek().lexeme) != Scanner::keywords.end()) {
        ErrorLogger::inst().log_note(peek().location, "`" + peek().lexeme + "` is reserved.");
    }
    throw ParserException();
}

std::shared_ptr<Expr> Parser::object_expr() {

    Token& colon = previous();

    auto type_annotation = annotation();
    // This must be a segmented annotation
    auto seg_type_annotation = std::dynamic_pointer_cast<Annotation::Segmented>(type_annotation);
    if (seg_type_annotation == nullptr) {
        ErrorLogger::inst().log_error(previous().location, E_INVALID_OBJ_TYPE, "Expected segmented type annotation.");
        throw ParserException();
    }

    // Consume the left brace
    consume(TOK_LEFT_BRACE, E_NO_LBRACE_IN_OBJ_EXPR, "Expected '{' before object expression.");

    Dictionary<std::string, std::shared_ptr<Expr>> fields;

    while (match({TOK_NEWLINE}))
        ; // Skip over newlines

    while (!check({TOK_RIGHT_BRACE}) && !is_at_end()) {
        Token name = consume(TOK_IDENT, E_NO_IDENT_IN_OBJ, "Expected identifier in object expression.");
        consume(TOK_COLON, E_MISSING_COLON_IN_OBJ, "Expected ':' after object field name.");
        std::shared_ptr<Expr> value = expression();
        fields.insert(name.lexeme, value);
        while (match({TOK_NEWLINE}) || match({TOK_COMMA}))
            ; // Skip over newlines or commas
    }

    consume(TOK_RIGHT_BRACE, E_UNMATCHED_BRACE_IN_OBJ_EXPR, "Expected '}' after object expression.");

    return std::make_shared<Expr::Object>(colon, seg_type_annotation, fields);
}

// MARK: Annotations

std::shared_ptr<Annotation> Parser::annotation() {
    std::shared_ptr<Annotation> type_annotation;
    if (check({TOK_IDENT})) {
        type_annotation = segmented_annotation();
    } else if (match({KW_FUN})) {
        type_annotation = function_annotation();
    } else if (match({TOK_LEFT_PAREN})) {
        type_annotation = tuple_annotation();
    } else {
        ErrorLogger::inst().log_error(peek().location, E_INVALID_TYPE_ANNOTATION, "Expected valid type annotation.");
        throw ParserException();
    }
    return type_annotation;
}

std::shared_ptr<Annotation> Parser::segmented_annotation() {

    std::shared_ptr<Annotation> type_annotation = std::make_shared<Annotation::Segmented>(std::vector<std::shared_ptr<Annotation::Segmented::Class>>());

    auto seg_type_annotation = std::dynamic_pointer_cast<Annotation::Segmented>(type_annotation);

    do {
        Token name = consume(TOK_IDENT, E_MISSING_IDENT_IN_TYPE, "Expected identifier in type annotation.");
        auto temp = std::make_shared<Annotation::Segmented::Class>(name.lexeme, std::vector<std::shared_ptr<Annotation>>());
        if (check({TOK_LT})) {
            grouping_tokens.push(TOK_GT);
            advance();
            if (!check({TOK_GT})) {
                temp->type_args.push_back(annotation());
                while (match({TOK_COMMA})) {
                    if (check({TOK_GT})) {
                        break;
                    }
                    temp->type_args.push_back(annotation());
                }
            }
            consume(TOK_GT, E_UNMATCHED_ANGLE_IN_TYPE, "Expected '>' after type arguments.");
        }
        seg_type_annotation->classes.push_back(temp);
    } while (match({TOK_COLON_COLON}));

    resolve_annotation(seg_type_annotation);

    while (check({TOK_STAR, TOK_LEFT_SQUARE})) {
        if (match({TOK_STAR})) {
            type_annotation = std::make_shared<Annotation::Pointer>(type_annotation);
        }
        if (match({TOK_LEFT_SQUARE})) {
            consume(TOK_RIGHT_SQUARE, E_UNMATCHED_SQUARE_IN_TYPE, "Expected ']' after array type.");
            type_annotation = std::make_shared<Annotation::Array>(type_annotation);
        }
    }

    return type_annotation;
}

std::shared_ptr<Annotation::Function> Parser::function_annotation() {
    std::vector<std::pair<TokenType, std::shared_ptr<Annotation>>> params;
    std::shared_ptr<Annotation> ret;

    consume(TOK_LEFT_PAREN, E_NO_LPAREN_IN_FUN_TYPE, "Expected '(' after 'fun' in type annotation.");
    grouping_tokens.push(TOK_RIGHT_PAREN);
    while (match({TOK_NEWLINE}))
        ; // Skip over newlines

    if (!check({TOK_RIGHT_PAREN})) {
        if (match({KW_VAR})) {
            auto param = std::make_pair(KW_VAR, annotation());
            params.push_back(param);
        } else {
            auto param = std::make_pair(KW_CONST, annotation());
            params.push_back(param);
        }
        while (match({TOK_COMMA})) {
            if (check({TOK_RIGHT_PAREN})) {
                break;
            }
            if (match({KW_VAR})) {
                auto param = std::make_pair(KW_VAR, annotation());
                params.push_back(param);
            } else {
                auto param = std::make_pair(KW_CONST, annotation());
                params.push_back(param);
            }
        }
    }

    consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_TYPE, "Expected ')' after function parameters.");
    consume(TOK_DOUBLE_ARROW, E_NO_ARROW_IN_FUN_TYPE, "Expected '=>' after function parameters.");

    TokenType return_declarer = match({KW_VAR}) ? KW_VAR : KW_CONST;

    ret = annotation();

    return std::make_shared<Annotation::Function>(params, ret, return_declarer, false);
}

std::shared_ptr<Annotation::Tuple> Parser::tuple_annotation() {
    std::vector<std::shared_ptr<Annotation>> tuple_annotations;

    if (match({TOK_RIGHT_PAREN})) {
        if (match({TOK_DOUBLE_ARROW})) {
            // This is an error
            ErrorLogger::inst().log_error(previous().location, E_ARROW_IN_NON_FUN_TYPE, "Function type must be specified with 'fun' keyword.");
            throw ParserException();
        }
        return std::make_shared<Annotation::Tuple>(tuple_annotations);
    }

    tuple_annotations.push_back(annotation());
    while (match({TOK_COMMA})) {
        if (check({TOK_RIGHT_PAREN})) {
            break;
        }
        tuple_annotations.push_back(annotation());
    }

    consume(TOK_RIGHT_PAREN, E_UNMATCHED_PAREN_IN_TYPE, "Expected ')' after tuple type.");

    if (match({TOK_DOUBLE_ARROW})) {
        // This is an error
        ErrorLogger::inst().log_error(previous().location, E_ARROW_IN_NON_FUN_TYPE, "Function type must be specified with 'fun' keyword.");
        throw ParserException();
    }

    return std::make_shared<Annotation::Tuple>(tuple_annotations);
}

void Parser::resolve_annotation(std::shared_ptr<Annotation::Segmented>& /*annotation*/) {
    // Right now, we don't need to do anything here.
}

std::vector<std::shared_ptr<Stmt>> Parser::parse(const std::vector<std::shared_ptr<Token>>& tokens) {
    this->tokens = tokens;
    std::vector<std::shared_ptr<Stmt>> statements;

    while (current != tokens.size()) {
        while (match({TOK_NEWLINE}))
            ; // Skip over newlines
        while (!is_at_end()) {
            statements.push_back(statement());
            while (match({TOK_NEWLINE}))
                ; // Skip over newlines
        }
        current++;
        statements.push_back(std::make_shared<Stmt::EndOfFile>());
    }

    return statements;
}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;

    while (current != tokens.size()) {
        while (match({TOK_NEWLINE}))
            ; // Skip over newlines
        while (!is_at_end()) {
            statements.push_back(statement());
            // Note: Theoretically, we could enforce "declarations only" by calling declaration_statement() here
            // However, this would make the parser less flexible and harder to test.
            // We enforce this in the type checker instead; errors usually begin with "E_GLOBAL_".
            while (match({TOK_NEWLINE}))
                ; // Skip over newlines
        }
        current++;
        statements.push_back(std::make_shared<Stmt::EndOfFile>());
    }

    return statements;
}
