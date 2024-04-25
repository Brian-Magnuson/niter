#ifndef ERROR_CODE_H
#define ERROR_CODE_H

/**
 * @brief An enum to name the different error codes that can be thrown by the compiler.
 *  Error codes are prefixed with E or W, where E represents an error, and W represents a warning.
 */
enum ErrorCode {

    // Special errors

    // Default error; should never be thrown under normal circumstances
    E_DEFAULT = 0,

    // Configuration errors
    E_CONFIG = 1000,

    // Scanner errors
    E_SCANNER = 2000,
    // An unexpected character was found in the source code
    E_UNEXPECTED_CHAR,
    // A backslash was found at the end of a line without a newline character
    E_NO_LF_AFTER_BACKSLASH,
    // A multi-line comment was not closed at the end of the file
    E_UNCLOSED_COMMENT,
    // A closing comment '*/' was found without an opening '/*'
    E_CLOSING_UNOPENED_COMMENT,
    // A character literal was not closed after the first character
    E_UNCLOSED_CHAR,
    // An empty character literal was found
    E_EMPTY_CHAR,
    // An illegal escape sequence was found
    E_ILLEGAL_ESC_SEQ,
    // A single-line string literal was not closed at the end of the line
    E_UNCLOSED_STRING,
    // A multi-line string literal was not closed at the end of the file
    E_UNCLOSED_MULTI_LINE_STRING,
    // A numeric literal was found with multiple decimal points
    E_MULTIPLE_DECIMAL_POINTS,
    // A decimal point was found in a number with its base set to non-decimal
    E_NON_DECIMAL_FLOAT,
    // A number was interpreted as having an exponent, but no digits were found after the 'e' or 'E'
    E_NO_DIGITS_IN_EXPONENT,
    // An integer literal was too large to be stored properly
    E_INT_TOO_LARGE,
    // A floating-point literal was too large to be stored properly
    E_FLOAT_TOO_LARGE,
    // A number was found with a non-digit character in it
    E_NON_DIGIT_IN_NUMBER,

    // Parser errors
    E_PARSER = 3000,
    // A statement was found without a semicolon or newline at the end
    E_MISSING_STMT_END,
    // A statement was found where an expression was expected
    E_NOT_AN_EXPRESSION,
    // A list of arguments was found without matching parentheses
    E_UNMATCHED_PAREN_IN_ARGS,
    // A grouping expression was found without matching parentheses
    E_UNMATCHED_PAREN_IN_GROUPING,
    // A tuple expression was found without matching parentheses
    E_UNMATCHED_PAREN_IN_TUPLE,
    // A left square bracket was found without a matching right square bracket
    E_UNMATCHED_LEFT_SQUARE,
    // An equal sign was found without a valid left-hand side
    E_INVALID_ASSIGNMENT,
    // A call expression was found to have over the maximum number of arguments (of 255)
    E_TOO_MANY_ARGS,
    // A variable declaration was found without an identifier
    E_UNNAMED_VAR,
    // A scope resolution operator was found without a valid right-hand side
    E_NOT_AN_IDENTIFIER,
    // A type annotation was found that was not an identifier expression
    E_INVALID_TYPE_ANNOTATION,

    // Type definition errors
    E_TYPE_DEF = 4000,

    // Type checking errors
    E_TYPE_CHECK = 5000,

    // Code generation errors
    E_CODEGEN = 6000,

    // Post-processing errors
    E_POST_PROCESSING = 8000,

    // Compiler malfunction errors
    E_MALFUNCTION = 9000,
    // Statement was reached that should be unreachable
    E_UNREACHABLE,
    // Statement was reached that should be impossible
    E_IMPOSSIBLE,
    // Statement was reached that should be unimplemented
    E_UNIMPLEMENTED,
    // A function was called with an invalid argument
    E_CONVERSION,
    // An error for testing purposes
    E_TEST_ERROR
};

#endif // ERROR_CODE_H
