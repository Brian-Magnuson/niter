#include "logger.h"
#include <fstream>
#include <iomanip>

std::string colorize(Color color) {
    switch (color) {
    case Color::RED:
        return "\033[31m";
    case Color::GREEN:
        return "\033[32m";
    case Color::YELLOW:
        return "\033[33m";
    case Color::BLUE:
        return "\033[34m";
    case Color::MAGENTA:
        return "\033[35m";
    case Color::CYAN:
        return "\033[36m";
    case Color::WHITE:
        return "\033[37m";
    case Color::RESET:
        return "\033[0m";
    default:
        return "\033[0m";
    }
}

void ErrorLogger::print_pretty_error(const Location& location, const std::string& display_text) {
    *out << std::endl;
    *out << *location.file_name << ":" << location.line << ":"
         << location.column << std::endl;

    *out
        << colorize(Color::RED) << "Error "
        << errors.size() << ": "
        << colorize(Color::RESET)
        << display_text << std::endl;
    std::string err_line = location.source_code->substr(location.line_index, location.source_code->find('\n', location.line_index) - location.line_index);

    // Pad the line number with spaces so that the caret lines up with the error
    // 4 spaces should be good enough for 5 digits
    *out << std::setw(5) << location.line << " | "
         << err_line << std::endl;
    // 5 digits + 3 extra characters = 8

    *out << std::string(location.column + 8, ' ') << colorize(Color::RED)
         << "^";

    // Just in case the length is 0, we don't want to print any extra tildes
    if (location.length > 1)
        *out << std::string(location.length - 1, '~');

    *out << colorize(Color::RESET)
         << std::endl;
    /*
    Example output:
    test_files/error_test.nit:1:5
    Error 1: Test error message
        1 | var x = 5
            ^~~

    */
}

void ErrorLogger::print_pretty_error(const std::string& display_text) {
    *out << std::endl;
    *out << "From compiler:" << std::endl;

    *out << colorize(Color::RED) << "Error: "
         << colorize(Color::RESET)
         << display_text << std::endl;
}

void ErrorLogger::print_pretty_note(const Location& location, const std::string& display_text) {
    if (location.file_name->empty())
        return; // Don't print notes for internal errors

    // *out << std::endl; // Typically, note messages will come immediately after an error message.
    // Omitting this line will put the note right next to the error message.

    *out << *location.file_name << ":" << location.line << ":"
         << location.column << std::endl;

    *out
        << colorize(Color::CYAN) << "Note: "
        << colorize(Color::RESET)
        << display_text << std::endl;
    std::string err_line = location.source_code->substr(location.line_index, location.source_code->find('\n', location.line_index) - location.line_index);

    // Pad the line number with spaces so that the caret lines up with the error
    // 4 spaces should be good enough for 5 digits
    *out << std::setw(5) << location.line << " | "
         << err_line << std::endl;
    // 5 digits + 3 extra characters = 8

    *out << std::string(location.column + 8, ' ') << colorize(Color::CYAN)
         << "^";

    // Just in case the length is 0, we don't want to print any extra tildes
    if (location.length > 1)
        *out << std::string(location.length - 1, '~');

    *out << colorize(Color::RESET)
         << std::endl;
    /*
    Example output:
    test_files/error_test.nit:1:5
    Info 1: Test info message
        1 | var x = 5
            ^~~

    */
}

void ErrorLogger::log_error(const Location& location, ErrorCode error_code, const std::string& message) {
    auto new_message = std::to_string(static_cast<int>(error_code)) + " " + message;
    errors.push_back(error_code);
    if (printing_enabled)
        print_pretty_error(location, new_message);
}

void ErrorLogger::log_error(ErrorCode error_code, const std::string& message) {
    auto new_message = std::to_string(static_cast<int>(error_code)) + " " + message;
    errors.push_back(error_code);
    if (printing_enabled)
        print_pretty_error(new_message);
}

void ErrorLogger::log_note(const Location& location, const std::string& message) {
    if (printing_enabled)
        print_pretty_note(location, message);
}

void ErrorLogger::reset() {
    out = &std::cerr;
    errors.clear();
    printing_enabled = true;
}
