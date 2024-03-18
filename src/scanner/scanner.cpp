#include "scanner.h"
#include "../logger/logger.h"
#include <cctype>

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

char Scanner::peek_next(int lookahead) {
    if (current + lookahead >= source->length())
        return '\0';
    return (*source)[current + lookahead];
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

Token Scanner::make_token(TokenType tok_type, const std::any& literal) const {
    std::string text = source->substr(start, current - start);
    auto location = Location{
        filename,           // The name of the file where the token is located.
        line,               // The line number of the token.
        start - line_index, // The column number of the token.
        current - start,    // The length of the token.
        line_index,         // The index of the line in the source code string where the token is located.
        source              // A shared pointer to the source code string.
    };
    return Token{tok_type, text, literal, location};
}

void Scanner::add_token(TokenType tok_type, const std::any& literal) {
    tokens.push_back(make_token(tok_type, literal));
}

bool Scanner::is_digit(char c, int base) {
    switch (base) {
    case 2:
        return c == '0' || c == '1';
    case 8:
        return c >= '0' && c <= '7';
    case 10:
        return c >= '0' && c <= '9';
    case 16:
        return c >= '0' && c <= '9' ||
               c >= 'a' && c <= 'f' ||
               c >= 'A' && c <= 'F';
    default:
        return false;
        // TODO: Also log an UNREACHABLE error here
    }
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
        if (match('=')) {
            add_token(TOK_MINUS_EQ);
        } else if (match('>')) {
            add_token(TOK_ARROW);
        } else {
            add_token(TOK_MINUS);
        }
        break;
    case '%':
        add_token(match('=') ? TOK_PERCENT_EQ : TOK_PERCENT);
        break;
    case '^':
        add_token(match('=') ? TOK_CARET_EQ : TOK_CARET);
        break;
    case '*':
        // It can be any of these: '*', '*=', '*/'
        if (match('=')) {
            add_token(TOK_STAR_EQ);
        } else if (match('/')) {
            // Not valid since it's not the start of a comment
            Token t = make_token(TOK_STAR_SLASH);
            ErrorLogger::inst()
                .log_error(t, E_CLOSING_UNOPENED_COMMENT, "Closing comment '*/' without opening '/*'.");
        } else {
            add_token(TOK_STAR);
        }
        break;
    case '/':
        // It can be any of these: '/', '/=', '/*'
        if (match('=')) {
            add_token(TOK_SLASH_EQ);
        } else if (match('/')) {
            // Start single-line comment
            single_line_comment();
        } else if (match('*')) {
            // Start multi-line comment
            multi_line_comment();
        } else {
            add_token(TOK_SLASH);
        }
        break;
    case ',':
        add_token(TOK_COMMA);
        break;
    case '\'':
        char_literal();
        break;
    case '"':
        string_literal();
        break;
    case '\n':
        add_token(TOK_NEWLINE);
        line++;
        line_index = current;
        break;
    case '\\':
        if (!is_at_end() && !match('\n')) {
            // Backslash is used for line continuation. Therefore, it is only valid at the end of a line.
            Token t = make_token(TOK_BACKSLASH);
            ErrorLogger::inst()
                .log_error(t, E_NO_LF_AFTER_BACKSLASH, "Expected newline after backslash.");
        }
        break;
    case ';':
        add_token(TOK_SEMICOLON);
        break;
    case '&':
        if (match('&')) {
            add_token(match('=') ? TOK_AMP_AMP_EQ : TOK_AMP_AMP);
        } else if (match('=')) {
            add_token(TOK_AMP_EQ);
        } else {
            add_token(TOK_AMP);
        }
        break;
    case '|':
        if (match('|')) {
            add_token(match('=') ? TOK_BAR_BAR_EQ : TOK_BAR_BAR);
        } else if (match('=')) {
            add_token(TOK_BAR_EQ);
        } else {
            add_token(TOK_BAR);
        }
        break;
    case '!':
        add_token(match('=') ? TOK_BANG_EQ : TOK_BANG);
        break;
    case '=':
        if (match('=')) {
            add_token(TOK_EQ_EQ);
        } else if (match('>')) {
            add_token(TOK_DOUBLE_ARROW);
        } else {
            add_token(TOK_EQ);
        }
        break;
    case '>':
        add_token(match('=') ? TOK_GE : TOK_GT);
        break;
    case '<':
        add_token(match('=') ? TOK_LE : TOK_LT);
        break;
    case '.':
        // Can be '.' or '..' or '...'
        if (match('.')) {
            add_token(match('.') ? TOK_TRIPLE_DOT : TOK_DOT_DOT);
        } else if (is_digit(peek())) {
            numeric_literal();
        } else {
            add_token(TOK_DOT);
        }
        break;
    case ':':
        // Can be ':' or '::'
        add_token(match(':') ? TOK_COLON_COLON : TOK_COLON);
        break;
    case ' ':
    case '\r':
    case '\t':
        // Ignore this whitespace
        break;
    default:
        if (is_digit(c)) {
            numeric_literal();
        } else if (is_alpha(c)) {
            identifier();
        } else {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_UNEXPECTED_CHAR, "Unexpected character.");
        }
        // TODO: Implement the rest of the scanner
    }
}

