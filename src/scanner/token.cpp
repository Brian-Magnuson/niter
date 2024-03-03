#include "token.h"

#include <string>

std::string token_type_to_string(TokenType type) {
    switch (type) {
    case TOK_EOF:
        return "TOK_EOF";
    case TOK_LEFT_PAREN:
        return "TOK_LEFT_PAREN";
    case TOK_RIGHT_PAREN:
        return "TOK_RIGHT_PAREN";
    case TOK_LEFT_BRACE:
        return "TOK_LEFT_BRACE";
    case TOK_RIGHT_BRACE:
        return "TOK_RIGHT_BRACE";
    case TOK_LEFT_SQUARE:
        return "TOK_LEFT_SQUARE";
    case TOK_RIGHT_SQUARE:
        return "TOK_RIGHT_SQUARE";

    case TOK_PLUS:
        return "TOK_PLUS";
    case TOK_PLUS_EQ:
        return "TOK_PLUS_EQ";
    case TOK_MINUS:
        return "TOK_MINUS";
    case TOK_MINUS_EQ:
        return "TOK_MINUS_EQ";
    case TOK_STAR:
        return "TOK_STAR";
    case TOK_STAR_EQ:
        return "TOK_STAR_EQ";
    case TOK_SLASH:
        return "TOK_SLASH";
    case TOK_SLASH_EQ:
        return "TOK_SLASH_EQ";
    case TOK_SLASH_SLASH:
        return "TOK_SLASH_SLASH";
    case TOK_SLASH_STAR:
        return "TOK_SLASH_STAR";
    case TOK_STAR_SLASH:
        return "TOK_STAR_SLASH";
    case TOK_PERCENT:
        return "TOK_PERCENT";
    case TOK_PERCENT_EQ:
        return "TOK_PERCENT_EQ";
    case TOK_CARET:
        return "TOK_CARET";
    case TOK_CARET_EQ:
        return "TOK_CARET_EQ";

    case TOK_COMMA:
        return "TOK_COMMA";

    case TOK_SINGLE_QUOTE:
        return "TOK_SINGLE_QUOTE";
    case TOK_DOUBLE_QUOTE:
        return "TOK_DOUBLE_QUOTE";
    case TOK_TRIPLE_QUOTES:
        return "TOK_TRIPLE_QUOTES";

    case TOK_NEWLINE:
        return "TOK_NEWLINE";
    case TOK_BACKSLASH:
        return "TOK_BACKSLASH";
    case TOK_SEMICOLON:
        return "TOK_SEMICOLON";

    case TOK_AMP:
        return "TOK_AMP";
    case TOK_AMP_AMP:
        return "TOK_AMP_AMP";
    case TOK_AMP_EQ:
        return "TOK_AMP_EQ";
    case TOK_AMP_AMP_EQ:
        return "TOK_AMP_AMP_EQ";
    case TOK_BAR:
        return "TOK_BAR";
    case TOK_BAR_BAR:
        return "TOK_BAR_BAR";
    case TOK_BAR_EQ:
        return "TOK_BAR_EQ";
    case TOK_BAR_BAR_EQ:
        return "TOK_BAR_BAR_EQ";
    case TOK_BANG:
        return "TOK_BANG";
    case TOK_BANG_EQUAL:
        return "TOK_BANG_EQUAL";
    case TOK_EQ:
        return "TOK_EQ";
    case TOK_EQ_EQ:
        return "TOK_EQ_EQ";
    case TOK_GT:
        return "TOK_GT";
    case TOK_GE:
        return "TOK_GE";
    case TOK_LT:
        return "TOK_LT";
    case TOK_LE:
        return "TOK_LE";
    case TOK_DOT:
        return "TOK_DOT";
    case TOK_DOT_DOT:
        return "TOK_DOT_DOT";
    case TOK_TRIPLE_DOT:
        return "TOK_TRIPLE_DOT";
    case TOK_COLON:
        return "TOK_COLON";
    case TOK_COLON_COLON:
        return "TOK_COLON_COLON";

    case TOK_ARROW:
        return "TOK_ARROW";
    case TOK_DOUBLE_ARROW:
        return "TOK_DOUBLE_ARROW";

    case TOK_IDENT:
        return "TOK_IDENT";
    case TOK_STR:
        return "TOK_STR";
    case TOK_CSTR:
        return "TOK_CSTR";
    case TOK_TSTR:
        return "TOK_TSTR";
    case TOK_INT:
        return "TOK_INT";
    case TOK_FLOAT:
        return "TOK_FLOAT";
    case TOK_DOUBLE:
        return "TOK_DOUBLE";

    case KW_AND:
        return "KW_AND";
    case KW_OR:
        return "KW_OR";
    case KW_NOT:
        return "KW_NOT";
    case KW_IF:
        return "KW_IF";
    case KW_ELSE:
        return "KW_ELSE";
    case KW_LOOP:
        return "KW_LOOP";
    case KW_WHILE:
        return "KW_WHILE";
    case KW_FOR:
        return "KW_FOR";
    case KW_IN:
        return "KW_IN";
    case KW_BREAK:
        return "KW_BREAK";
    case KW_CONTINUE:
        return "KW_CONTINUE";
    case KW_RETURN:
        return "KW_RETURN";
    case KW_YIELD:
        return "KW_YIELD";

    case KW_VAR:
        return "KW_VAR";
    case KW_CONST:
        return "KW_CONST";
    case KW_FUN:
        return "KW_FUN";
    case KW_OPER:
        return "KW_OPER";
    case KW_STRUCT:
        return "KW_STRUCT";
    case KW_ENUM:
        return "KW_ENUM";
    case KW_TYPE:
        return "KW_TYPE";
    case KW_INTERFACE:
        return "KW_INTERFACE";
    case KW_USING:
        return "KW_USING";
    case KW_NAMESPACE:
        return "KW_NAMESPACE";
    case KW_STATIC:
        return "KW_STATIC";
    case KW_GLOBAL:
        return "KW_GLOBAL";

    case KW_TRUE:
        return "KW_TRUE";
    case KW_FALSE:
        return "KW_FALSE";
    case KW_NIL:
        return "KW_NIL";
    case KW_SELF:
        return "KW_SELF";

    case KW_AS:
        return "KW_AS";
    case KW_TYPEOF:
        return "KW_TYPEOF";
    case KW_IS:
        return "KW_IS";
    case KW_ALLOC:
        return "KW_ALLOC";
    case KW_DEALLOC:
        return "KW_DEALLOC";

    case KW_EXTERN:
        return "KW_EXTERN";
    }
}

std::string Token::to_string() const {
    std::string result = "[";
    result += token_type_to_string(tok_type);
    result += ", '";
    result += lexeme;
    result += "', ";
    result += std::to_string(location.line);
    result += ":";
    result += std::to_string(location.column);
    result += ":";
    result += std::to_string(location.length);
    result += "]";
    return result;
}
