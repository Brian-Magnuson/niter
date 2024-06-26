#ifndef PARSER_H
#define PARSER_H

#include "../logger/error_code.h"
#include "../scanner/token.h"
#include "../utility/decl.h"
#include "../utility/expr.h"
#include "../utility/stmt.h"
#include "annotation.h"
#include <exception>
#include <memory>
#include <stack>
#include <vector>

/**
 * @brief An exception class for parser errors.
 *
 */
class ParserException : public std::exception {};

/**
 * @brief A class to parse a vector of tokens into an abstract syntax tree.
 *
 */
class Parser {
    // The vector of tokens to parse.
    std::vector<std::shared_ptr<Token>> tokens;
    // The current token index.
    unsigned current = 0;
    // A stack to keep track of grouping tokens. If the stack is empty, newlines are significant.
    std::stack<TokenType> grouping_tokens;

    /**
     * @brief Returns the current token.
     *
     * @return Token& The token at the current index.
     */
    Token& peek();

    /**
     * @brief Returns the previous token.
     *
     * @return Token& The token at the previous index.
     */
    Token& previous();

    /**
     * @brief Checks if the current token is any of the given types. Returns false if the current token is an EOF token.
     *
     * @param types The types to check.
     * @return true If the current token is of the given type.
     * @return false If not or if the current token is an EOF token.
     */
    bool check(const std::vector<TokenType>& types);

    /**
     * @brief Checks if an EOF token was reached.
     *
     * @return true If the current token has TokenType TOK_EOF.
     * @return false Otherwise.
     */
    bool is_at_end();

    /**
     * @brief Advances the parser and returns the previous token.
     * E.g., if the current token is the first token, it will return the first token and then advance to the second token.
     *
     * If newlines are currently insignificant, it will skip over any newline tokens.
     *
     * @return Token& The previous token.
     */
    Token& advance();

    /**
     * @brief Check if the current token is any of the given types and advances the parser if it is.
     *
     * @param types The types to check.
     * @return true If the current token is any of the given types.
     * @return false Otherwise.
     */
    bool match(const std::vector<TokenType>& types);

    /**
     * @brief Checks if the current token is of the given type and advances the parser if it is. Otherwise, it logs an error.
     *
     * @param tok_type The type to check.
     * @param error_code The error code to log.
     * @param message The message to be logged with the error.
     * @return Token& The token that was consumed.
     */
    Token& consume(TokenType tok_type, ErrorCode error_code, const std::string& message);

    /**
     * @brief Consumes tokens until a safe token is reached. Used to recover from errors.
     *
     */
    void synchronize();

    // MARK: Statements

    /**
     * @brief Parses a generic statement. Will catch any exceptions thrown by the specific statement parsers.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed statement.
     */
    std::shared_ptr<Stmt> statement();

    /**
     * @brief Parses a declaration statement, i.e. a declaration by itself.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed declaration statement.
     */
    std::shared_ptr<Stmt> declaration_statement();

    /**
     * @brief Parses an if statement.
     * An if statement begins with the `if` keyword followed by an expression, a statement or statements, and an optional `else` keyword followed by a statement or statements.
     *
     * @return std::shared_ptr<Stmt>
     */
    std::shared_ptr<Stmt> if_statement();

    /**
     * @brief Parses an expression statement, i.e. an expression by itself.
     * Expression statements are expressions followed by either a semicolon or a newline.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed expression statement.
     * @throw ParserException If an error occurs while parsing the statement. Will be caught by the statement() function.
     */
    std::shared_ptr<Stmt> expression_statement();

    /**
     * @brief Parses a return statement.
     *
     * @return std::shared_ptr<Stmt> A pointer to the parsed return statement.
     * @throw ParserException If an error occurs while parsing the statement. Will be caught by the statement() function.
     */
    std::shared_ptr<Stmt> return_statement();

    // MARK: Declarations

    /**
     * @brief Parsers a variable declaration.
     * Variable declarations begin with either "var" or "const" followed by an identifier and an optional initializer.
     *
     * @return std::shared_ptr<Decl> A pointer to the parsed variable declaration.
     * @throw ParserException If an error occurs while parsing the declaration. Will be caught by the statement() function.
     */
    std::shared_ptr<Decl> var_decl();

