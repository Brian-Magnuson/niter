#ifndef PARSER_H
#define PARSER_H

#include "../logger/error_code.h"
#include "../scanner/token.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include <memory>
#include <vector>

/**
 * @brief A class to parse a vector of tokens into an abstract syntax tree.
 *
 */
class Parser {
    // The vector of tokens to parse.
    std::vector<Token> tokens;
    // The current token index.
    int current = 0;

    /**
     * @brief Returns the current token.
     *
     * @return Token& The token at the current index.
     */
    Token& peek();

    /**
     * @brief Returns the previous token.
     *
     * @return Token& The token at the previous index.
     */
    Token& previous();

    /**
     * @brief Checks if the current token is of the given type.
     *
     * @param tok_type The type to check.
     * @return true If the current token is of the given type.
     * @return false Otherwise.
     */
    bool check(TokenType tok_type);

    /**
     * @brief Checks if an EOF token was reached.
     *
     * @return true If the current token has TokenType TOK_EOF.
     * @return false Otherwise.
     */
    bool is_at_end();

    /**
     * @brief Advances the parser and returns the previous token.
     * E.g., if the current token is the first token, it will return the first token and then advance to the second token.
     *
     * @return Token& The previous token.
     */
    Token& advance();

    /**
     * @brief Check if the current token is any of the given types and advances the parser if it is.
     *
     * @param types The types to check.
     * @return true If the current token is any of the given types.
     * @return false Otherwise.
     */
    bool match(const std::vector<TokenType>& types);

    /**
     * @brief Checks if the current token is of the given type and advances the parser if it is. Otherwise, it logs an error.
     *
     * @param tok_type The type to check.
     * @param error_code The error code to log.
     * @param message The message to be logged with the error.
     * @return Token& The token that was consumed.
     */
    Token& consume(TokenType tok_type, ErrorCode error_code, const std::string& message);

    /**
     * @brief Consumes tokens until a safe token is reached. Used to recover from errors.
     *
     */
    void synchronize();

    /* ==== STATEMENTS ==== */

    /**
     * @brief Parses a generic statement.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed statement.
     */
    std::shared_ptr<Stmt> statement();

    /**
     * @brief Parses a print statement.
     * Print statements begin with the keyword "puts" followed by an expression.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed print statement.
     */
    std::shared_ptr<Stmt> print_statement();

    /**
     * @brief Parses an expression statement, i.e. an expression by itself.
     * Expression statements are expressions followed by either a semicolon or a newline.
     *
     * @return std::shared_ptr<Stmt>
     */
    std::shared_ptr<Stmt> expression_statement();

    /* ==== EXPRESSIONS ==== */

    /**
     * @brief Parses a generic expression.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed expression.
     */
    std::shared_ptr<Expr> expression();

    /**
     * @brief Parses an assignment expression.
     * Assignment expressions are expressions followed by an equal sign and another expression.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed assignment expression.
     */
    std::shared_ptr<Expr> assign_expr();

    /**
     * @brief Parses a logical OR expression.
     * Logical OR expressions are expressions separated by the "or" keyword.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed logical OR expression.
     */
    std::shared_ptr<Expr> or_expr();

    /**
     * @brief Parses a logical AND expression.
     * Logical AND expressions are expressions separated by the "and" keyword.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed logical AND expression.
     */
    std::shared_ptr<Expr> and_expr();

    /**
     * @brief Parses an equality expression.
     * Equality expressions are expressions separated by the "==" or "!=" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed equality expression.
     */
    std::shared_ptr<Expr> equality_expr();

    /**
     * @brief Parses a comparison expression.
     * Comparison expressions are expressions separated by the "<", "<=", ">", or ">=" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed comparison expression.
     */
    std::shared_ptr<Expr> comparison_expr();

    /**
     * @brief Parses a term expression.
     * Term expressions are expressions separated by the "+" or "-" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed term expression.
     */
    std::shared_ptr<Expr> term_expr();

    /**
     * @brief Parses a factor expression.
     * Factor expressions are expressions separated by the "*", "/" or "%" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed factor expression.
     */
    std::shared_ptr<Expr> factor_expr();

    /**
     * @brief Parses a power expression.
     * Power expressions are expressions separated by the "^" operator.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed power expression.
     */
    std::shared_ptr<Expr> power_expr();

    /**
     * @brief Parses a unary expression.
     * Unary expressions are expressions preceded by the "-" or "!" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed unary expression.
     */
    std::shared_ptr<Expr> unary_expr();

    /**
     * @brief Parses an access expression.
     * Access expressions are expressions followed by a "." or "->" or a set of brackets.
     * There is also another expression to specify the field or element to access.
     * They are used to access fields or elements of an object or array.
     *
     * @return std::shared_ptr<Expr>
     */
    std::shared_ptr<Expr> access_expr();

    /**
     * @brief Parses a call expression.
     * Call expressions are expressions followed by a pair of parentheses containing zero or more arguments.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed call expression.
     */
    std::shared_ptr<Expr> call_expr();

    /**
     * @brief Parses a primary expression.
     * Primary expressions are the most basic expressions and can be literals, identifiers, or grouped expressions.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed primary expression.
     */
    std::shared_ptr<Expr> primary_expr();

public:
    /**
     * @brief Construct a new Parser object.
     *
     * @param tokens The vector of tokens to parse.
     */
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    /**
     * @brief Parses the vector of tokens into an abstract syntax tree.
     *
     * @return std::vector<std::shared_ptr<Stmt>> A vector of AST statements.
     */
    std::vector<std::shared_ptr<Stmt>> parse();
};

#endif // PARSER_H
