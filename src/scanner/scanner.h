#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <string>
#include <unordered_map>
#include <vector>

class Scanner {
    static std::unordered_map<std::string, TokenType> keywords;
    std::string source;
    std::string filename;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;
};

#endif // SCANNER_H
