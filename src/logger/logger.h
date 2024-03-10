#ifndef LOGGER_H
#define LOGGER_H

#include "../scanner/token.h"
#include "error_code.h"
#include <iostream>
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
 * @brief A singleton class to log compiler errors and warnings.
 *
 */
class ErrorLogger {
private:
    // A reference to the output stream to log errors to.
    std::ostream* out = &std::cerr;
    // The total number of errors logged.
    unsigned total_errors = 0;
    // The total number of warnings logged.
    // unsigned total_warnings = 0;

    /**
     * @brief Prints a pretty error message to the console.
     *
     * @param location The location of the error
     * @param display_text The error message to display
     */
    void print_pretty_error(const Location& location, const std::string& display_text);

    ErrorLogger() = default;
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;

public:
    /**
     * @brief Logs an error message to the console.
     *
     * @param token The relevant token.
     * @param error_code The error code.
     * @param message The error message.
     */
    void log_error(const Token& token, ErrorCode error_code, const std::string& message);

    /**
     * @brief Changes the output stream to log errors to.
     *
     * @param new_out The new output stream to log errors to.
     */
    void set_ostream(std::ostream& new_out) {
        out = &new_out;
    }

    /**
     * @brief Get the instance object of the ErrorLogger singleton. Will create the instance if it does not exist.
     *
     * @return ErrorLogger& A reference to the ErrorLogger singleton instance.
     */
    static ErrorLogger& inst() {
        static ErrorLogger instance;
        return instance;
    }
};

#endif // LOGGER_H
