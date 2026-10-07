#pragma once

#include "token.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;

public:
    Lexer(const std::string& source);

    std::vector<Token> tokenize();
};