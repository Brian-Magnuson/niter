#ifndef ERROR_CODE_H
#define ERROR_CODE_H

/**
 * @brief An enum to name the different error codes that can be thrown by the compiler.
 *  Error codes are prefixed with E or W, where E represents an error, and W represents a warning.
 * Note: Error codes are named based on what the compiler *observes*, not what is disallowed.
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
    // A call expression was found to have over the maximum number of arguments (of 255)
    E_TOO_MANY_ARGS,
    // A variable declaration was found without an identifier
    E_UNNAMED_VAR,
    // A scope resolution operator was found without a valid right-hand side
    E_NOT_AN_IDENTIFIER,
    // A type annotation was found that did not start with a valid token
    E_INVALID_TYPE_ANNOTATION,
    // No identifier was found after the colon in a type annotation
    E_MISSING_IDENT_IN_TYPE,
    // A type annotation was found without a matching right square bracket
    E_UNMATCHED_SQUARE_IN_TYPE,
    // A type annotation was found without a matching right angle bracket
    E_UNMATCHED_ANGLE_IN_TYPE,
    // A function pointer type was found without a left parenthesis
    E_NO_LPAREN_IN_FUN_TYPE,
    // A function pointer type was found without a double arrow token
    E_NO_ARROW_IN_FUN_TYPE,
    // A type annotation was found without a matching right parenthesis
    E_UNMATCHED_PAREN_IN_TYPE,
    // An arrow token was found in a non-function type
    E_ARROW_IN_NON_FUN_TYPE,
    // A function declaration was found without an identifier
    E_UNNAMED_FUN,
    // A function declaration was found without a left parenthesis
    E_NO_LPAREN_IN_FUN_DECL,
    // A function declaration was found without a right parenthesis
    E_UNMATCHED_PAREN_IN_PARAMS,
    // A function declaration parameter was found without a type annotation
    E_AUTO_IN_PARAM,
    // A function declaration was found without a left brace
    E_NO_LBRACE_IN_FUN_DECL,
    // A function declaration was found without a right brace
    E_UNMATCHED_BRACE_IN_FUN_DECL,
    // The extern keyword was found without a valid declarer
    E_NO_DECLARER_AFTER_EXTERN,
    // A struct declaration was found without an identifier
    E_UNNAMED_STRUCT,
    // A struct declaration was found without a left brace
    E_NO_LBRACE_IN_STRUCT_DECL,
    // A struct declaration was found without a right brace
    E_UNMATCHED_BRACE_IN_STRUCT_DECL,

    // Global type errors
    E_GLOBAL_TYPE = 4000,
    // A symbol was declared with the same name in the same scope
    E_SYMBOL_ALREADY_DECLARED,
    // The main function was found with the wrong signature
    E_INVALID_MAIN_SIGNATURE,
    // A namespace was introduced in a struct
    E_NAMESPACE_IN_STRUCT,
    // A struct was declared with the same name in the same scope
    E_STRUCT_ALREADY_DECLARED,
    // An expression statement was found in global space
    E_GLOBAL_EXPRESSION,
    // A return statement was found outside of a function
    E_GLOBAL_RETURN,
    // A print statement was found outside of a function
    E_GLOBAL_PRINT,

    // Local type errors
    E_LOCAL_TYPE = 5000,
    // A namespace was introduced in a non-global scope
    E_NAMESPACE_IN_LOCAL_SCOPE,
    // A struct was introduced in a non-global scope
    E_STRUCT_IN_LOCAL_SCOPE,
    // A function was introduced in a non-global scope
    E_FUN_IN_LOCAL_SCOPE,
    // A variable was declared without an initializer
    E_AUTO_WITHOUT_INITIALIZER,
    // A constant was declared without an initializer
    E_UNINITIALIZED_CONST,
    // A local variable was declared with the same name in the same scope
    E_LOCAL_ALREADY_DECLARED,
    // A variable was found that was never declared
    E_UNKNOWN_VAR,
    // A type annotation could not be resolved
    E_UNKNOWN_TYPE,
    // An assignment was found with incompatible types
    E_INCOMPATIBLE_TYPES,
    // A cast expression was found with incompatible types
    E_INVALID_CAST,
    // An attempt was made to assign const pointer to a non-const pointer
    E_INVALID_PTR_DECLARER,
    // An array was found with inconsistent types
    E_INCONSISTENT_ARRAY_TYPES,
    // A function was found to have multiple parameters with the same name
    E_DUPLICATE_PARAM_NAME,
    // A function was found with a return statement with an incompatible type
    E_RETURN_INCOMPATIBLE,
    // A return statement with a value was found in a void function
    E_RETURN_IN_VOID_FUN,
    // A return statement was not found in a non-void function
    E_NO_RETURN_IN_NON_VOID_FUN,
    // A dot-access expression was found without an identifier after the dot
    E_NO_IDENT_AFTER_DOT,
    // An attempt was made to dereference a non-pointer
    E_DEREFERENCE_NON_POINTER,
    // A dot-access expression was found on a non-struct type
    E_ACCESS_ON_NON_STRUCT,
    // A subscript-access expression was found on a non-array type
    E_INDEX_ON_NON_ARRAY,
    // A tuple was found with an index that was not a literal
    E_NO_LITERAL_INDEX_ON_TUPLE,
    // A tuple was found with an index that was out of range
    E_TUPLE_INDEX_OUT_OF_RANGE,
    // A struct member could not be found
    E_INVALID_STRUCT_MEMBER,
    // A function call was found on a non-function type
    E_CALL_ON_NON_FUN,
    // A function call was found with an incorrect number of arguments
    E_INVALID_ARITY,
    // The unary operator '&' was found on a non-lvalue
    E_ADDRESS_OF_NON_LVALUE,
    // An assignment was found to a non-lvalue
    E_ASSIGN_TO_NON_LVALUE,
    // An assignment was found to a constant
    E_ASSIGN_TO_CONST,

    // Code generation errors
    E_CODEGEN = 6000,
    // The code generator expected a constant, but a non-constant was found
    E_NOT_A_CONSTANT,
    // The emitter was unable to create a target machine
    E_NO_TARGET_MACHINE,
    // The emitter could not open the output file
    E_INVALID_OUTPUT,
    // The target machine could not emit a file of the specified type
    E_INVALID_OUTPUT_TYPE,

    // Post-processing errors
    E_POST_PROCESSING = 8000,

    // Compiler malfunction errors
    E_MALFUNCTION = 9000,
    // An unknown error occurred
    E_UNKNOWN,
    // Statement was reached that should be unreachable. Typically used when a series of conditional checks do not catch every case.
    E_UNREACHABLE,
    // Statement was reached that should be impossible. Typically used when a function does not behave as expected.
    E_IMPOSSIBLE,
    // Statement was reached that should be unimplemented
    E_UNIMPLEMENTED,
    // An error occurred while writing to a file
    E_IO,
    // A function was called with an invalid argument
    E_CONVERSION,
    // Any cast failed
    E_ANY_CAST,
    // An error for testing purposes
    E_TEST_ERROR,
    // The compiler attempted to exit the root scope
    E_EXITED_ROOT_SCOPE,
    // An expression was interpreted as a literal, but the literal was not recognized
    E_UNRECOGNIZED_LITERAL,
    // The llvm::Module could not be verified
    E_UNVERIFIED_MODULE,

};

#endif // ERROR_CODE_H
