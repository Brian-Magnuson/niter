#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
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
    // The source code to be scanned.
    std::string source;
    // The name of the file where the source code is located. Used for error messages.
    std::string filename;
    // The list of tokens scanned from the source code.
    std::vector<Token> tokens;
    // The index of the first character of the current token.
    int start = 0;
    // The index of the character from the source currently being considered.
    int current = 0;
    // The line number of the current token.
    int line = 1;

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
};

#endif // SCANNER_H