void Scanner::single_line_comment() {
    while (!is_at_end() && peek() != '\n') {
        advance();
    }
}

void Scanner::multi_line_comment() {
    while (!is_at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            advance();
            advance();
            return;
        }
        if (peek() == '\n') {
            line++;
            line_index = current;
        }
        advance();
    }
    Token t = make_token(TOK_EOF);
    ErrorLogger::inst()
        .log_error(t, E_UNCLOSED_COMMENT, "Comment was not closed at the end of the file.");
}

char Scanner::read_escape_sequence() {
    char c = advance();
    switch (c) {
    case 'b':
        return '\b'; // Backspace
    case 'f':
        return '\f'; // Form feed
    case 'n':
        return '\n'; // Line feed (newline)
    case 'r':
        return '\r'; // Carriage return
    case 't':
        return '\t'; // Horizontal tab
    case '0':
        return '\0'; // Null character
    case '\\':
        return '\\'; // Backslash
    case '\'':
        return '\''; // Single quote
    case '"':
        return '"'; // Double quote
    case '%':
        return '%'; // Percent (just in case we need it for formatting)
    case '{':
        return '{'; // Left brace (just in case we need it for formatting)
    default:
        Token t = make_token(TOK_UNKNOWN);
        ErrorLogger::inst()
            .log_error(t, E_ILLEGAL_ESC_SEQ, "Illegal escape sequence.");
        return c;
    }
}

void Scanner::char_literal() {
    // Ensure the next character is not a newline
    if (is_at_end() || peek() == '\n') {
        Token t = make_token(TOK_SINGLE_QUOTE);
        ErrorLogger::inst()
            .log_error(t, E_UNCLOSED_CHAR, "Character literal was not closed after the first character.");
        return;
    }
    if (peek() == '\'') {
        Token t = make_token(TOK_SINGLE_QUOTE);
        ErrorLogger::inst()
            .log_error(t, E_EMPTY_CHAR, "Empty character literal found.");
        return;
    }

    // Read the character
    if (match('\\')) {
        char c = read_escape_sequence();
        add_token(TOK_CHAR, c);
    } else {
        add_token(TOK_CHAR, advance());
    }

    // Ensure the character literal is closed
    if (!match('\'')) {
        Token t = make_token(TOK_SINGLE_QUOTE);
        ErrorLogger::inst()
            .log_error(t, E_UNCLOSED_CHAR, "Character literal was not closed after the first character.");
    }
}

void Scanner::string_literal() {
    bool is_multi_line = false;
    std::string literal;

    if (peek() == '"' && peek_next() == '"') {
        advance();
        advance();
        is_multi_line = true;
    }
    while (true) {
        if (is_multi_line && is_at_end()) {
            Token t = make_token(TOK_EOF);
            ErrorLogger::inst()
                .log_error(t, E_UNCLOSED_MULTI_LINE_STRING, "Multi-line string literal was not closed at the end of the file.");
            return;
        }
        if (!is_multi_line && peek() == '\n') {
            Token t = make_token(TOK_NEWLINE);
            ErrorLogger::inst()
                .log_error(t, E_UNCLOSED_STRING, "Single-line string literal was not closed at the end of the line.");
            return;
        }
        if (is_multi_line && peek() == '"' && peek_next() == '"' && peek_next(2) == '"') {
            advance();
            advance();
            advance();
            break;
        }
        if (!is_multi_line && peek() == '"') {
            advance();
            break;
        }
        if (peek() == '\\') {
            literal += read_escape_sequence();
        } else {
            literal += advance();
        }
    }
    add_token(TOK_STR, literal);
}

