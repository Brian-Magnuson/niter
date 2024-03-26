#ifndef PARSER_H
#define PARSER_H

#include "../logger/error_code.h"
#include "../scanner/token.h"
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
     * @return Token The token that was consumed.
     */
    Token consume(TokenType tok_type, ErrorCode error_code, const std::string& message);

    /**
     * @brief Checks if the current token is of the given type.
     *
     * @param tok_type The type to check.
     * @return true If the current token is of the given type.
     * @return false Otherwise.
     */
    bool check(TokenType tok_type);

    /**
     * @brief Advances the parser and returns the previous token.
     * E.g., if the current token is the first token, it will return the first token and then advance to the second token.
     *
     * @return Token The previous token.
     */
    Token advance();

    /**
     * @brief Checks if an EOF token was reached.
     *
     * @return true If the current token has TokenType TOK_EOF.
     * @return false Otherwise.
     */
    bool is_at_end();

    /**
     * @brief Returns the current token.
     *
     * @return Token The token at the current index.
     */
    Token peek();

    /**
     * @brief Returns the previous token.
     *
     * @return Token The token at the previous index.
     */
    Token previous();

    /**
     * @brief Consumes tokens until a safe token is reached. Used to recover from errors.
     *
     */
    void synchronize();
};

#endif // PARSER_H
