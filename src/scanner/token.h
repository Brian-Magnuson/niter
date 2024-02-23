#ifndef TOKEN_H
#define TOKEN_H

/**
 * @brief An enum to represent the different types of tokens.
 */
enum TokenType {

    TOK_EOF,

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
    TOK_SLASH,
    TOK_SLASH_EQ,
    TOK_SLASH_SLASH,
    TOK_SLASH_STAR,
    TOK_STAR_SLASH,
    TOK_PERCENT,
    TOK_PERCENT_EQ,
    TOK_CARET,
    TOK_CARET_EQ,

    TOK_COMMA,
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
    TOK_BANG_EQUAL,
    TOK_EQ,
    TOK_EQ_EQ,
    TOK_GT,
    TOK_GE,
    TOK_LT,
    TOK_LE,
    TOK_DOT,
    TOK_DOT_DOT,
    TOK_TRIPLE_DOT,

    // Literals

    TOK_IDENT,
    TOK_STR,
    TOK_CSTR,
    TOK_TSTR,
    TOK_INT,
    TOK_FLOAT,

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
    KW_GLOBAL,

    KW_TRUE,
    KW_FALSE,
    KW_NIL,
    KW_SELF,

    KW_AS,
    KW_TYPEOF,
    KW_IS,
    KW_NEW,
    KW_DEL,

    KW_EXTERN,
};

#endif // TOKEN_H