void Scanner::numeric_literal() {

    /*
    Under the following rules, numbers of these forms are allowed:
    - Decimal: 1234567890
    - Binary: 0b101010
    - Octal: 0o1234567
    - Hexadecimal: 0x1234abcdef
    - Floating point: 123.456
    - Exponential notation: 1.23e4 1.23E4 1.23e+4 1.23e-4
    - Exponent in decimal integer: 123e4 123E4 123e+4 123e-4
    - Underscores: 1_000_000
    - Float without leading 0: .123
    - Float without trailing 0: 123.
    The following forms specifically are not allowed:
    - Multiple decimal points: 123.456.789
    - Decimal point in non-decimal number: 0x123.456
    - Exponential notation without digits: 1.23e
    - Very large integers and floating point numbers
    */

    std::string num_string;
    char first_digit = source->at(current - 1); // This is because advance() has already been called.
    num_string += first_digit;
    bool is_float = num_string.at(0) == '.'; // If the first digit is a decimal point, it's a float. If not, number can be changed to a float later.
    int base = 10;
    // If the first digit is 0 (the number is not a float (yet)), check for base
    if (first_digit == '0') {
        if (peek() == 'x') {
            // Hexadecimal number
            advance();
            base = 16;
        } else if (peek() == 'b') {
            // Binary number
            advance();
            base = 2;
        } else if (peek() == 'o') {
            // Octal number
            advance();
            base = 8;
        }
    }

    // Read the number
    while (is_digit(peek(), base) || peek() == '_' || peek() == '.') {
        if (peek() != '_') {
            num_string += advance();
        } else if (peek() == '.') {
            if (is_float) {
                Token t = make_token(TOK_UNKNOWN);
                ErrorLogger::inst()
                    .log_error(t, E_MULTIPLE_DECIMAL_POINTS, "Multiple decimal points in a number.");
                return;
            } else if (base != 10) {
                Token t = make_token(TOK_UNKNOWN);
                ErrorLogger::inst()
                    .log_error(t, E_NON_DECIMAL_FLOAT, "Floating point numbers must be in base 10.");
                return;
            } else {
                is_float = true;
                num_string += advance();
            }
        } else {
            advance();
        }
    }
    // Check for exponential notation
    if (base == 10 && (peek() == 'e' || peek() == 'E')) {
        is_float = true; // Exponential notation always makes the number a float
        num_string += advance();
        if (peek() == '+' || peek() == '-') {
            num_string += advance();
        }
        if (!is_digit(peek())) {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_NO_DIGITS_IN_EXPONENT, "Exponential notation must have at least one digit in the exponent.");
            return;
        }
        while (is_digit(peek()) || peek() == '_') {
            if (peek() != '_') {
                num_string += advance();
            } else {
                advance();
            }
        }
    }

    // The next character should not be a letter or a digit outside the base
    // This would otherwise make numbers confusing to read.
    // Numbers should be followed by a space, a newline, or a non-alphanumeric character.
    if (is_alpha_numeric(peek())) {
        Token t = make_token(TOK_UNKNOWN);
        ErrorLogger::inst()
            .log_error(t, E_NON_DIGIT_IN_NUMBER, "Numbers should be followed by a space, a newline, or a non-alphanumeric character.");
        return;
    }

    if (is_float) {
        try {
            double num = std::stod(num_string);
            add_token(TOK_FLOAT, num);
        } catch (const std::invalid_argument& e) {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_CONVERSION, "An unknown error occurred while parsing a floating point number.");
        } catch (const std::out_of_range& e) {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_FLOAT_TOO_LARGE, "Floating point number is too large.");
        }
    } else {
        try {
            long long num = std::stoll(num_string, nullptr, base);
            add_token(TOK_INT, num);
        } catch (const std::invalid_argument& e) {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_CONVERSION, "An unknown error occurred while parsing an integer.");
        } catch (const std::out_of_range& e) {
            Token t = make_token(TOK_UNKNOWN);
            ErrorLogger::inst()
                .log_error(t, E_INT_TOO_LARGE, "Integer is too large.");
        }
    }
}
