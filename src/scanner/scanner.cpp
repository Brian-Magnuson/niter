#include "scanner.h"

std::unordered_map<std::string, TokenType> Scanner::keywords = {
    {"and", KW_AND},
    {"or", KW_OR},
    {"not", KW_NOT},
    {"if", KW_IF},
    {"else", KW_ELSE},
    {"loop", KW_LOOP},
    {"while", KW_WHILE},
    {"for", KW_FOR},
    {"in", KW_IN},
    {"break", KW_BREAK},
    {"continue", KW_CONTINUE},
    {"return", KW_RETURN},
    {"yield", KW_YIELD},
    {"var", KW_VAR},
    {"const", KW_CONST},
    {"fun", KW_FUN},
    {"oper", KW_OPER},
    {"struct", KW_STRUCT},
    {"enum", KW_ENUM},
    {"type", KW_TYPE},
    {"interface", KW_INTERFACE},
    {"using", KW_USING},
    {"namespace", KW_NAMESPACE},
    {"static", KW_STATIC},
    {"global", KW_GLOBAL},
    {"true", KW_TRUE},
    {"false", KW_FALSE},
    {"nil", KW_NIL},
    {"self", KW_SELF},
    {"as", KW_AS},
    {"typeof", KW_TYPEOF},
    {"is", KW_IS},
    {"alloc", KW_ALLOC},
    {"dealloc", KW_DEALLOC},
    {"extern", KW_EXTERN}
};

char Scanner::advance() {
    current++;
    return (*source)[current - 1];
}

char Scanner::peek() {
    if (is_at_end())
        return '\0';
    return (*source)[current];
}

char Scanner::peek_next() {
    if (current + 1 >= source->length())
        return '\0';
    return (*source)[current + 1];
}

bool Scanner::is_at_end() {
    return current >= source->length();
}

bool Scanner::match(char expected) {
    if (is_at_end())
        return false;
    if ((*source)[current] != expected)
        return false;
    current++;
    return true;
}

void Scanner::add_token(TokenType tok_type, std::any literal) {
    std::string text = source->substr(start, current - start);
    auto location = Location{
        filename,           // The name of the file where the token is located.
        line,               // The line number of the token.
        start - line_index, // The column number of the token.
        current - start,    // The length of the token.
        line_index,         // The index of the line in the source code string where the token is located.
        source              // A shared pointer to the source code string.
    };
    tokens.push_back(Token{tok_type, text, literal, location});
}

bool Scanner::is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool Scanner::is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Scanner::is_alpha_numeric(char c) {
    return is_alpha(c) || is_digit(c);
}

void Scanner::scan_token() {
    // Get the next char
    char c = advance();
    switch (c) {
    case '(':
        add_token(TOK_LEFT_PAREN);
        break;
    case ')':
        add_token(TOK_RIGHT_PAREN);
        break;
    case '{':
        add_token(TOK_LEFT_BRACE);
        break;
    case '}':
        add_token(TOK_RIGHT_BRACE);
        break;
    case '[':
        add_token(TOK_LEFT_SQUARE);
        break;
    case ']':
        add_token(TOK_RIGHT_SQUARE);
        break;
    case '+':
        add_token(match('=') ? TOK_PLUS_EQ : TOK_PLUS);
        break;
    case '-':
        add_token(match('=') ? TOK_MINUS_EQ : TOK_MINUS);
        break;
    case '*':
        add_token(match('=') ? TOK_STAR_EQ : TOK_STAR);
        break;
    }
    // TODO: Implement the rest of the scanner
}
