#ifndef LOGGER_H
#define LOGGER_H

#include "../scanner/token.h"
#include "error_code.h"
#include <iostream>
#include <string>
#include <vector>

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
    // A list of the errors that have been logged.
    std::vector<ErrorCode> errors;
    // A boolean to determine if the error logger should print to the ostream.
    bool printing_enabled = true;

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
     * @param location The location of the error.
     * @param error_code The error code.
     * @param message The error message.
     */
    void log_error(const Location& location, ErrorCode error_code, const std::string& message);

    /**
     * @brief Changes the output stream to log errors to.
     *
     * @param new_out The new output stream to log errors to.
     */
    void set_ostream(std::ostream& new_out) {
        out = &new_out;
    }

    /**
     * @brief Sets whether the error logger should print to the ostream.
     *
     * @param enabled If true, the error logger will print to the ostream. If false, it will not.
     */
    void set_printing_enabled(bool enabled) {
        printing_enabled = enabled;
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

    /**
     * @brief Get the errors object.
     *
     * @return const std::vector<ErrorCode>& The list of errors that have been logged.
     */
    const std::vector<ErrorCode>& get_errors() const {
        return errors;
    }

    /**
     * @brief Clears the list of errors that have been logged.
     *
     */
    void clear_errors() {
        errors.clear();
    }

    /**
     * @brief Resets the error logger to its default state.
     * Errors are cleared, printing is enabled, and the ostream is set to std::cerr.
     * Useful for testing.
     *
     */
    void reset();
};

#endif // LOGGER_H
