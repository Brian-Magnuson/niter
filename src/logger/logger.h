#ifndef LOGGER_H
#define LOGGER_H

#include "../scanner/token.h"
#include "error_code.h"
#include <string>

/**
 * @brief An enum to represent the different colors that can be used to colorize text in the console.
 *
 */
enum class Color {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    RESET
};

/**
 * @brief A function to return the escape sequence for a given color.
 *
 * @param color The color to return the escape sequence for. Defaults to Color::RESET.
 * @return std::string The escape sequence for the given color.
 * E.g. colorize(Color::RED) returns "\033[31m"
 */
std::string colorize(Color color = Color::RESET);

/**
 * @brief A class to log compiler errors and warnings.
 *
 */
class ErrorLogger {
private:
    // The total number of errors logged.
    unsigned total_errors = 0;
    // The total number of warnings logged.
    unsigned total_warnings = 0;

    /**
     * @brief Prints a pretty error message to the console.
     *
     * @param location The location of the error
     * @param display_text The error message to display
     */
    void print_pretty_error(const Location& location, const std::string& display_text);

public:
    /**
     * @brief Logs an error message to the console.
     *
     * @param token The relevant token.
     * @param error_code The error code.
     * @param message The error message.
     */
    void log_error(const Token& token, ErrorCode error_code, const std::string& message);
};

#endif // LOGGER_H
