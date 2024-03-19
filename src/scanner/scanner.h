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
     * @brief Returns the character at the current index plus the lookahead without advancing the scanner.
     *
     * @param lookahead The number of characters to look ahead. Defaults to 1. If lookahead is 0, effectively returns the character at the current index like peek().
     * Warning: Keep this value low to avoid performance issues. Currently, the maximum value is 2.
     * @return char The character at the current index plus the lookahead. If the current index plus lookahead is greater than the length of the source code, returns '\0'.
     */
    char peek_next(int lookahead = 1);

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
     * @brief Creates a new token without adding it to the list of tokens.
     * Useful for error logging where a token is needed but should not be added to the list of tokens.
     *
     * @param tok_type The type of the token.
     * @param literal The literal text of the token, if applicable.
     * @return Token The new token.
     */
    Token make_token(TokenType tok_type, const std::any& literal = std::any()) const;

    /**
     * @brief Adds a new token to the list of tokens.
     *
     * @param tok_type The type of the token.
     * @param literal The literal text of the token, if applicable.
     */
    void add_token(TokenType tok_type, const std::any& literal = std::any());

    /**
     * @brief Checks if the current character is a digit and advances the scanner if it is.
     *
     * @param c The character to check.
     * @param base The base of the number. Defaults to 10.
     * @return true If the character is a digit within the bounds for the base.
     * @return false Otherwise.
     */
    bool is_digit(char c, int base = 10);

    /**
     * @brief Checks if the passed-in character is an alphabetic character or an underscore.
     *
     * @param c The character to check.
     * @return true If the character is A-Z, a-z, or _.
     * @return false Otherwise.
     */
    bool is_alpha(char c);

    /**
     * @brief Checks if the passed-in character is alphanumeric or an underscore.
     *
     * @param c The character to check.
     * @return true If the character is A-Z, a-z, 0-9, or _.
     * @return false Otherwise.
     */
    bool is_alpha_numeric(char c);

    /**
     * @brief Scans a new token from the source code and adds it to the list of tokens.
     *
     */
    void scan_token();

    /**
     * @brief Starts advancing the scanner, ignoring characters until it finds a newline token.
     *
     */
    void single_line_comment();

    /**
     * @brief Starts advancing the scanner, ignoring characters until it finds a star-slash token.
     *
     */
    void multi_line_comment();

    /**
     * @brief Reads an escape sequence from the source code and returns the corresponding character.
     *
     * @return char The character corresponding to the escape sequence.
     */
    char read_escape_sequence();

    /**
     * @brief Reads a character token from the source code and adds it to the list of tokens.
     *
     */
    void char_literal();

    /**
     * @brief Reads a string token from the source code and adds it to the list of tokens.
     * If the first two characters are double quotes, reads a multi-line string literal.
     */
    void string_literal();

    /**
     * @brief Reads a number token from the source code and adds it to the list of tokens.
     *
     */
    void numeric_literal();

    /**
     * @brief Reads an identifier token from the source code and adds it to the list of tokens.
     * If the identifier is a keyword, adds the keyword token instead.
     * If the identifier is a boolean or null literal, adds the corresponding token instead.
     * If the identifier is 'inf' or 'NaN', adds the corresponding float literal token instead.
     *
     */
    void identifier();

public:
    /**
     * @brief Construct a new Scanner object.
     *
     */
    Scanner() = default;

    /**
     * @brief Scans the source code, adding tokens to the stored vector of Tokens.
     *
     * @param filename A shared ptr to the name of the file. Necessary for storing location information in Tokens.
     * @param source_code A shared ptr a string containing the entire source code in the file.
     */
    void scan_file(std::shared_ptr<std::string> filename, std::shared_ptr<std::string> source_code);

    /**
     * @brief Get the tokens object
     *
     * @return const std::vector<Token>& The vector of tokens accumulated.
     */
    const std::vector<Token>& get_tokens() const;

    /**
     * @brief Prints out all the tokens from the tokens vector, each on a new line.
     *
     * @param out The output stream. Default is `std::cout`.
     */
    void print_all_tokens(std::ostream& out = std::cout) const;

    /**
     * @brief Clears the tokens stored in the tokens vector.
     *
     */
    void reset_tokens();
};

#endif // SCANNER_H