    /**
     * @brief Parses a function declaration.
     * Function declarations begin with "fun" followed by an identifier, a list of parameters, and a set of braces containing statements.
     *
     * @return std::shared_ptr<Decl> A pointer to the parsed function declaration.
     * @throw ParserException If an error occurs while parsing the declaration. Will be caught by the statement() function.
     */
    std::shared_ptr<Decl> fun_decl();

    /**
     * @brief Parses an external function declaration.
     * External function declarations begin with "extern fun" followed by an identifier, a list of parameters, and a type annotation.
     * Unlike function declarations, external function declarations do not have a body.
     *
     * @param is_variadic Whether the external function is variadic.
     * @return std::shared_ptr<Decl> A pointer to the parsed external function declaration.
     * @throw ParserException If an error occurs while parsing the declaration. Will be caught by the statement() function.
     */
    std::shared_ptr<Decl> extern_fun_decl(bool is_variadic = false);

    /**
     * @brief Parses a struct declaration.
     * Struct declarations begin with "struct" followed by an identifier and a set of braces containing member declarations.
     * Struct scopes are considered global space, so functions may be declared inside a struct.
     * Unlike other global scopes, the requirement of "declarations only" can be enforced here in the parser.
     *
     * @return std::shared_ptr<Decl> A pointer to the parsed struct declaration.
     */
    std::shared_ptr<Decl> struct_decl();

    // MARK: Expressions

    /**
     * @brief Parses a generic expression.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> expression();

    /**
     * @brief Parses an assignment expression.
     * Assignment expressions are expressions followed by an equal sign and another expression.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed assignment expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> assign_expr();

    /**
     * @brief Parses a logical OR expression.
     * Logical OR expressions are expressions separated by the "or" keyword.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed logical OR expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> or_expr();

    /**
     * @brief Parses a logical AND expression.
     * Logical AND expressions are expressions separated by the "and" keyword.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed logical AND expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> and_expr();

    /**
     * @brief Parses an equality expression.
     * Equality expressions are expressions separated by the "==" or "!=" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed equality expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> equality_expr();

    /**
     * @brief Parses a comparison expression.
     * Comparison expressions are expressions separated by the "<", "<=", ">", or ">=" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed comparison expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> comparison_expr();

    /**
     * @brief Parses a term expression.
     * Term expressions are expressions separated by the "+" or "-" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed term expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> term_expr();

    /**
     * @brief Parses a factor expression.
     * Factor expressions are expressions separated by the "*", "/" or "%" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed factor expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> factor_expr();

    /**
     * @brief Parses a power expression.
     * Power expressions are expressions separated by the "^" operator.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed power expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> power_expr();

    /**
     * @brief Parses a unary expression.
     * Unary expressions are expressions preceded by the "-" or "!" operators.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed unary expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> unary_expr();

    /**
     * @brief Parses an access or index expression.
     * Access expressions are expressions followed by a "." or "->".
     * Index expressions are expressions followed by a pair of square brackets containing an expression.
     * They are used to access fields or elements of an object, array, or pointer.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed access or index expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> access_index_expr();

    /**
     * @brief Parses a call expression.
     * Call expressions are expressions followed by a pair of parentheses containing zero or more arguments.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed call expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> call_expr();

    /**
     * @brief Parses a cast expression.
     * Cast expressions are expressions followed by a "as" keyword and a type annotation.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed cast expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> cast_expr();

    /**
     * @brief Parses a primary expression.
     * Primary expressions are the most basic expressions and can be literals, identifiers, or grouped expressions.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed primary expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> primary_expr();

    /**
     * @brief Parses an object expression.
     * An object expression is an annotation for a struct followed by a set of braces containing member initializers.
     * Object expressions are used to create instances of structs.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed object expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> object_expr();

    /**
     * @brief Parses an array expression.
     * Array expressions can either be in list form or in array-generator form.
     *
     * @return std::shared_ptr<Expr> A pointer to the parsed array expression.
     * @throw ParserException If an error occurs while parsing the expression. Will be caught by the statement() function.
     */
    std::shared_ptr<Expr> array_expr();

