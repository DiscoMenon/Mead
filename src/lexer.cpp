#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_map>

Lexer::Lexer(const std::string& source)
    : source(source), position(0), line(1), column(1) {
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (position < source.length()) {

        char current = source[position];

        // Whitespace
        if (current == ' ' || current == '\t' || current == '\r') {
            position++;
            column++;
            continue;
        }

        // Newline
        if (current == '\n') {
            position++;
            line++;
            column = 1;
            continue;
        }

        // Integer
        if (std::isdigit(current)) {
            int startColumn = column;
            std::string number;
            bool isFloat = false;

            while (position < source.length() &&
                std::isdigit(source[position])) {

                number += source[position];
                position++;
                column++;
            }

            // Decimal part
            if (position < source.length() &&
                source[position] == '.' &&
                position + 1 < source.length() &&
                std::isdigit(source[position + 1])) {

                isFloat = true;

                number += '.';
                position++;
                column++;

                while (position < source.length() &&
                    std::isdigit(source[position])) {

                    number += source[position];
                    position++;
                    column++;
                }
            }

            tokens.push_back({
                isFloat ? TokenType::FLOAT : TokenType::INTEGER,
                number,
                line,
                startColumn
            });

            continue;
        }

        // Identifier / keyword
        if (std::isalpha(current) || current == '_') {
            int startColumn = column;
            std::string word;

            while (position < source.length() &&
                   (std::isalnum(source[position]) ||
                    source[position] == '_')) {

                word += source[position];
                position++;
                column++;
            }

            if (word == "print") {
                tokens.push_back({
                    TokenType::PRINT,
                    word,
                    line,
                    startColumn
                });
            } else if (word == "var") {
                tokens.push_back({
                    TokenType::VAR,
                    word,
                    line,
                    startColumn
                });
            } else if (word == "true" || word == "false") {
                tokens.push_back({
                    TokenType::BOOL,
                    word,
                    line,
                    startColumn
                });
            } else {
                tokens.push_back({
                    TokenType::IDENTIFIER,
                    word,
                    line,
                    startColumn
                });
            }

            continue;
        }

        // String
        if (current == '"') {
            int startColumn = column;

            position++;
            column++;

            std::string value;

            while (position < source.length() &&
                   source[position] != '"') {

                value += source[position];
                position++;
                column++;
            }

            if (position >= source.length()) {
                throw std::runtime_error(
                    "Unterminated string"
                );
            }

            position++;
            column++;

            tokens.push_back({
                TokenType::STRING,
                value,
                line,
                startColumn
            });

            continue;
        }

        // Single-character tokens
        switch (current) {

            case '(':
                tokens.push_back({TokenType::LPAREN, "(", line, column});
                break;

            case ')':
                tokens.push_back({TokenType::RPAREN, ")", line, column});
                break;

            case '{':
                tokens.push_back({TokenType::LBRACE, "{", line, column});
                break;

            case '}':
                tokens.push_back({TokenType::RBRACE, "}", line, column});
                break;

            case '[':
                tokens.push_back({TokenType::LBRACKET, "[", line, column});
                break;

            case ']':
                tokens.push_back({TokenType::RBRACKET, "]", line, column});
                break;

            case '+':
                tokens.push_back({TokenType::PLUS, "+", line, column});
                break;

            case '-':
                tokens.push_back({TokenType::MINUS, "-", line, column});
                break;

            case '*':
                tokens.push_back({TokenType::STAR, "*", line, column});
                break;

            case '/':
                tokens.push_back({TokenType::SLASH, "/", line, column});
                break;

            case '%':
                tokens.push_back({TokenType::PERCENT, "%", line, column});
                break;

            case ':':
                tokens.push_back({TokenType::COLON, ":", line, column});
                break;

            case ',':
                tokens.push_back({TokenType::COMMA, ",", line, column});
                break;

            case '.':
                tokens.push_back({TokenType::DOT, ".", line, column});
                break;

            case ';':
                tokens.push_back({TokenType::SEMICOLON, ";", line, column});
                break;

            case '=':
                if (position + 1 < source.length() &&
                    source[position + 1] == '=') {

                    tokens.push_back({
                        TokenType::EQUAL_EQUAL,
                        "==",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    tokens.push_back({
                        TokenType::EQUAL,
                        "=",
                        line,
                        column
                    });
                }

                break;

            case '!':
                if (position + 1 < source.length() &&
                    source[position + 1] == '=') {

                    tokens.push_back({
                        TokenType::NOT_EQUAL,
                        "!=",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    tokens.push_back({
                        TokenType::NOT,
                        "!",
                        line,
                        column
                    });
                }

                break;

            case '<':
                if (position + 1 < source.length() &&
                    source[position + 1] == '=') {

                    tokens.push_back({
                        TokenType::LESS_EQUAL,
                        "<=",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    tokens.push_back({
                        TokenType::LESS,
                        "<",
                        line,
                        column
                    });
                }

                break;

            case '>':
                if (position + 1 < source.length() &&
                    source[position + 1] == '=') {

                    tokens.push_back({
                        TokenType::GREATER_EQUAL,
                        ">=",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    tokens.push_back({
                        TokenType::GREATER,
                        ">",
                        line,
                        column
                    });
                }

                break;

            case '&':
                if (position + 1 < source.length() &&
                    source[position + 1] == '&') {

                    tokens.push_back({
                        TokenType::AND,
                        "&&",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    throw std::runtime_error(
                        "Unexpected character '&'"
                    );
                }

                break;

            case '|':
                if (position + 1 < source.length() &&
                    source[position + 1] == '|') {

                    tokens.push_back({
                        TokenType::OR,
                        "||",
                        line,
                        column
                    });

                    position++;
                    column++;

                } else {
                    throw std::runtime_error(
                        "Unexpected character '|'"
                    );
                }

                break;

            default:
                throw std::runtime_error(
                    std::string("Unexpected character: ") + current
                );
        }

        position++;
        column++;
    }

    tokens.push_back({
        TokenType::END_OF_FILE,
        "",
        line,
        column
    });

    return tokens;
}
