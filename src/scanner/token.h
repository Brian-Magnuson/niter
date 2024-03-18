#ifndef TOKEN_H
#define TOKEN_H

#include <any>
#include <memory>
#include <string>

/**
 * @brief An enum to represent the different types of tokens.
 *
 * Token types all begin with TOK or KW, where TOK represents a symbol or literal token, and KW represents a keyword token.
 */
enum TokenType {

    TOK_EOF,
    TOK_UNKNOWN, // For tokens not recognized by the scanner

    // Symbols

    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_LEFT_BRACE,
    TOK_RIGHT_BRACE,
    TOK_LEFT_SQUARE,
    TOK_RIGHT_SQUARE,

    TOK_PLUS,
    TOK_PLUS_EQ,
    TOK_MINUS,
    TOK_MINUS_EQ,
    TOK_STAR,
    TOK_STAR_EQ,
    TOK_STAR_SLASH, // Comments only
    TOK_SLASH,
    TOK_SLASH_EQ,
    TOK_SLASH_SLASH, // Comments only
    TOK_SLASH_STAR,  // Comments only
    TOK_PERCENT,
    TOK_PERCENT_EQ,
    TOK_CARET,
    TOK_CARET_EQ,

    TOK_COMMA,

    TOK_SINGLE_QUOTE,  // Character literal only
    TOK_DOUBLE_QUOTE,  // String literal only
    TOK_TRIPLE_QUOTES, // Multi-line string literal only

    TOK_NEWLINE,
    TOK_BACKSLASH, // Line continuation only
    TOK_SEMICOLON,

    TOK_AMP,
    TOK_AMP_AMP,
    TOK_AMP_EQ,
    TOK_AMP_AMP_EQ,
    TOK_BAR,
    TOK_BAR_BAR,
    TOK_BAR_EQ,
    TOK_BAR_BAR_EQ,
    TOK_BANG,
    TOK_BANG_EQ,
    TOK_EQ,
    TOK_EQ_EQ,
    TOK_GT,
    TOK_GE,
    TOK_LT,
    TOK_LE,
    TOK_DOT,
    TOK_DOT_DOT,
    TOK_TRIPLE_DOT,
    TOK_COLON,
    TOK_COLON_COLON,

    TOK_ARROW,
    TOK_DOUBLE_ARROW,

    // Literals

    TOK_IDENT,
    TOK_CHAR,
    TOK_STR,
    TOK_CSTR,
    TOK_TSTR,
    TOK_INT,   // Literals stored as 64-bit integers internally
    TOK_FLOAT, // Literals stored as double-precision floating-point numbers internally

    // Keywords

    KW_AND,
    KW_OR,
    KW_NOT,
    KW_IF,
    KW_ELSE,
    KW_LOOP,
    KW_WHILE,
    KW_FOR,
    KW_IN,
    KW_BREAK,
    KW_CONTINUE,
    KW_RETURN,
    KW_YIELD,

    KW_VAR,
    KW_CONST,
    KW_FUN,
    KW_OPER,
    KW_STRUCT,
    KW_ENUM,
    KW_TYPE,
    KW_INTERFACE,
    KW_USING,
    KW_NAMESPACE,
    KW_STATIC,
    KW_GLOBAL,

    KW_TRUE,
    KW_FALSE,
    KW_NIL,
    KW_SELF,

    KW_AS,
    KW_TYPEOF,
    KW_IS,
    KW_ALLOC,
    KW_DEALLOC,

    KW_EXTERN,
};

/**
 * @brief A struct to represent the location of a token in the source code.
 *
 */
struct Location {
    // The name of the file where the token is located.
    std::shared_ptr<std::string> file_name;
    // The line number of the token.
    unsigned line;
    // The column number of the token.
    unsigned column;
    // The length of the token.
    unsigned length;
    // The index of the line in the source code string where the token is located.
    unsigned line_index;
    // A shared pointer to the source code string.
    std::shared_ptr<std::string> source_code;
};

/**
 * @brief Returns a string representation of the given token type.
 *
 * E.g. token_type_to_string(TOK_IDENT) returns "TOK_IDENT"
 *
 * @param type The token type to convert to a string.
 * @return std::string The string representation of the token type.
 */
std::string token_type_to_string(TokenType type);

/**
 * @brief A class to represent a token scanned from the source code.
 *
 */
class Token {
public:
    // The type of the token.
    const TokenType tok_type;
    // A string representing the lexeme of the token from the source code.
    const std::string lexeme;
    // The literal value of the token, if it has one.
    const std::any literal;
    // The location of the token in the source code.
    const Location location;

    /**
     * @brief Construct a new Token object
     *
     * @param tok_type The type of the token.
     * @param lexeme The string representing the lexeme of the token from the source code.
     * @param literal The literal value of the token, if it has one.
     * @param location The location of the token in the source code, including the line number, column number, and length.
     */
    Token(
        TokenType tok_type,
        const std::string& lexeme,
        const std::any& literal,
        Location location
    )
        : tok_type(tok_type), lexeme(lexeme), literal(literal), location(location) {}

    /**
     * @brief A string representation of the token specifically for printing and debugging.
     *
     * @return std::string A string representation of the token. E.g. "[TOK_IDENT, 'foo', 1:1:3]"
     */
    std::string to_string() const;
};

#endif // TOKEN_H
