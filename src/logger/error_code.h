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
    E_UNREACHABLE = 9001,
    // Statement was reached that should be impossible
    E_IMPOSSIBLE = 9002,
    // Statement was reached that should be unimplemented
    E_UNIMPLEMENTED = 9003,
};

#endif // ERROR_CODE_H