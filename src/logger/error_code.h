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
    // A closing comment '*/' was found without an opening '/*'
    E_CLOSING_UNOPENED_COMMENT,
    // A backslash was found at the end of a line without a newline character
    E_NO_LF_AFTER_BACKSLASH,

    // Parser errors
    E_PARSER = 3000,

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
};

#endif // ERROR_CODE_H