    // MARK: Annotations

    /**
     * @brief Parses a type annotation.
     *
     * @return std::shared_ptr<Annotation> The parsed type annotation.
     * @throw ParserException If an error occurs while parsing the annotation. Will be caught by the statement() function.
     */
    std::shared_ptr<Annotation> annotation();

    /**
     * @brief Parses a segmented type annotation.
     * A segmented is made up of multiple "classes" separated by "::".
     * A class is an identifier with optional type arguments.
     * A class cannot be a tuple, array, pointer, or function annotation.
     * E.g. "Vector<i32>::Iterator".
     * Additionally, if the segmented annotation ends with "[]" or "*", it will
     * be parsed as an array or pointer type annotation.
     *
     * @return std::shared_ptr<Annotation> The segmented, array, or pointer type annotation.
     * @throw ParserException If an error occurs while parsing the annotation. Will be caught by the statement() function.
     */
    std::shared_ptr<Annotation> segmented_annotation();

    /**
     * @brief Parses a function type annotation.
     * Function type annotations must begin with the keyword "fun" followed by a list of paramter types, "=>", and a return type.
     * The parameter and return types may be mutable, and, thus, may have the "var" keyword prepended to them.
     *
     * @return std::shared_ptr<Annotation::Function> The parsed function type annotation.
     * @throw ParserException If an error occurs while parsing the annotation. Will be caught by the statement() function.
     */
    std::shared_ptr<Annotation::Function> function_annotation();

    /**
     * @brief Parses a tuple type annotation.
     * Tuple type annotations are a list of type annotations separated by commas and enclosed in parentheses.
     * Tuple types cannot begin with "fun" to not be confused with function type annotations.
     * E.g. "(int, int)".
     * Trailing commas are allowed.
     * "()" is the empty tuple type.
     *
     * @return std::shared_ptr<Annotation::Tuple> The parsed tuple type annotation.
     * @throw ParserException If an error occurs while parsing the annotation. Will be caught by the statement() function.
     */
    std::shared_ptr<Annotation::Tuple> tuple_annotation();

    /**
     * @brief Parses an array type annotation.
     * Array type annotations are a pair of square brackets containing a type annotation, a semicolon, and a literal size.
     * The size can also be `*` to indicate unknown size.
     * E.g. "[int; 10]", "[int; *]".
     *
     * @return std::shared_ptr<Annotation::Array> The parsed array type annotation.
     * @throw ParserException If an error occurs while parsing the annotation. Will be caught by the statement() function.
     */
    std::shared_ptr<Annotation::Array> array_annotation();

    /**
     * @brief Resolves a segmented annotation to its fully qualified name.
     * Useful if a using declaration is present.
     *
     * @param annotation A reference segmented annotation to resolve. Will be modified in place.
     */
    void resolve_annotation(std::shared_ptr<Annotation::Segmented>& annotation);

public:
    // MARK: Interface

    /**
     * @brief Construct a new Parser object.
     *
     * @param tokens The vector of tokens to parse.
     * @deprecated Use the default constructor instead.
     */
    Parser(const std::vector<std::shared_ptr<Token>>& tokens) : tokens(tokens) {}

    /**
     * @brief Construct a new Parser object.
     * Does not initialize the tokens vector.
     * Pass the tokens vector to the parse() function instead.
     *
     */
    Parser() = default;

    /**
     * @brief Parses the vector of tokens into an abstract syntax tree.
     * Also sets the tokens vector to the given vector of tokens.
     *
     * @param tokens The vector of tokens to parse.
     * @return std::vector<std::shared_ptr<Stmt>> A vector of AST statements.
     */
    std::vector<std::shared_ptr<Stmt>> parse(const std::vector<std::shared_ptr<Token>>& tokens);

    /**
     * @brief Parses the vector of tokens into an abstract syntax tree.
     *
     * @return std::vector<std::shared_ptr<Stmt>> A vector of AST statements.
     * @deprecated Use the parse(const std::vector<std::shared_ptr<Token>>& tokens) function instead.
     */
    std::vector<std::shared_ptr<Stmt>> parse();
};

#endif // PARSER_H
