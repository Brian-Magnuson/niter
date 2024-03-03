#include "logger.h"
#include <fstream>
#include <iostream>

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
    std::cerr << *location.file_name << ":" << location.line << ":"
              << location.column << std::endl;

    std::cerr << colorize(Color::RED) << "Error: " << colorize(Color::RESET)
              << display_text << std::endl;
    std::string err_line = location.source_code->substr(location.line_index, location.source_code->find('\n', location.line_index));
    std::cerr << location.line << " | " << err_line << std::endl;
    std::cerr << std::string(location.column, ' ') << colorize(Color::RED)
              << "^" << std::string(location.length - 1, '~')
              << colorize(Color::RESET)
              << std::endl
              << std::endl;
    /*
    Example output:
    test_files/error_test.nit:1:5
    Error: 1000 Test error message
    1 | let x = 5
        ^~~

    */
}

void ErrorLogger::log_error(const Token& token, ErrorCode error_code, const std::string& message) {
    auto new_message = std::to_string(static_cast<int>(error_code)) + " " + message;
    print_pretty_error(token.location, new_message);
    total_errors++;
}
