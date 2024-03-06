#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief A class for scanning source code into a list of tokens.
 *
 */
class Scanner {
    // A map of keywords to their respective token types. To be initialized in the source file.
    static std::unordered_map<std::string, TokenType> keywords;
    // The source code to be scanned. A shared pointer is used to avoid copying the source code string.
    std::shared_ptr<std::string> source;
    // The name of the file where the source code is located. Used for error messages. A shared pointer is used to avoid copying the filename string.
    std::shared_ptr<std::string> filename;
    // The list of tokens scanned from the source code.
    std::vector<Token> tokens;
    // The index of the first character of the current token.
    unsigned start = 0;
    // The index of the character from the source currently being considered.
    unsigned current = 0;
    // The line number of the current token.
    unsigned line = 1;
    // The index of the first character of the current line.
    unsigned line_index = 0;

    /**
     * @brief Advances the scanner by one character and returns the character at the previous index.
     * E.g. if the current character is 'a', calling advance() will return 'a' and advance the scanner to the next character.
     *
     * @return char The character at the previous index.
     */
    char advance();

    /**
     * @brief Returns the character at the current index without advancing the scanner.
     *
     * @return char The character at the current index.
     */
    char peek();

    /**
     * @brief Returns the character at the next index without advancing the scanner.
     *
     * @return char The character at the next index.
     */
    char peek_next();

    /**
     * @brief Checks if the scanner has reached the end of the source code.
     *
     * @return true If the current index is at the end of the source code.
     * @return false If the current index is not at the end of the source code.
     */
    bool is_at_end();

    /**
     * @brief Checks if the current character is the expected character and advances the scanner if it is.
     *
     * @param expected The character to check for.
     * @return true If the current character is the expected character.
     * @return false If the current character is not the expected character or if the scanner is at the end of the source code.
     */
    bool match(char expected);

    /**
     * @brief Adds a new token to the list of tokens.
     *
     * @param tok_type The type of the token.
     * @param literal The literal text of the token, if applicable.
     */
    void add_token(TokenType tok_type, std::any literal = std::any());
};

#endif // SCANNER_H
